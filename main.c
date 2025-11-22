#include "prayertimes.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <option>\n", argv[0]);
    printf("Options:\n");
    printf("  -v, --version\n");
    return 1;
  }

  if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
    printf("libmuslim v0.0.1\n");
    return 0;
  }

  if (argc < 6) {
    printf("Usage: %s <year> <month> <day> <latitude> <longitude> <timezone>\n",
           argv[0]);
    printf("Example: %s 2025 11 21 -6.2851291 106.9814968 +7.0\n", argv[0]);
    return 1;
  }

  int year, month, day;
  double latitude, longitude, timezone;

  if (sscanf(argv[1], "%d", &year) != 1 || year < 1900 || year > 2100) {
    printf("Invalid year: %s\n", argv[1]);
    return 1;
  }
  if (sscanf(argv[2], "%d", &month) != 1 || month < 1 || month > 12) {
    printf("Invalid month: %s\n", argv[2]);
    return 1;
  }
  if (sscanf(argv[3], "%d", &day) != 1 || day < 1 || day > 31) {
    printf("Invalid day: %s\n", argv[3]);
    return 1;
  }
  if (sscanf(argv[4], "%lf", &latitude) != 1 || latitude < -90.0 ||
      latitude > 90.0) {
    printf("Invalid latitude: %s\n", argv[4]);
    return 1;
  }
  if (sscanf(argv[5], "%lf", &longitude) != 1 || longitude < -180.0 ||
      longitude > 180.0) {
    printf("Invalid longitude: %s\n", argv[5]);
    return 1;
  }
  if (sscanf(argv[6], "%lf", &timezone) != 1 || timezone < -12.0 ||
      timezone > 14.0) {
    printf("Invalid timezone: %s\n", argv[6]);
    return 1;
  }

  // Calculate prayer times
  year = atoi(argv[1]);
  month = atoi(argv[2]);
  day = atoi(argv[3]);
  latitude = atof(argv[4]);
  longitude = atof(argv[5]);
  timezone = atof(argv[6]);

  struct PrayerTimes pt =
      calculate_prayer_times(year, month, day, latitude, longitude, timezone);

  char buf[16];

  format_time_hm(pt.fajr, buf, sizeof(buf));
  printf("Fajr    = %s\n", buf);
  fflush(NULL);

  format_time_hm(pt.sunrise, buf, sizeof(buf));
  printf("Sunrise = %s\n", buf);
  fflush(NULL);

  format_time_hm(pt.dhuha, buf, sizeof(buf));
  printf("Dhuha   = %s\n", buf);
  fflush(NULL);

  format_time_hm(pt.dhuhr, buf, sizeof(buf));
  printf("Dhuhr   = %s\n", buf);
  fflush(NULL);

  format_time_hm(pt.asr, buf, sizeof(buf));
  printf("Asr     = %s\n", buf);
  fflush(NULL);

  format_time_hm(pt.maghrib, buf, sizeof(buf));
  printf("Maghrib = %s\n", buf);
  fflush(NULL);

  format_time_hm(pt.isha, buf, sizeof(buf));
  printf("Isha    = %s\n", buf);

  return 0;
}
