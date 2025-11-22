#include "prayertimes.h"
#include <math.h>
#include <stdio.h>

// Helper: normalize angle to [0,360)
static double normalize_deg(double angle) {
  double a = fmod(angle, 360.0);
  if (a < 0)
    a += 360.0;
  return a;
}

// Calculate Julian Day from a calendar date (simplified)
static double julian_day(int year, int month, int day) {
  if (month <= 2) {
    year -= 1;
    month += 12;
  }
  int A = year / 100;
  int B = 2 - A + (A / 4);
  double jd = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) +
              day + B - 1524.5;
  return jd;
}

// Calculate solar declination and equation of time
static void sun_position(double jd, double *decl, double *eqt) {
  double D = jd - JULIAN_EPOCH;

  double g = normalize_deg(SUN_MEAN_ANOMALY_OFFSET + SUN_MEAN_ANOMALY_RATE * D);
  double q =
      normalize_deg(SUN_MEAN_LONGITUDE_OFFSET + SUN_MEAN_LONGITUDE_RATE * D);

  double L =
      normalize_deg(q + SUN_ECCENTRICITY_AMPLITUDE1 * sin(g * DEG_TO_RAD) +
                    SUN_ECCENTRICITY_AMPLITUDE2 * sin(2 * g * DEG_TO_RAD));

  double e = OBLIQUITY_COEFF - OBLIQUITY_RATE * D;

  double RA =
      atan2(cos(e * DEG_TO_RAD) * sin(L * DEG_TO_RAD), cos(L * DEG_TO_RAD)) *
      RAD_TO_DEG;
  RA = normalize_deg(RA);

  *eqt = (q / 15.0) - (RA / 15.0);
  *decl = asin(sin(e * DEG_TO_RAD) * sin(L * DEG_TO_RAD)) * RAD_TO_DEG;
}

// Compute the time difference from solar noon for a given sun altitude angle
// For angles below horizon (Fajr, Sunrise, Maghrib, Isha): pass positive angle
// For angles above horizon (Asr): pass negative angle to indicate positive altitude
static double hour_angle(double lat, double decl, double angle) {
  double lat_rad = lat * DEG_TO_RAD;
  double decl_rad = decl * DEG_TO_RAD;
  double angle_rad = angle * DEG_TO_RAD;

  // Formula: cos(H) = [sin(h) - sin(φ) × sin(δ)] / [cos(φ) × cos(δ)]
  // For below-horizon events: h is negative, so we use -sin(angle) where angle is positive
  // For above-horizon events: h is positive, so we use sin(angle) directly
  double numerator = -sin(angle_rad) - sin(lat_rad) * sin(decl_rad);
  double denominator = cos(lat_rad) * cos(decl_rad);
  double ha = acos(numerator / denominator);
  return ha * RAD_TO_DEG / 15.0; // convert from degrees to hours
}

// Format time (double hours) into "HH:MM"
void format_time_hm(double timeHours, char *outBuffer, size_t bufSize) {
  int hours = (int)timeHours;
  double fraction = timeHours - hours;
  int minutes = (int)ceil(fraction * 60.0); // Always round up (Kemenag method)

  if (minutes >= 60) {
    hours += 1;
    minutes -= 60;
  }

  hours %= 24;

  snprintf(outBuffer, bufSize, "%02d:%02d", hours, minutes);
}

// Format time into "HH:MM:SS"
void format_time_hms(double timeHours, char *outBuffer, size_t bufSize) {
  int hours = (int)timeHours;
  double fraction = timeHours - hours;
  int totalSeconds = (int)(fraction * 3600.0 + 0.5);

  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;

  if (minutes >= 60) {
    hours += minutes / 60;
    minutes %= 60;
  }

  hours %= 24;

  snprintf(outBuffer, bufSize, "%02d:%02d:%02d", hours, minutes, seconds);
}

struct PrayerTimes calculate_prayer_times(int year, int month, int day,
                                          double latitude, double longitude,
                                          double timezone) {
  double jd = julian_day(year, month, day);
  double decl, eqt;
  sun_position(jd, &decl, &eqt);

  double noon = 12.0 + timezone - (longitude / 15.0) - eqt;

  double ha_sunrise = hour_angle(latitude, decl, REFRACTION_CORRECTION);
  double sunrise = noon - ha_sunrise;
  double maghrib = noon + ha_sunrise;

  double ha_fajr = hour_angle(latitude, decl, FAJR_ANGLE_KEMENAG);
  double fajr = noon - ha_fajr;

  double ha_isha = hour_angle(latitude, decl, ISHA_ANGLE_KEMENAG);
  double isha = noon + ha_isha;

  double asr_angle = atan(1.0 / (SHADOW_FACTOR_STANDARD +
                                 tan(fabs(latitude - decl) * DEG_TO_RAD))) *
                     RAD_TO_DEG;
  // For Asr, angle is above horizon, so we pass negative to indicate positive altitude
  double ha_asr = hour_angle(latitude, decl, -asr_angle);
  double asr = noon + ha_asr;

  // Calculate Dhuha time (Kemenag standard: ~28-30 minutes after sunrise)
  // This represents when the sun is sufficiently high (about one spear length)
  double dhuha = sunrise + (DHUHA_START_OFFSET / 60.0);

  // Apply ihtiyat (precautionary) adjustments according to Kemenag standard
  fajr += IHTIYAT_FAJR / 60.0;       // +2 minutes
  sunrise += IHTIYAT_SUNRISE / 60.0; // -2 minutes
  noon += IHTIYAT_DHUHR / 60.0;      // +2 minutes
  asr += IHTIYAT_ASR / 60.0;         // +2 minutes
  maghrib += IHTIYAT_MAGHRIB / 60.0; // +2 minutes
  isha += IHTIYAT_ISHA / 60.0;       // +2 minutes

  // Note: Dhuha is calculated AFTER sunrise ihtiyat adjustment
  // So it's based on the adjusted sunrise time + 28 minutes
  // This needs to be recalculated after the ihtiyat is applied
  dhuha = sunrise + (DHUHA_START_OFFSET / 60.0);

  struct PrayerTimes times = {
      .fajr = fajr,
      .sunrise = sunrise,
      .dhuha = dhuha,
      .dhuhr = noon,
      .asr = asr,
      .maghrib = maghrib,
      .isha = isha,
  };

  return times;
}
