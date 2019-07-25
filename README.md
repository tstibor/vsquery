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
	       Date in format yyyy-mm-dd, e.g. '2017-06-30' [default: current date]
	-s, --stepjd <float>
	       Partition tabular data by step size in julian date [default: 0.01]
version: 0.0.2-7 © 2017 by Thomas Stibor <thomas@stibor.net>
```
If no options and parameters are provided, then the default settings are used and the following results are calculated
```
>./src/vsquery
┌───────────────────────────────────────────── observer ──────────────────────────────────────────────┐
│lat,lng                                                                          50.146145,  8.498569│
├───────────────────────────────────────────── date ──────────────────────────────────────────────────┤
│julian                                                                                 2458690.394233│
│calendar (LT)                                                                     2019-07-25 21:27:41│
├───────────────────────────────────────────── sun ───────────────────────────────────────────────────┤
│↓ set                                                                             2019-07-25 21:18:50│
│↑ rise                                                                            2019-07-26 05:44:29│
│night length (hours)                                                                            8.428│
├───────────────────────────────────────────── twilight ──────────────────────────────────────────────┤
│amateur astronomy begin                                                           2019-07-25 23:29:05│
│amateur astronomy end                                                             2019-07-26 03:34:13│
│amateur astronomy night length (hours)                                                          4.086│
│professional astronomy begin                                                      2019-07-26 00:16:42│
│professional astronomy end                                                        2019-07-26 02:46:36│
│professional astronomy night length (hours)                                                     2.498│
├───────────────────────────────────────────── moon ──────────────────────────────────────────────────┤
│↓ set                                                                             2019-07-25 14:24:31│
│↑ rise                                                                            2019-07-26 01:06:22│
│↔ transit                                                                         2019-07-26 08:12:23│
│disk illuminated fraction                                                                    0.473606│
│distance to earth                                                                       395144.791 km│
└─────────────────────────────────────────────────────────────────────────────────────────────────────┘
```
For checking e.g. the visibility of [NGC2244](http://simbad.u-strasbg.fr/simbad/sim-id?Ident=ngc+2244) and [WASP2](http://simbad.u-strasbg.fr/simbad/sim-id?Ident=wasp+2) at La Palma, Roque de los Muchachos at date: *2017-10-28* execute
```
>./src/vsquery --latitude 28.755620 --longitude -17.885742 --date 2017-10-28 ngc2244 wasp2
┌───────────────────────────────────────────── observer ──────────────────────────────────────────────┐
│lat,lng                                                                          28.755619,-17.885742│
├───────────────────────────────────────────── date ──────────────────────────────────────────────────┤
│julian                                                                                 2458055.395515│
│calendar (LT)                                                                     2017-10-28 21:29:32│
├───────────────────────────────────────────── sun ───────────────────────────────────────────────────┤
│↓ set                                                                             2017-10-28 20:28:32│
│↑ rise                                                                            2017-10-29 09:20:34│
│night length (hours)                                                                           12.867│
├───────────────────────────────────────────── twilight ──────────────────────────────────────────────┤
│amateur astronomy begin                                                           2017-10-28 21:35:11│
│amateur astronomy end                                                             2017-10-29 08:13:49│
│amateur astronomy night length (hours)                                                         10.644│
│professional astronomy begin                                                      2017-10-28 21:49:33│
│professional astronomy end                                                        2017-10-29 07:59:25│
│professional astronomy night length (hours)                                                    10.164│
├───────────────────────────────────────────── moon ──────────────────────────────────────────────────┤
│↑ rise                                                                            2017-10-28 16:19:22│
│↔ transit                                                                         2017-10-28 21:51:58│
│↓ set                                                                             2017-10-29 03:28:03│
│disk illuminated fraction                                                                    0.539893│
│distance to earth                                                                       398780.857 km│
└─────────────────────────────────────────────────────────────────────────────────────────────────────┘
┌───────────────────────────────────────────── object 'ngc2244' ──────────────────────────────────────┐
│ra, dec                                  (097.979167, 004.941667) (06:31:55.000, +04:56:30.000) J2000│
│↓ set                                                                             2017-10-28 13:28:56│
│↑ rise                                                                            2017-10-29 01:00:04│
│↔ transit                                                                         2017-10-29 07:12:32│
│                                                                                                     │
├ airmass ──┬── azimuth ──┬── altitude ──┬────  date ────┬──── time ────┬── moon separation (degrees) ┤
│15.76      │  266.03     │    3.04      │  2017-10-29   │   01:16:32   │   138.0121                  │
│ 8.79      │  267.75     │    6.20      │  2017-10-29   │   01:30:56   │   137.8981                  │
│ 6.00      │  269.48     │    9.36      │  2017-10-29   │   01:45:20   │   137.7841                  │
│ 4.55      │  271.22     │   12.53      │  2017-10-29   │   01:59:44   │   137.6700                  │
│ 3.67      │  272.98     │   15.69      │  2017-10-29   │   02:14:08   │   137.5558                  │
│ 3.08      │  274.79     │   18.85      │  2017-10-29   │   02:28:32   │   137.4415                  │
│ 2.66      │  276.64     │   21.99      │  2017-10-29   │   02:42:56   │   137.3272                  │
│ 2.35      │  278.56     │   25.13      │  2017-10-29   │   02:57:20   │   137.2128                  │
│ 2.11      │  280.56     │   28.25      │  2017-10-29   │   03:11:44   │   137.0984                  │
│ 1.92      │  282.67     │   31.35      │  2017-10-29   │   03:26:08   │   136.9838                  │
│ 1.77      │  284.89     │   34.43      │  2017-10-29   │   03:40:32   │   136.8692                  │
│ 1.64      │  287.26     │   37.47      │  2017-10-29   │   03:54:56   │   136.7546                  │
│ 1.54      │  289.81     │   40.47      │  2017-10-29   │   04:09:20   │   136.6398                  │
│ 1.45      │  292.58     │   43.42      │  2017-10-29   │   04:23:44   │   136.5250                  │
│ 1.38      │  295.60     │   46.31      │  2017-10-29   │   04:38:08   │   136.4102                  │
│ 1.32      │  298.93     │   49.12      │  2017-10-29   │   04:52:32   │   136.2952                  │
│ 1.27      │  302.64     │   51.84      │  2017-10-29   │   05:06:56   │   136.1802                  │
│ 1.23      │  306.78     │   54.44      │  2017-10-29   │   05:21:20   │   136.0652                  │
│ 1.19      │  311.44     │   56.90      │  2017-10-29   │   05:35:44   │   135.9500                  │
│ 1.16      │  316.70     │   59.17      │  2017-10-29   │   05:50:08   │   135.8348                  │
│ 1.14      │  322.65     │   61.22      │  2017-10-29   │   06:04:32   │   135.7196                  │
│ 1.12      │  329.35     │   62.99      │  2017-10-29   │   06:18:56   │   135.6042                  │
│ 1.11      │  336.80     │   64.43      │  2017-10-29   │   06:33:20   │   135.4888                  │
│ 1.10      │  344.94     │   65.47      │  2017-10-29   │   06:47:44   │   135.3734                  │
│ 1.09      │  353.59     │   66.06      │  2017-10-29   │   07:02:08   │   135.2578                  │
│ 1.09      │    2.48     │   66.17      │  2017-10-29   │   07:16:32   │   135.1423                  │
│ 1.10      │   11.27     │   65.79      │  2017-10-29   │   07:30:56   │   135.0266                  │
│ 1.10      │   19.66     │   64.94      │  2017-10-29   │   07:45:20   │   134.9109                  │
│ 1.12      │   27.43     │   63.68      │  2017-10-29   │   07:59:44   │   134.7951                  │
│ 1.13      │   34.47     │   62.05      │  2017-10-29   │   08:14:08   │   134.6792                  │
│ 1.15      │   40.75     │   60.11      │  2017-10-29   │   08:28:32   │   134.5633                  │
│ 1.18      │   46.31     │   57.93      │  2017-10-29   │   08:42:56   │   134.4473                  │
│ 1.21      │   51.23     │   55.55      │  2017-10-29   │   08:57:20   │   134.3313                  │
│ 1.25      │   55.59     │   53.01      │  2017-10-29   │   09:11:44   │   134.2152                  │
└───────────┴─────────────┴──────────────┴───────────────┴──────────────┴─────────────────────────────┘
┌───────────────────────────────────────────── object 'wasp2' ────────────────────────────────────────┐
│ra, dec                                  (307.725533, 006.429538) (20:30:54.128, +06:25:46.338) J2000│
│↑ rise                                                                            2017-10-28 14:57:24│
│↔ transit                                                                         2017-10-28 21:13:10│
│↓ set                                                                             2017-10-29 03:28:55│
│                                                                                                     │
├ airmass ──┬── azimuth ──┬── altitude ──┬────  date ────┬──── time ────┬── moon separation (degrees) ┤
│ 1.10      │  332.53     │   65.30      │  2017-10-28   │   20:28:32   │    24.1229                  │
│ 1.09      │  340.78     │   66.55      │  2017-10-28   │   20:42:56   │    24.1464                  │
│ 1.08      │  349.71     │   67.36      │  2017-10-28   │   20:57:20   │    24.1706                  │
│ 1.08      │  359.07     │   67.67      │  2017-10-28   │   21:11:44   │    24.1953                  │
│ 1.08      │    8.46     │   67.46      │  2017-10-28   │   21:26:08   │    24.2205                  │
│ 1.09      │   17.51     │   66.75      │  2017-10-28   │   21:40:32   │    24.2464                  │
│ 1.10      │   25.91     │   65.58      │  2017-10-28   │   21:54:56   │    24.2728                  │
│ 1.11      │   33.49     │   64.01      │  2017-10-28   │   22:09:20   │    24.2997                  │
│ 1.13      │   40.21     │   62.11      │  2017-10-28   │   22:23:44   │    24.3272                  │
│ 1.16      │   46.12     │   59.94      │  2017-10-28   │   22:38:08   │    24.3553                  │
│ 1.18      │   51.30     │   57.56      │  2017-10-28   │   22:52:32   │    24.3839                  │
│ 1.22      │   55.85     │   55.01      │  2017-10-28   │   23:06:56   │    24.4131                  │
│ 1.26      │   59.88     │   52.33      │  2017-10-28   │   23:21:20   │    24.4429                  │
│ 1.31      │   63.47     │   49.55      │  2017-10-28   │   23:35:44   │    24.4732                  │
│ 1.37      │   66.69     │   46.68      │  2017-10-28   │   23:50:08   │    24.5040                  │
│ 1.45      │   69.61     │   43.74      │  2017-10-29   │   00:04:32   │    24.5354                  │
│ 1.53      │   72.29     │   40.75      │  2017-10-29   │   00:18:56   │    24.5674                  │
│ 1.63      │   74.76     │   37.71      │  2017-10-29   │   00:33:20   │    24.5999                  │
│ 1.76      │   77.06     │   34.64      │  2017-10-29   │   00:47:44   │    24.6329                  │
│ 1.91      │   79.22     │   31.55      │  2017-10-29   │   01:02:08   │    24.6665                  │
│ 2.10      │   81.27     │   28.43      │  2017-10-29   │   01:16:32   │    24.7007                  │
│ 2.33      │   83.22     │   25.29      │  2017-10-29   │   01:30:56   │    24.7353                  │
│ 2.64      │   85.10     │   22.14      │  2017-10-29   │   01:45:20   │    24.7706                  │
│ 3.06      │   86.92     │   18.99      │  2017-10-29   │   01:59:44   │    24.8063                  │
│ 3.64      │   88.69     │   15.83      │  2017-10-29   │   02:14:08   │    24.8426                  │
│ 4.50      │   90.44     │   12.66      │  2017-10-29   │   02:28:32   │    24.8794                  │
│ 5.92      │   92.16     │    9.50      │  2017-10-29   │   02:42:56   │    24.9167                  │
│ 8.62      │   93.87     │    6.34      │  2017-10-29   │   02:57:20   │    24.9546                  │
│15.23      │   95.59     │    3.18      │  2017-10-29   │   03:11:44   │    24.9930                  │
│38.23      │   97.32     │    0.04      │  2017-10-29   │   03:26:08   │    25.0319                  │
└───────────┴─────────────┴──────────────┴───────────────┴──────────────┴─────────────────────────────┘
```
For each parameterized object, the rise, transit and set, as well as the
azimuth, altitude and moon distance separation is determined from the time of sun set to the time of sun rise.

## License
This project is licensed under the [GPL2 license](http://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).
