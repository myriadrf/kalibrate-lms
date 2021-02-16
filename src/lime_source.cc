/*
 * Copyright (c) 2021, Supreeth Herle
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     *  Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *     *  Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <complex>
#include <iostream>
#include <lime/LimeSuite.h>

#include "lime_source.h"

extern int g_verbosity;


lime_source::lime_source(double sample_rate,
			 double fpga_master_clock_freq,
			 double external_ref) {

	m_desired_sample_rate = sample_rate;
	m_fpga_master_clock_freq = fpga_master_clock_freq;
	m_external_ref = external_ref;
	m_sample_rate = 0.0;
	m_cb = new circular_buffer(CB_LEN, sizeof(complex), 0);

	pthread_mutex_init(&m_u_mutex, 0);
}


lime_source::~lime_source() {

	delete m_cb;
	LMS_Close(m_dev);
	pthread_mutex_destroy(&m_u_mutex);
}

void lime_source::tune_dac(uint16_t dacVal) {

	if (LMS_VCTCXOWrite(m_dev, dacVal) != 0) {
		fprintf(stderr, "Failed to set runtime VCTCXO DAC trim value\n");
	}
	fprintf(stderr, "VCTCXO DAC value set to: %f\n", get_board_dac());
}

double lime_source::get_board_dac() {

	double dac_value = 0.0;
	if (LMS_ReadCustomBoardParam(m_dev, BOARD_PARAM_DAC, &dac_value, NULL) != 0) {
		fprintf(stderr, "Failed to read runtime VCTCXO DAC trim value\n");
	}
	return dac_value;
}


void lime_source::stop() {

	pthread_mutex_lock(&m_u_mutex);
	if(m_dev) {
		LMS_StopStream(&m_rx_stream);
		LMS_DestroyStream(m_dev, &m_rx_stream);
	}
	pthread_mutex_unlock(&m_u_mutex);
}


void lime_source::start() {

	pthread_mutex_lock(&m_u_mutex);
	if(m_dev) {
		// TODO: Perform calibration if possible

		/* configure Streams */
		m_rx_stream = {};
		m_rx_stream.isTx = false;
		m_rx_stream.channel = 0;
		m_rx_stream.fifoSize = 1024 * 1024;
		m_rx_stream.throughputVsLatency = 0.3;
		m_rx_stream.dataFmt = lms_stream_t::LMS_FMT_I16;

		LMS_SetupStream(m_dev, &m_rx_stream);

		LMS_StartStream(&m_rx_stream);
	}
	pthread_mutex_unlock(&m_u_mutex);
}


double lime_source::sample_rate() {

	return m_sample_rate;
}

int lime_source::tune(double freq) {

	double actual_freq = 0.0;
	int ret = 0;

	pthread_mutex_lock(&m_u_mutex);
	if (LMS_SetLOFrequency(m_dev, LMS_CH_RX, 0, freq) != 0) {
		fprintf(stderr, "LMS_SetLOFrequency: Failed to set RX LO frequency\n");
		ret = -1;
	}

	if (LMS_GetLOFrequency(m_dev, LMS_CH_RX, 0, &actual_freq) != 0) {
		fprintf(stderr, "LMS_GetLOFrequency: Failed to get RX LO frequency\n");
		ret = -1;
	}
	pthread_mutex_unlock(&m_u_mutex);

	return ret;
}

void lime_source::set_antenna(const std::string antenna) {

	int idx = -1;
	lms_name_t name_list[12]; /* large enough list for antenna names. */
	const char* c_name = antenna.c_str();
	int num_names;
	int i;

	num_names = LMS_GetAntennaList(m_dev, LMS_CH_RX, 0, name_list);
	for (i = 0; i < num_names; i++) {
		if (!strcmp(c_name, name_list[i])) {
			idx = i;
			break;
		}
	}
	if (idx < 0) {
		fprintf(stderr, "Invalid Rx Antenna: %s\n", c_name);
	}

	if (LMS_SetAntenna(m_dev, LMS_CH_RX, 0, idx) != 0)
		fprintf(stderr, "LMS_SetAntenna: Failed to set RX antenna\n");
}

bool lime_source::set_gain(double gain) {

	if (LMS_SetGaindB(m_dev, LMS_CH_RX, 0, gain) < 0)
		fprintf(stderr, "Error setting RX gain to %f\n", gain);

	return true;
}

void print_range(lms_range_t *range) {
	fprintf(stderr, "Sampling Rate Range: Min=%f Max=%f Step=%f\n", range->min, range->max, range->step);
}

/*
 * open() should be called before multiple threads access lime_source.
 */
int lime_source::open(char *subdev) {

	// Magic number - reference taken from running USRP B210
	#define SAMPLES_PER_PACKET 2040

	unsigned int i, n, s_len;
	//should be large enough to hold all detected devices
	lms_info_str_t info_list[8];
	lms_range_t range_sr;

	if ((n = LMS_GetDeviceList(info_list)) < 0)
		fprintf(stderr, "LMS_GetDeviceList(NULL) failed\n");
	fprintf(stderr, "Devices found: %d\n", n);
	if (n < 1)
		return -1;

	// TODO: Handle case of multiple LimeSDRs

	fprintf(stderr, "Device info: %s\n", info_list[0]);
	//open the first device
	if (LMS_Open(&m_dev, info_list[0], NULL) != 0) {
		LMS_Close(m_dev);
		return -1;
	}

	//Initialize device with default configuration
	//Do not use if you want to keep existing configuration
	//Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
	if (LMS_Init(m_dev) != 0) {
		LMS_Close(m_dev);
		return -1;
	}

	// LimeSDR Mini does not support setting reference clock
	// Buggy: Setting of External clock gets stuck, so reset it first and then apply clock freq
	if (!strstr(info_list[0], "LimeSDR Mini")) {
		// Reset clock
		if (LMS_SetClockFreq(m_dev, LMS_CLOCK_EXTREF, -1.0) != 0) {
			fprintf(stderr, "LMS_SetClockFreq: failed to reset external clock frequency\n");
			return -1;
		}

		// Set external clock
		if (m_external_ref != -1.0) {
			fprintf(stderr, "Setting external reference clock to %f frequency\n", m_external_ref);
			if (LMS_SetClockFreq(m_dev, LMS_CLOCK_EXTREF, m_external_ref) != 0) {
				fprintf(stderr, "LMS_SetClockFreq: failed to set external clock frequency\n");
				return -1;
			}
		}
	}

	//Enable RX channel
	//Channels are numbered starting at 0
	if (LMS_EnableChannel(m_dev, LMS_CH_RX, 0, true) != 0) {
		fprintf(stderr, "LMS_EnableChannel: Failed to enable RX: 0 channel\n");
		return -1;
	}

	/* set samplerate */
	if (LMS_GetSampleRateRange(m_dev, LMS_CH_RX, &range_sr) != 0)
		return -1;
	print_range(&range_sr);

	// Decimation is set to 32 - refer LMSDevice.cpp in osmo-trx
	if (LMS_SetSampleRate(m_dev, m_desired_sample_rate, 32) != 0) {
		fprintf(stderr, "LMS_SetSampleRate: Failed to set RX sampling rate\n");
		return -1;
	}

	double sr_rf;
	if (LMS_GetSampleRate(m_dev, LMS_CH_RX, 0, &m_sample_rate, &sr_rf) != 0) {
		fprintf(stderr, "LMS_GetSampleRate: Failed to get RX sampling rate\n");
		return -1;
	}

	fprintf(stderr, "Sample rate: %f\n", m_sample_rate);

	set_antenna("LNAH");

	// RX gain to midpoint
	set_gain((maxRxGain() + minRxGain())/2);

	m_recv_samples_per_packet = SAMPLES_PER_PACKET;

	return 0;
}

double lime_source::maxRxGain()
{
	return 73.0;
}

double lime_source::minRxGain()
{
	return 0.0;
}

std::string handle_rx_err(lms_stream_status_t *status, bool &overrun) {

	overrun = false;
	std::ostringstream ost("");

	if (status->overrun == 0 && status->underrun == 0) {
		// Do nothing
	} else if (status->overrun > 0) {
		overrun = true;
		ost << "error: receive buffer is full (overrun)\n";
	} else if (status->underrun > 0) {
		ost << "error: receive buffer underrun\n";
	} else {
		ost << "error: unknown error\n";
	}

	return ost.str();
}

int lime_source::fill(unsigned int num_samples, unsigned int *overrun) {

	int16_t *ubuf = new int16_t[m_recv_samples_per_packet * 2];
	int num_smpls, expect_smpls;
	unsigned int i, j, space, overrun_cnt;
	complex *c;
	bool overrun_pkt = false;
	lms_stream_meta_t rx_metadata = {};
	rx_metadata.flushPartialPacket = false;
	rx_metadata.waitForTimestamp = false;

	overrun_cnt = 0;

	while ((m_cb->data_available() < num_samples)
			&& m_cb->space_available() > 0) {
		pthread_mutex_lock(&m_u_mutex);
		num_smpls = LMS_RecvStream(&m_rx_stream, ubuf, m_recv_samples_per_packet, &rx_metadata, 100);
		pthread_mutex_unlock(&m_u_mutex);

		lms_stream_status_t status;
		if (LMS_GetStreamStatus(&m_rx_stream, &status) != 0) {
			fprintf(stderr, "Rx LMS_GetStreamStatus failed\n");
		}

		std::string err_str = handle_rx_err(&status, overrun_pkt);
		if (overrun_pkt) {
			overrun_cnt++;
		}

		// write complex<short> input to complex<float> output
		c = (complex *)m_cb->poke(&space);

		// set space to number of complex items to copy
		if(space > m_recv_samples_per_packet)
			space = m_recv_samples_per_packet;

		// write data
		for(i = 0, j = 0; i < space; i += 1, j += 2)
			c[i] = complex(ubuf[j], ubuf[j + 1]);

		// update cb
		m_cb->wrote(i);
	}

	// if the cb is full, we left behind data from the usb packet
	if(m_cb->space_available() == 0) {
		fprintf(stderr, "warning: local overrun\n");
	}

	if (overrun)
		*overrun = overrun_cnt;

	return 0;
}


int lime_source::read(complex *buf, unsigned int num_samples, unsigned int *samples_read) {

	unsigned int n;

	if(fill(num_samples, 0))
		return -1;

	n = m_cb->read(buf, num_samples);

	if(samples_read)
		*samples_read = n;

	return 0;
}


/*
 * Don't hold a lock on this and use the lime at the same time.
 */
circular_buffer *lime_source::get_buffer() {

	return m_cb;
}


int lime_source::flush(unsigned int flush_count) {

	m_cb->flush();
	fill(flush_count, 0);
	m_cb->flush();

	return 0;
}
