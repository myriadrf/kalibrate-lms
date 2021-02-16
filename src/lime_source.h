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

#include "config.h"

#include <lime/LimeSuite.h>

#include "complex.h"
#include "circular_buffer.h"


class lime_source {
public:
	lime_source(double sample_rate,
			double fpga_master_clock_freq = 0.0,
			double external_ref = -1.0);

	~lime_source();

	int open(char *subdev);
	int read(complex *buf,
		unsigned int num_samples,
		unsigned int *samples_read);

	int fill(unsigned int num_samples, unsigned int *overrun);
	int tune(double freq);
	void set_antenna(const std::string antenna);
	bool set_gain(double gain);
	void start();
	void stop();
	double maxRxGain();
	double minRxGain();
	int flush(unsigned int flush_count = FLUSH_COUNT);
	void tune_dac(uint16_t dacVal);
	double get_board_dac();
	circular_buffer *get_buffer();

	double sample_rate();

private:
	lms_device_t        *m_dev;
	lms_stream_t		m_rx_stream;

	double				m_sample_rate;
	double				m_desired_sample_rate;
	double				m_external_ref;
	unsigned int        m_recv_samples_per_packet;
	double				m_fpga_master_clock_freq;

	circular_buffer		*m_cb;

	/*
	 * This mutex protects access to the lime
	 */
	pthread_mutex_t		m_u_mutex;

	static const unsigned int	FLUSH_COUNT	= 10;
	static const unsigned int	CB_LEN		= (1 << 20);
	static const int			NCHAN		= 1;
};
