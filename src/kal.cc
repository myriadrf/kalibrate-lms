/*
 * Copyright (c) 2010, Joshua Lackey
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

/*
 * kal
 *
 *    Two functions:
 *
 * 	1.  Calculates the frequency offset between a local GSM tower and the
 * 	    LimeSDR clock.
 *
 *	2.  Identifies the frequency of all GSM base stations in a given band.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_VERSION "custom build"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef D_HOST_OSX
#include <libgen.h>
#endif /* D_HOST_OSX */
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <libgen.h>

#include "lime_source.h"
#include "fcch_detector.h"
#include "arfcn_freq.h"
#include "offset.h"
#include "c0_detect.h"
#include "version.h"

static const double GSM_RATE = 1625000.0 / 6.0;


int g_verbosity = 0;
int g_debug = 0;

void usage(char *prog) {

	printf("kalibrate v%s, Copyright (c) 2010, Joshua Lackey\n", kal_version_string);
	printf("\nUsage:\n");
	printf("\tGSM Base Station Scan:\n");
	printf("\t\t%s <-s band indicator> [options]\n", basename(prog));
	printf("\n");
	printf("\tClock Offset Calculation:\n");
	printf("\t\t%s <-f frequency | -c channel> [options]\n", basename(prog));
	printf("\n");
	printf("Where options are:\n");
	printf("\t-s\tband to scan (GSM850, GSM900, EGSM, DCS, PCS)\n");
	printf("\t-f\tfrequency of nearby GSM base station\n");
	printf("\t-c\tchannel of nearby GSM base station\n");
	printf("\t-b\tband indicator (GSM850, GSM900, EGSM, DCS, PCS)\n");
	printf("\t-R\tRX subdev spec (Not Supported)\n");
	printf("\t-A\tantenna LNAH or LNAL or LNAW, defaults to LNAH\n");
	printf("\t-g\tgain (0.0 - 73.0), defaults to 36.5\n");
	printf("\t-x\texternal reference input in Hz\n");
	printf("\t-v\tverbose\n");
	printf("\t-D\tenable debug messages\n");
	printf("\t-h\thelp\n");
	exit(-1);
}


int main(int argc, char **argv) {

	char *endptr;
	int c, bi = BI_NOT_DEFINED, chan = -1, bts_scan = 0;
	char *antenna_args = NULL;
	char *subdev = NULL;
	double fpga_master_clock_freq = 30.72e6;
	double external_ref = -1.0;
	float gain = 36.5;
	double freq = -1.0, fd;
	lime_source *u;

	while((c = getopt(argc, argv, "f:c:s:b:R:A:g:F:x:vDh?")) != EOF) {
		switch(c) {
			case 'f':
				freq = strtod(optarg, 0);
				break;

			case 'c':
				chan = strtoul(optarg, 0, 0);
				break;

			case 's':
				if((bi = str_to_bi(optarg)) == -1) {
					fprintf(stderr, "error: bad band "
					   "indicator: ``%s''\n", optarg);
					usage(argv[0]);
				}
				bts_scan = 1;
				break;

			case 'b':
				if((bi = str_to_bi(optarg)) == -1) {
					fprintf(stderr, "error: bad band "
					   "indicator: ``%s''\n", optarg);
					usage(argv[0]);
				}
				break;

			case 'R':
				errno = 0;
				subdev = optarg;
				break;

			case 'A':
				if (!strcmp(optarg, "LNAH") || !strcmp(optarg, "LNAW") || !strcmp(optarg, "LNAL")) {
					antenna_args = optarg;
				} else {
					fprintf(stderr, "error: bad "
						   "antenna: ``%s''\n",
						   optarg);
						usage(argv[0]);
				}
				break;

			case 'g':
				gain = strtod(optarg, 0);
				if((gain < 0.0) || (73.0 < gain))
					usage(argv[0]);
				break;

			case 'F':
				fpga_master_clock_freq = strtod(optarg, 0);
				break;

			case 'x':
				external_ref = strtod(optarg, 0);
				break;

			case 'v':
				g_verbosity++;
				break;

			case 'D':
				g_debug = 1;
				break;

			case 'h':
			case '?':
			default:
				usage(argv[0]);
				break;
		}
	}

	// sanity check frequency / channel
	if(bts_scan) {
		if(bi == BI_NOT_DEFINED) {
			fprintf(stderr, "error: scaning requires band\n");
			usage(argv[0]);
		}
	} else {
		if(freq < 0.0) {
			if(chan < 0) {
				fprintf(stderr, "error: must enter channel or "
				   "frequency\n");
				usage(argv[0]);
			}
			if((freq = arfcn_to_freq(chan, &bi)) < 869e6)
				usage(argv[0]);
		}
		if((freq < 869e6) || (2e9 < freq)) {
			fprintf(stderr, "error: bad frequency: %lf\n", freq);
			usage(argv[0]);
		}
		chan = freq_to_arfcn(freq, &bi);
	}

	if(g_debug) {
#ifdef D_HOST_OSX
		printf("debug: Mac OS X version\n");
#endif
		printf("debug: FPGA Master Clock Freq:\t%f\n", fpga_master_clock_freq);
		printf("debug: External Reference    :\t%s\n", external_ref != -1.0 ? "Yes" : "No");
		printf("debug: RX Subdev Spec        :\t%s\n", subdev? subdev : "");
		printf("debug: Antenna               :\t%s\n", antenna_args? antenna_args : "LNAH");
		printf("debug: Gain                  :\t%f\n", gain);
	}

	// let the device decide on the decimation
	u = new lime_source(GSM_RATE, fpga_master_clock_freq, external_ref);
	if(!u) {
		fprintf(stderr, "error: radio_source\n");
		return -1;
	}
	if(u->open(subdev) == -1) {
		fprintf(stderr, "error: radio_source::open\n");
		return -1;
	}
	if (antenna_args) {
		u->set_antenna(antenna_args);
	}
	if(!u->set_gain(gain)) {
		fprintf(stderr, "error: radio_source::set_gain\n");
		return -1;
	}

	if(!bts_scan) {
		if(u->tune(freq) == -1) {
			fprintf(stderr, "error: radio_source::tune\n");
			return -1;
		}

		fprintf(stderr, "%s: Calculating clock frequency offset.\n",
		   basename(argv[0]));
		fprintf(stderr, "Using %s channel %d (%.1fMHz)\n",
		   bi_to_str(bi), chan, freq / 1e6);

		if (external_ref == -1.0) {
			float off;
			uint16_t dac = (uint16_t) u->get_board_dac();
			uint16_t delta = 1;
			int max = 50;
			float lowest = 100e6;
			uint16_t dac_l = 0;
			do {
				fprintf(stderr, "================================================\n");
				u->tune_dac(dac);
				offset_detect(u, &off);
				if (fabs(off) < fabs(lowest)) {
					dac_l = dac;
					lowest = off;
				}
				if (fabs(off) < 50) {
					fprintf(stderr, "\nTest DAC trim value in range [%d-%d]\n", dac-6, dac+6);
					for (int i = -6; i < 6; i++) {
						fprintf(stderr, "================================================\n");
						u->tune_dac(dac + i);
						offset_detect(u, &off);
						if (fabs(off) < fabs(lowest)) {
							dac_l = dac + i;
							lowest = off;
						}
					}
					break;
				}

				if (off < 0)
					dac -= delta;
				if (off > 0)
					dac += delta;
				if (off == 0)
					break;

				if (max-- < 0) break;
			} while (true);
			fprintf(stderr, "Found lowest offset of %fHz at %fMHz (%f ppm) using DAC trim %u\n", lowest, freq/1e6, lowest/freq*1e6, dac_l);
			u->tune_dac(dac_l);
		} else {
			offset_detect(u, NULL);
		}

		delete u;

		return 0;
	}

	fprintf(stderr, "%s: Scanning for %s base stations.\n",
	   basename(argv[0]), bi_to_str(bi));

	c0_detect(u, bi);

	delete u;

	return 0;
}
