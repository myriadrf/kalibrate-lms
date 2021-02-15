About Kalibrate
===============

Kalibrate is a GSM signal detector and clock frequency offset calculator.
Clock accuracy and frequency offset is an important aspect of GSM and cellular
networks. Out-of-specification clock accuracy causes a number of network issues
such as inconsistent network detection by the handset, handover failure, and
poor data and voice call performance.

LimeSuite Driver (LimeSuite):
  * https://github.com/myriadrf/LimeSuite

Release Notes

Support for LimeSDR and LimeSDR-Mini.

Build
=====

```
$ autoreconf -i
$ ./configure
$ make
$ cd src
```

Examples
========

LimeSDR (USB) with internal reference:

```
$ ./kal -s GSM900 -A LNAL
Devices found: 1
Device info: LimeSDR-USB, media=USB 3.0, module=FX3, addr=1d50:6108, serial=00090726074D2F1B
Reference clock 30.72 MHz
Disabling external reference clock
Sampling Rate Range: Min=100000.000000 Max=61440000.000000 Step=0.000000
kal: Scanning for GSM-900 base stations.
        chan: 2 (935.4MHz + 230Hz)      power: 673437.23


$ ./kal -f 935.4e6 -A LNAL
Devices found: 1
Device info: LimeSDR-USB, media=USB 3.0, module=FX3, addr=1d50:6108, serial=00090726074D2F1B
Reference clock 30.72 MHz
Disabling external reference clock
Sampling Rate Range: Min=100000.000000 Max=61440000.000000 Step=0.000000
kal: Calculating clock frequency offset.
Using GSM-900 channel 2 (935.4MHz)
average		[min, max]	(range, stddev)
+ 245Hz		[226, 263]	(37, 8.856748)
overruns: 0
not found: 12
```

LimeSDR (USB) with external 10MHz reference - Leo Bodnar GPS reference clock:

```
$ ./kal -s GSM900 -A LNAL -x 10.0e6
Devices found: 1
Device info: LimeSDR-USB, media=USB 3.0, module=FX3, addr=1d50:6108, serial=00090726074D2F1B
Reference clock 30.72 MHz
Sampling Rate Range: Min=100000.000000 Max=61440000.000000 Step=0.000000
kal: Scanning for GSM-900 base stations.
        chan: 1 (935.2MHz + 22.819kHz)  power: 295159.52                      
        chan: 2 (935.4MHz +  32Hz)      power: 793036.65


$ ./kal -f 935.4e6 -A LNAL -x 10.0e6
Devices found: 1
Device info: LimeSDR-USB, media=USB 3.0, module=FX3, addr=1d50:6108, serial=00090726074D2F1B
Reference clock 30.72 MHz
Sampling Rate Range: Min=100000.000000 Max=61440000.000000 Step=0.000000
kal: Calculating clock frequency offset.
Using GSM-900 channel 2 (935.4MHz)
average		[min, max]	(range, stddev)
+  25Hz		[8, 44]	(36, 9.328476)
overruns: 0
not found: 1
```

LimeSDR-Mini with internal reference:

```
$ ./kal -s GSM900 -A LNAW
Devices found: 1
Device info: LimeSDR Mini, media=USB 3.0, module=FT601, addr=24607:1027, serial=1D588161783274
Reference clock 40.00 MHz
Sampling Rate Range: Min=100000.000000 Max=30720000.000000 Step=0.000000
kal: Scanning for GSM-900 base stations.
        chan: 2 (935.4MHz - 128Hz)      power: 329156.49                      
        chan: 92 (953.4MHz - 167Hz)     power: 33095.20


$ ./kal -f 935.4e6 -A LNAW
Devices found: 1
Device info: LimeSDR Mini, media=USB 3.0, module=FT601, addr=24607:1027, serial=1D588161783274
Reference clock 40.00 MHz
Sampling Rate Range: Min=100000.000000 Max=30720000.000000 Step=0.000000
kal: Calculating clock frequency offset.
Using GSM-900 channel 2 (935.4MHz)
average		[min, max]	(range, stddev)
-  83Hz		[-158, -14]	(144, 37.776100)
overruns: 0
not found: 73
```

Authors
=======

Kalibrate was originally written by Joshua Lackey <jl@thre.at> in 2010.
Subsequent UHD device support and other changes were added by
Tom Tsou <tom@tsou.cc>.
Subsequent LimeSDR device support and other changes were added by Supreeth Herle <herlesupreeth@gmail.com>
