#ifndef PRAYERTIMES_H
#define PRAYERTIMES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <string.h>

#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)

#define JULIAN_EPOCH 2451545.0

#define SUN_MEAN_ANOMALY_OFFSET 357.529
#define SUN_MEAN_ANOMALY_RATE 0.98560028

#define SUN_MEAN_LONGITUDE_OFFSET 280.459
#define SUN_MEAN_LONGITUDE_RATE 0.98564736

#define SUN_ECCENTRICITY_AMPLITUDE1 1.915
#define SUN_ECCENTRICITY_AMPLITUDE2 0.020

#define OBLIQUITY_COEFF 23.439
#define OBLIQUITY_RATE 0.00000036

#define REFRACTION_CORRECTION 0.833 // for dhuha/maghrib (deg)

#define FAJR_ANGLE_KEMENAG 20.0 // Kemenag Fajr = −20°
#define ISHA_ANGLE_KEMENAG 18.0 // Kemenag Isha = −18°

#define SHADOW_FACTOR_STANDARD 1.0 // for Asr (standard madhab)

// Dhuha prayer time parameters (Kemenag standard)
// Based on official Kemenag data: Dhuha starts ~28-30 minutes after sunrise
#define DHUHA_START_OFFSET 28.0  // Start 28 minutes after sunrise (in minutes)
#define DHUHA_END_OFFSET 5.0     // End 5 minutes before Dhuhr (in minutes)

// Ihtiyat (precautionary) adjustments in minutes (Kemenag standard)
#define IHTIYAT_FAJR 2.0
#define IHTIYAT_SUNRISE -2.0
#define IHTIYAT_DHUHR 2.0
#define IHTIYAT_ASR 2.0
#define IHTIYAT_MAGHRIB 2.0
#define IHTIYAT_ISHA 2.0

struct PrayerTimes {
  double fajr;
  double sunrise;
  double dhuha;     // Dhuha prayer time (Kemenag: ~28-30 min after sunrise)
  double dhuhr;
  double asr;
  double maghrib;
  double isha;
};

void format_time_hm(double timeHours, char *outBuffer, size_t bufSize);

void format_time_hms(double timeHours, char *outBuffer, size_t bufSize);

struct PrayerTimes calculate_prayer_times(int year, int month, int day,
                                          double latitude, double longitude,
                                          double timezone);

#ifdef __cplusplus
}
#endif

#endif
