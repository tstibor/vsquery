Command line tool for determining visibility of astronomical objects
==============

[![Tag Version](https://img.shields.io/github/tag/tstibor/vsquery.svg)](https://github.com/tstibor/vsquery/tags)
[![License](http://img.shields.io/:license-gpl2-blue.svg)](http://www.gnu.org/licenses/gpl-2.0.html)

The command line tool *vsquery* calculates based on observers latitude and longitude coordinates the
* set and rise of the sun
* set, transit and set of the moon
* moons disk illumination and moon-earth distance
* begin of twilight such that it is dark enough for most amateur astronomical observations
* begin of twilight such that the sky is completely dark for professional astronomical observations
* end of twilight for professional astronomical observations
* end of twilight for most amateur astronomical observations
* rise, transit and set of arbitrary astronomical objects by online querying the sesame database
* calculating azimuth and altitude of the astronomical objects starting from sun set to sun rise

## Getting Started <a id="getting.started"></a>
Make sure to install the development package of [libnova](http://libnova.sourceforge.net/) and [libcurl](https://curl.haxx.se/libcurl/). Checkout out the repository and execute
```
./autogen.sh && ./configure
```
## Usage <a id="usage"></a>
Execute *vsquery* with parameter `--help` to get a parameter and default settings overview
```
>./src/vsquery --help
usage: ./src/vsquery [option] <object 1> <object 2> .... <object N>
	-a, --latitude <float>
	       Latitude coordinate in decimal degree [default: 50.146145]
	-o, --longitude <float>
	       Longitude coordinate in decimal degree [default: 8.498569]
	-d, --date <string>
	       Date in format yyyy-mm-dd, e.g. '2017-06-30' [default: current date and time]
version: 0.0.1 © 2017 by Thomas Stibor <thomas@stibor.net>
```
If no options and parameters are provided, then the default settings are used and the following results are calculated
```
>./src/vsquery
##### observer #
(lat,lng)       : (50.146145,8.498569)
##### date #####
julian          : 2457976.271315
calendar        : 2017-08-10 18:30:41
##### sun ######
set             : 2017-08-10 20:53:11
rise            : 2017-08-11 06:08:03
twilight amateur astro. begin     : 2017-08-10 22:45:50
twilight professional astro. begin: 2017-08-10 23:17:58
twilight professional astro. end  : 2017-08-11 03:43:12
twilight amateur astro. end       : 2017-08-11 04:15:20
##### moon #####
rise            : 2017-08-10 22:15:50
transit         : 2017-08-11 04:03:48
set             : 2017-08-11 10:01:56
disk            : 0.906064 (illuminated fraction of the moons disk, value between 0 and 1)
lunar earth dist: 384569.034 km
```
For checking e.g. the visibility of [NGC2244](http://simbad.u-strasbg.fr/simbad/sim-id?Ident=ngc+2244) and [WASP2](http://simbad.u-strasbg.fr/simbad/sim-id?Ident=wasp+2) at La Palma, Roque de los Muchachos at date: *2017-10-28* execute
```
>./src/vsquery --latitude 28.755620 --longitude -17.885742 --date 2017-10-28 ngc2244 wasp2
##### observer #
(lat,lng)       : (28.755619,-17.885742)
##### date #####
julian          : 2458055.289469
calendar        : 2017-10-28 18:56:50
##### sun ######
set             : 2017-10-28 20:28:32
rise            : 2017-10-29 09:20:34
twilight amateur astro. begin     : 2017-10-28 21:35:11
twilight professional astro. begin: 2017-10-28 21:49:33
twilight professional astro. end  : 2017-10-29 07:59:25
twilight amateur astro. end       : 2017-10-29 08:13:49
##### moon #####
rise            : 2017-10-28 16:19:22
transit         : 2017-10-28 21:51:58
set             : 2017-10-29 03:28:03
disk            : 0.539893 (illuminated fraction of the moons disk, value between 0 and 1)
lunar earth dist: 398780.857 km
#### object ngc2244 ####
set    : 2017-10-28 13:28:56
rise   : 2017-10-29 01:00:05
transit: 2017-10-29 07:12:32
azimuth     altitude	date	   time		moon separation
210.8961   -51.8642	2017-10-28 20:28:32	140.279113 (degrees)
215.8962   -50.1213	2017-10-28 20:42:56	140.166595 (degrees)
220.5094   -48.1633	2017-10-28 20:57:20	140.054002 (degrees)
...
...
 46.3052    57.9345	2017-10-29 08:42:56	134.449912 (degrees)
 51.2244    55.5535	2017-10-29 08:57:20	134.333869 (degrees)
 55.5861    53.0119	2017-10-29 09:11:44	134.217763 (degrees)
#### object wasp2 ####
set    : 2017-10-29 03:28:55
rise   : 2017-10-28 14:57:24
transit: 2017-10-28 21:13:10
azimuth     altitude	date	   time		moon separation
332.5314    65.2973	2017-10-28 20:28:32	24.122893 (degrees)
340.7766    66.5537	2017-10-28 20:42:56	24.146450 (degrees)
349.7148    67.3614	2017-10-28 20:57:20	24.170577 (degrees)
...
...
167.9211   -54.1591	2017-10-29 08:42:56	26.016798 (degrees)
174.0270   -54.6561	2017-10-29 08:57:20	26.067205 (degrees)
180.2360   -54.8146	2017-10-29 09:11:44	26.118085 (degrees)
```
For each parameterized object, the rise, transit and set, as well as the
azimuth, altitude and moon distance separation is determined from the time of sun set to the time of sun rise.

## License
This project is licensed under the [GPL2 license](http://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).
