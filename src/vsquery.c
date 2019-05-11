/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (c) 2017 Thomas Stibor <thomas@stibor.net>
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <curl/curl.h>
#include <libnova/solar.h>
#include <libnova/lunar.h>
#include <libnova/julian_day.h>
#include <libnova/rise_set.h>
#include <libnova/transform.h>
#include <libnova/utility.h>
#include <libnova/angular_separation.h>
#include <libnova/airmass.h>

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "NA"
#endif

#define QUERY_URL "http://cdsweb.u-strasbg.fr/cgi-bin/nph-sesame/-oI?"
#define MAX_DATE_LEN 10

#define LINE_BOTTOM \
	fprintf(stdout, "└─────────────────────────────────────────────────────────────────────────────────────────────────────┘\n");

struct options {
	float o_lat;
	float o_lng;
	float o_step_jd;
	char o_date[MAX_DATE_LEN + 1];
};

struct options opt = {
	/* Bad Soden am Taunus */
	.o_lat = 50.146146,
	.o_lng = 8.498569,
	.o_step_jd = 0.01,
	.o_date = {0}
};

struct mem_t {
	char *memory;
	size_t size;
};

struct rise_set_t {
	struct ln_rst_time rst_sun;	/* Sun rise and set, -0.8333 degree below horizon. */
	struct ln_rst_time rst_aa_tw;	/* Sun rise and set, -15 degree below horizon (darkness for amateur astronomy). */
	struct ln_rst_time rst_a_tw;	/* Sun rise and set, -18 degree below horizon (darkness for professional astronomy). */
};

static void line_top(const char *title, const char *start, const char *end)
{
	uint8_t i;

	fprintf(stdout, "%s", start);
	for (i = 0; i < 45; i++)
		fprintf(stdout, "─");
	fprintf(stdout, " %s ", title);
	for (i = 0; i < 54 - strlen(title); i++)
		fprintf(stdout, "─");
	fprintf(stdout, "%s\n", end);
}

static void usage(const char *cmd_name, const int rc)
{
        fprintf(stdout, "usage: %s [option] <object 1> <object 2> .... <object N>\n"
                "\t-a, --latitude <float>\n"
                "\t       Latitude coordinate in decimal degree [default: %f]\n"
                "\t-o, --longitude <float>\n"
                "\t       Longitude coordinate in decimal degree [default: %f]\n"
                "\t-d, --date <string>\n"
                "\t       Date in format yyyy-mm-dd, e.g. '2017-06-30' [default: current date]\n"
		"\t-s, --stepjd <float>\n"
		"\t       Partition tabular data by step size in julian date [default: %.2f]\n"
                "version: %s © 2017 by Thomas Stibor <thomas@stibor.net>\n",
                cmd_name, opt.o_lat, opt.o_lng, opt.o_step_jd, PACKAGE_VERSION);
        exit(rc);
}

static int parseopts(int argc, char *argv[])
{
	struct option long_opts[] = {
		{"latitude",  required_argument, 0, 'a'},
		{"longitude", required_argument, 0, 'o'},
		{"date",      required_argument, 0, 'd'},
		{"step_jd",   required_argument, 0, 's'},
		{"help",      no_argument, 0,	    'h'},
		{0, 0, 0, 0}
	};

	int c;
	while ((c = getopt_long(argc, argv, "a:o:d:s:h",
				long_opts, NULL)) != -1) {
		switch (c) {
		case 'a': {
			opt.o_lat = atof(optarg);
			break;
		}
		case 'o': {
			opt.o_lng = atof(optarg);
			break;
		}
		case 'd': {
			strncpy(opt.o_date, optarg, MAX_DATE_LEN);
			break;
		}
		case 's': {
			opt.o_step_jd = atof(optarg);
			break;
		}
		case 'h': {
			usage(argv[0], 0);
			break;
		}
		case 0: {
			break;
		}
		default:
			return -EINVAL;
		}
	}

	return 0;
}

static size_t writemem_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct mem_t *mem = (struct mem_t *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		fprintf(stderr, "not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

static long validate_number(const char *str)
{
	char *endptr;
	long val;

	errno = 0;
	val = strtol(str, &endptr, 10);

	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
	    || (errno != 0 && val == 0))
		return -1;

	if (endptr == str)
		return -1;

	return val;
}

static int parse_date(const char *str, struct ln_date *date)
{
	const char *delim = "-";
	char *token = NULL;
	uint8_t w_token = 0;

	struct tm *ltime;
	struct timeval tv;
	struct timezone tz;

	/* Get current time with microseconds precission. */
	gettimeofday(&tv, &tz);

	/* Convert to localtime representation. */
	ltime = localtime(&tv.tv_sec);

	/* Fill in date struct. */
	date->seconds = ltime->tm_sec + ((double)tv.tv_usec / 1000000);
	date->minutes = ltime->tm_min;
	date->hours = ltime->tm_hour;
	date->days = ltime->tm_mday;
	date->months = ltime->tm_mon + 1;
	date->years = ltime->tm_year + 1900;

	if (str == NULL || strlen(str) == 0)
		return 0;

	token = strtok((char *)str, delim);
	if (!token)
		return -1;

	while (token != NULL) {

		long val = validate_number(token);
		if (w_token == 0)
			date->years = val == -1 ? date->years : val;
		else if (w_token == 1)
			date->months = val == -1 ? date->months : val;
		else if (w_token == 2)
			date->days = val == -1 ? date->days : val;
		else {
			fprintf(stderr, "unknown date token\n");
			return -1;
		}
		token = strtok(NULL, delim);
		w_token++;
	}

	return 0;
}

static int get_ra_dec(const char *str, double *ra, double *dec)
{
	const char *nl_delim = "\n";
	char *token = NULL;
	bool found[2] = {0};

	token = strtok((char *)str, nl_delim);
	if (!token)
		return -1;

	while (token != NULL) {
		const char *begin_str = NULL;

		begin_str = strstr(token, "#=N=NED");
		if (begin_str)
			found[0] = 1;
		else {
			begin_str = strstr(token, "Simbad");
			if (begin_str)
				found[0] = 1;
		}

		if (found[0]) {
			begin_str = strstr(token, "%J ");
			if (begin_str)
				found[1] = 1;
		}

		if (found[0] && found[1]) {
			uint8_t n = 0;
			char *line = token;
			do {
				if (isdigit(*line) || (*line == '-' && isdigit(*(line + 1)))) {
					if (n == 0)
						*ra = strtod(line, &line);
					else if (n == 1)
						*dec = strtod(line, &line);
					++n;
				} else
					++line;
			} while (*line != '\0' && n < 2);
			return 0;
		}

		token = strtok(NULL, nl_delim);
	}

	return -1;
}

static int query_catalog(const char *object, double *ra, double *dec)
{
	int rc;
	CURL *curl_handle;
	CURLcode res;
	struct mem_t reply_content;

	reply_content.memory = malloc(1);
	reply_content.size = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	char query[256] = {0};
	snprintf(query, 255, "%s%s", QUERY_URL, object);

	curl_easy_setopt(curl_handle, CURLOPT_URL, query);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writemem_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&reply_content);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	res = curl_easy_perform(curl_handle);
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform failed: %s\n",
			curl_easy_strerror(res));
		rc = -1;
		goto cleanup;
	}

	rc = get_ra_dec(reply_content.memory, ra, dec);

cleanup:
	curl_easy_cleanup(curl_handle);
	free(reply_content.memory);
	curl_global_cleanup();

	return rc;
}

static void print_date(const char *title, const struct ln_zonedate *date)
{
	strlen(title) > 20 ?
		fprintf(stdout, "%-85s%4d-%02d-%02d %02d:%02d:%02d│\n",
			title,
			date->years, date->months, date->days,
			date->hours, date->minutes, (int)date->seconds) :
		fprintf(stdout, "%-87s%4d-%02d-%02d %02d:%02d:%02d│\n",
			title,
			date->years, date->months, date->days,
			date->hours, date->minutes, (int)date->seconds);
}

static void display_lunar(const double julian_date, struct ln_lnlat_posn *observer)
{
	int rc;
	struct ln_zonedate rise, set, transit;
	struct ln_rst_time rst_lunar;

	line_top("moon", "├", "┤");

	rc = ln_get_lunar_rst(julian_date, observer, &rst_lunar);
	if (rc) {
		fprintf(stdout, "moon is circumpolar, that is it remains the whole "
			"day either above or below the horizon\n");
	} else {
		ln_get_local_date(rst_lunar.rise, &rise);
		ln_get_local_date(rst_lunar.transit, &transit);
		ln_get_local_date(rst_lunar.set, &set);

		if (rst_lunar.rise < rst_lunar.transit &&
		    rst_lunar.rise < rst_lunar.set) {
			print_date("│↑ rise", &rise);
			if (rst_lunar.transit < rst_lunar.set) {
				print_date("│↔ transit", &transit);
				print_date("│↓ set", &set);
			} else {
				print_date("│↓ set", &set);
				print_date("│↔ transit", &transit);
			}
		} else if (rst_lunar.set < rst_lunar.rise &&
			   rst_lunar.set < rst_lunar.transit) {
			print_date("│↓ set", &set);
			if (rst_lunar.rise < rst_lunar.transit) {
				print_date("│↑ rise", &rise);
				print_date("│↔ transit", &transit);
			} else {
				print_date("│↔ transit", &transit);
				print_date("│↑ rise", &rise);
			}
		} else {
			print_date("│↔ transit", &transit);
			if (rst_lunar.set < rst_lunar.rise) {
				print_date("│↓ set", &set);
				print_date("│↑ rise", &rise);
			} else {
				print_date("│↑ rise", &rise);
				print_date("│↓ set", &set);
			}
		}
	}

	fprintf(stdout, "│%-93s%f│\n",
		"disk illuminated fraction ", ln_get_lunar_disk(julian_date));
#if 0
	fprintf(stdout, "phase           : %f\n", ln_get_lunar_phase(julian_date));
	fprintf(stdout, "bright limb     : %f\n", ln_get_lunar_bright_limb(julian_date));
#endif
	struct ln_rect_posn moon_pos;
	ln_get_lunar_geo_posn(julian_date, &moon_pos, 0);
	fprintf(stdout,"│%-87s %.3f km│\n",
		"distance to earth",
		sqrt(moon_pos.X * moon_pos.X
		     + moon_pos.Y * moon_pos.Y
		     + moon_pos.Z * moon_pos.Z));
	LINE_BOTTOM;
}

static void display_sun(const double julian_date, struct ln_lnlat_posn *observer, struct rise_set_t *rise_set)
{
	struct ln_rst_time rst_sun, rst_aa_tw, rst_a_tw;
	struct ln_zonedate rise_sun, set_sun;
	struct ln_zonedate rise_aa, set_aa, rise_a, set_a;

	line_top("sun", "├", "┤");
	ln_get_body_next_rst_horizon(julian_date, observer,
				     ln_get_solar_equ_coords, LN_SOLAR_STANDART_HORIZON, &rst_sun);

	/* Amateur astronomical twilight (the sky is dark enough for most astronomical observations) */
	ln_get_body_next_rst_horizon(julian_date, observer,
				     ln_get_solar_equ_coords, -15, &rst_aa_tw);
	/* Astronomical twilight (the sky is completely dark) */
	ln_get_body_next_rst_horizon(julian_date, observer,
				     ln_get_solar_equ_coords, -18, &rst_a_tw);

	/* Sun */
	ln_get_local_date(rst_sun.rise, &rise_sun);
	ln_get_local_date(rst_sun.set, &set_sun);

	/* Twilight amateur astronomical. */
	ln_get_local_date(rst_aa_tw.rise, &rise_aa);
	ln_get_local_date(rst_aa_tw.set, &set_aa);

	/* Twilight astronomical. */
	ln_get_local_date(rst_a_tw.rise, &rise_a);
	ln_get_local_date(rst_a_tw.set, &set_a);

	if (rst_sun.set < rst_sun.rise) {
		print_date("│↓ set", &set_sun);
		print_date("│↑ rise", &rise_sun);
	} else {
		print_date("│↑ rise", &rise_sun);
		print_date("│↓ set", &set_sun);
	}

	line_top("twilight", "├", "┤");
	print_date("│amateur astronomy begin", &set_aa);
	print_date("│professional astronomy begin", &set_a);
	print_date("│professional astronomy end", &rise_a);
	print_date("│amateur astronomy end", &rise_aa);

	memcpy(&rise_set->rst_sun, &rst_sun, sizeof(rst_sun));
	memcpy(&rise_set->rst_aa_tw, &rst_aa_tw, sizeof(rst_aa_tw));
	memcpy(&rise_set->rst_a_tw, &rst_a_tw, sizeof(rst_a_tw));
}

static void display_object(struct ln_equ_posn *equ_object,
			   struct ln_lnlat_posn *observer,
			   double julian_date, const char *object,
			   struct rise_set_t *rise_set)
{
	int rc;
	struct ln_zonedate rise, set, transit;
	struct ln_rst_time rst_object;
	bool not_visible = false;
	struct lnh_equ_posn hpos;
	char title[64] = {0};

	rc = ln_get_object_next_rst(julian_date, observer, equ_object, &rst_object);
	ln_equ_to_hequ(equ_object, &hpos);

	snprintf(title, 64, "object '%s'", object);
	line_top(title, "┌", "┐");
	fprintf(stdout, "│%-41s(%010.6f, %010.6f) (%02d:%02d:%06.3f, %c%02d:%02d:%06.3f) J2000│\n",
		"ra, dec",
		equ_object->ra, equ_object->dec,
		hpos.ra.hours, hpos.ra.minutes, hpos.ra.seconds,
		hpos.dec.neg ? '-' : '+',
		hpos.dec.degrees, hpos.dec.minutes, hpos.dec.seconds);
	switch (rc) {
		case -1 : {
			fprintf(stdout, "│object remains the whole day bellow the horizon%-54s│\n", " ");
			not_visible = true;
			break;
		}
		case 1: {
			fprintf(stdout, "│object is circumpolar, that is it remains the whole "
				"%-49s│\n",
				"day above the horizon");
			break;
		}
		default: {
			ln_get_local_date(rst_object.rise, &rise);
			ln_get_local_date(rst_object.transit, &transit);
			ln_get_local_date(rst_object.set, &set);

			if (rst_object.rise < rst_object.transit &&
			    rst_object.rise < rst_object.set) {
				print_date("│↑ rise", &rise);
				if (rst_object.transit < rst_object.set) {
					print_date("│↔ transit", &transit);
					print_date("│↓ set", &set);
				} else {
					print_date("│↓ set", &set);
					print_date("│↔ transit", &transit);
				}
			} else if (rst_object.set < rst_object.rise &&
				   rst_object.set < rst_object.transit) {
				print_date("│↓ set", &set);
				if (rst_object.rise < rst_object.transit) {
					print_date("│↑ rise", &rise);
					print_date("│↔ transit", &transit);
				} else {
					print_date("│↔ transit", &transit);
					print_date("│↑ rise", &rise);
				}
			} else {
				print_date("│↔ transit", &transit);
				if (rst_object.set < rst_object.rise) {
					print_date("│↓ set", &set);
					print_date("│↑ rise", &rise);
				} else {
					print_date("│↑ rise", &rise);
					print_date("│↓ set", &set);
				}
			}
			break;
		}
	}

	if (not_visible) {
		LINE_BOTTOM;
		return;
	}

	if (opt.o_step_jd <= 0) {
		LINE_BOTTOM;
		return;
	}

	struct ln_hrz_posn hrz_posn;
	struct ln_zonedate date;
	struct ln_equ_posn equ_lunar;
	double jd = rise_set->rst_sun.set;

	fprintf(stdout,
		"│%104s\n"
		"├ airmass ──┬── azimuth ──┬── altitude ──┬────  date ────┬──── time ────┬── moon separation (degrees) ┤\n",
		"│");
	while (jd < rise_set->rst_sun.rise) {
		ln_get_local_date(jd, &date);
		ln_get_hrz_from_equ(equ_object, observer, jd, &hrz_posn);
		ln_get_lunar_equ_coords(jd, &equ_lunar);
		if (hrz_posn.alt > 0)
		fprintf(stdout,
			"│%5.2f"
			"%9s"
			"%8.2f"
			"%8s"
			"%8.2f"
			"%9s"
			"%6d-%02d-%02d%6s%3s%02d:%02d:%02d"
			"%6s%3s"
			"%8.4f%18s│\n",
			ln_get_airmass(hrz_posn.alt, 750.0),
			"│",
			hrz_posn.az,
			"│",
			hrz_posn.alt,
			"│",
			date.years, date.months, date.days,
			"│", " ",
			date.hours, date.minutes, (int)date.seconds,
			"│", " ",
			ln_get_angular_separation(equ_object, &equ_lunar),
			" ");
		jd += opt.o_step_jd;
	};
	fprintf(stdout,
		"└───────────┴─────────────┴──────────────┴───────────────┴──────────────┴─────────────────────────────┘\n");
}

int parse_ra_dec(const char *deg_ra_dec, double *ra, double *dec)
{
	if (!deg_ra_dec)
		return -EINVAL;

	const char *delim = " ";
	char *token;
	uint8_t n_token = 0;
	char *end = NULL;
	char *str;
	int rc = -EINVAL;

	str = strdup(deg_ra_dec);
	token = strtok((char *)str, delim);
	if (!token)
		goto out;

	while (token != NULL) {

		if (n_token == 0) {
			*ra = strtod(token, &end);
			if (*end != '\0')
				goto out;
		} else if (n_token == 1) {
			*dec = strtod(token, &end);
			if (*end != '\0')
				goto out;
		} else
			goto out;

		token = strtok(NULL, delim);
		n_token++;
	}

out:
	if (n_token == 2 &&
	    *ra >= 0 && *ra <= 360 &&
	    *dec >= -90.0f && *dec <= 90.0f)
		rc = 0;

	if (str)
		free(str);

	return rc;
}

int main(int argc, char *argv[])
{
	int rc;
	rc = parseopts(argc, argv);
	if (rc) {
		fprintf(stderr, "try '%s --help' for more information", argv[0]);
		return -EINVAL;
	}

	struct ln_lnlat_posn observer = {
		.lat = opt.o_lat,
		.lng = opt.o_lng
	};

	struct ln_date date;
	rc = parse_date(opt.o_date, &date);
	if (rc < 0)
		fprintf(stderr, "parse_date failed, use local system date/time\n");

	line_top("observer", "┌", "┐");
	fprintf(stdout,
		"│%-80s%10.6f,%10.6f│\n", "lat,lng", observer.lat, observer.lng);

	double julian_date = ln_get_julian_day(&date);
	line_top("date", "├", "┤");
	fprintf(stdout,
		"│%-87s%f│\n", "julian", julian_date);
	fprintf(stdout,
		"│%-82s%d-%02d-%02d %02d:%02d:%02d│\n",
		"calendar (LT)", date.years, date.months, date.days,
		date.hours, date.minutes, (int)date.seconds);

	/* Set time of current date to morning 08:00:00, to correctly calculate set and rise of the current and next day.*/
	date.hours = 8; date.minutes = 0; date.seconds = 0;
	julian_date = ln_get_julian_day(&date);

	struct rise_set_t rise_set;
	display_sun(julian_date, &observer, &rise_set);
	display_lunar(julian_date, &observer);

	struct ln_equ_posn equ_object; /* right ascension, declination  of <object 1> <object 2> ... <object N> */
	while (optind < argc) {

		/* Try parsing objects in format "degree_ra degree_dec". */
		rc = parse_ra_dec(argv[optind], &equ_object.ra, &equ_object.dec);
		if (!rc)
			display_object(&equ_object, &observer, julian_date, argv[optind], &rise_set);
		else {
			rc = query_catalog(argv[optind], &equ_object.ra, &equ_object.dec);
			if (rc)
				fprintf(stderr, "no results found for object '%s'\n", argv[optind]);
			else
				display_object(&equ_object, &observer, julian_date, argv[optind], &rise_set);
		}
		optind++;
	}

	return 0;
}
