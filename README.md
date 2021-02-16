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
Sample rate: 270833.332607
kal: Scanning for GSM-900 base stations.
        chan: 2 (935.4MHz - 150Hz)      power: 470311.61
        chan: 86 (952.2MHz +  60Hz)     power: 230830.63


$ ./kal -f 935.4e6 -A LNAL
Devices found: 1
Device info: LimeSDR-USB, media=USB 3.0, module=FX3, addr=1d50:6108, serial=00090726074D2F1B
Reference clock 30.72 MHz
Disabling external reference clock
Sampling Rate Range: Min=100000.000000 Max=61440000.000000 Step=0.000000
Sample rate: 270833.332607
kal: Calculating clock frequency offset.
Using GSM-900 channel 2 (935.4MHz)
================================================
VCTCXO DAC value set to: 127.000000
average		[min, max]	(range, stddev)
-  25Hz		[-66, 14]	(80, 21.595409)
overruns: 0
not found: 0

Test DAC trim value in range [121-133]
================================================
VCTCXO DAC value set to: 121.000000
average		[min, max]	(range, stddev)
+ 587Hz		[547, 639]	(92, 25.123407)
overruns: 0
not found: 0
================================================
VCTCXO DAC value set to: 122.000000
average		[min, max]	(range, stddev)
+ 502Hz		[461, 544]	(83, 21.423199)
overruns: 0
not found: 3
================================================
VCTCXO DAC value set to: 123.000000
average		[min, max]	(range, stddev)
+ 393Hz		[350, 434]	(85, 22.113863)
overruns: 0
not found: 0
================================================
VCTCXO DAC value set to: 124.000000
average		[min, max]	(range, stddev)
+ 292Hz		[243, 333]	(90, 23.192686)
overruns: 0
not found: 0
================================================
VCTCXO DAC value set to: 125.000000
average		[min, max]	(range, stddev)
+ 196Hz		[138, 245]	(107, 26.214304)
overruns: 0
not found: 0
================================================
VCTCXO DAC value set to: 126.000000
average		[min, max]	(range, stddev)
+  79Hz		[28, 132]	(103, 29.041769)
overruns: 0
not found: 4
================================================
VCTCXO DAC value set to: 127.000000
average		[min, max]	(range, stddev)
-  22Hz		[-78, 27]	(105, 28.535673)
overruns: 0
not found: 2
================================================
VCTCXO DAC value set to: 128.000000
average		[min, max]	(range, stddev)
- 123Hz		[-170, -81]	(89, 23.122004)
overruns: 0
not found: 1
================================================
VCTCXO DAC value set to: 129.000000
average		[min, max]	(range, stddev)
- 233Hz		[-273, -198]	(75, 18.825752)
overruns: 0
not found: 1
================================================
VCTCXO DAC value set to: 130.000000
average		[min, max]	(range, stddev)
- 336Hz		[-370, -295]	(75, 17.255051)
overruns: 0
not found: 2
================================================
VCTCXO DAC value set to: 131.000000
average		[min, max]	(range, stddev)
- 441Hz		[-482, -399]	(83, 20.968220)
overruns: 0
not found: 0
================================================
VCTCXO DAC value set to: 132.000000
average		[min, max]	(range, stddev)
- 543Hz		[-585, -504]	(82, 23.805302)
overruns: 0
not found: 0
Found lowest offset of -21.564095Hz at 935.400000MHz (-0.023053 ppm) using DAC trim 127
VCTCXO DAC value set to: 127.000000
```

LimeSDR (USB) with external 10MHz reference - Leo Bodnar GPS reference clock:

```
$ ./kal -s GSM900 -A LNAL -x 10.0e6
Devices found: 1
Device info: LimeSDR-USB, media=USB 3.0, module=FX3, addr=1d50:6108, serial=00090726074D2F1B
Reference clock 30.72 MHz
Disabling external reference clock
Setting external reference clock to 10000000.000000 frequency
Sampling Rate Range: Min=100000.000000 Max=61440000.000000 Step=0.000000
kal: Scanning for GSM-900 base stations.
        chan: 1 (935.2MHz + 22.819kHz)  power: 295159.52      
        chan: 2 (935.4MHz +  32Hz)      power: 793036.65


$ ./kal -f 935.4e6 -A LNAL -x 10.0e6
Devices found: 1
Device info: LimeSDR-USB, media=USB 3.0, module=FX3, addr=1d50:6108, serial=00090726074D2F1B
Reference clock 30.72 MHz
Disabling external reference clock
Setting external reference clock to 10000000.000000 frequency
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
$ ./kal -s GSM900 -A LNAW -g 45
Devices found: 1
Device info: LimeSDR Mini, media=USB 3.0, module=FT601, addr=24607:1027, serial=1D588161783274
Reference clock 40.00 MHz
Sampling Rate Range: Min=100000.000000 Max=30720000.000000 Step=0.000000
Sample rate: 270833.330495
kal: Scanning for GSM-900 base stations.
        chan: 3 (935.6MHz -   10Hz)     power: 230188.16
        chan: 4 (935.8MHz + 128Hz)      power: 128849.91
        chan: 86 (952.2MHz +  51Hz)     power: 108510.78
        chan: 112 (957.4MHz + 22.956kHz)        power: 89879.75
        chan: 119 (958.8MHz + 31.935kHz)        power: 87845.77


$ ./kal -f 935.8e6 -A LNAW -g 45
Devices found: 1
Device info: LimeSDR Mini, media=USB 3.0, module=FT601, addr=24607:1027, serial=1D588161783274
Reference clock 40.00 MHz
Sampling Rate Range: Min=100000.000000 Max=30720000.000000 Step=0.000000
Sample rate: 270833.330495
kal: Calculating clock frequency offset.
Using GSM-900 channel 4 (935.8MHz)
================================================
VCTCXO DAC value set to: 178.000000
average		[min, max]	(range, stddev)
-  32Hz		[-108, 40]	(148, 38.195812)
overruns: 0
not found: 24

Test DAC trim value in range [172-184]
================================================
VCTCXO DAC value set to: 172.000000
average		[min, max]	(range, stddev)
+ 473Hz		[391, 545]	(154, 41.568333)
overruns: 0
not found: 9
================================================
VCTCXO DAC value set to: 173.000000
average		[min, max]	(range, stddev)
+ 385Hz		[314, 456]	(143, 37.254765)
overruns: 0
not found: 21
================================================
VCTCXO DAC value set to: 174.000000
average		[min, max]	(range, stddev)
+ 300Hz		[222, 372]	(151, 35.989777)
overruns: 0
not found: 7
================================================
VCTCXO DAC value set to: 175.000000
average		[min, max]	(range, stddev)
+ 221Hz		[146, 279]	(133, 38.328632)
overruns: 0
not found: 6
================================================
VCTCXO DAC value set to: 176.000000
average		[min, max]	(range, stddev)
+ 128Hz		[57, 184]	(127, 36.148319)
overruns: 0
not found: 9
================================================
VCTCXO DAC value set to: 177.000000
average		[min, max]	(range, stddev)
+  36Hz		[-28, 102]	(130, 36.664257)
overruns: 0
not found: 14
================================================
VCTCXO DAC value set to: 178.000000
average		[min, max]	(range, stddev)
-  48Hz		[-109, 19]	(128, 31.921885)
overruns: 0
not found: 13
================================================
VCTCXO DAC value set to: 179.000000
average		[min, max]	(range, stddev)
- 131Hz		[-198, -65]	(133, 36.958584)
overruns: 0
not found: 15
================================================
VCTCXO DAC value set to: 180.000000
average		[min, max]	(range, stddev)
- 203Hz		[-274, -109]	(165, 40.602825)
overruns: 0
not found: 65
================================================
VCTCXO DAC value set to: 181.000000
average		[min, max]	(range, stddev)
- 295Hz		[-377, -224]	(153, 39.257061)
overruns: 0
not found: 36
================================================
VCTCXO DAC value set to: 182.000000
average		[min, max]	(range, stddev)
- 389Hz		[-476, -311]	(164, 42.100746)
overruns: 0
not found: 49
================================================
VCTCXO DAC value set to: 183.000000
average		[min, max]	(range, stddev)
- 482Hz		[-573, -375]	(198, 54.824726)
overruns: 0
not found: 134
Found lowest offset of -31.985970Hz at 935.800000MHz (-0.034180 ppm) using DAC trim 178
VCTCXO DAC value set to: 178.000000
```

Authors
=======

Kalibrate was originally written by Joshua Lackey <jl@thre.at> in 2010.
Subsequent UHD device support and other changes were added by
Tom Tsou <tom@tsou.cc>.
Subsequent LimeSDR device support and other changes were added by Supreeth Herle <herlesupreeth@gmail.com>
