/*
 * timezone.h — optional DST-aware timezone helper for libmuslim
 *
 * `prayertimes.h` is pure astronomy: it takes a numeric UTC offset and does
 * math. It does NOT know about DST, because DST is a political rule, not an
 * astronomical one. This header is the optional companion that resolves the
 * correct offset for a given IANA zone and date, with DST and historical zone
 * changes honored by the host operating system's timezone database.
 *
 * Unlike `prayertimes.h` (which depends only on <math.h> and is fully
 * portable), this header touches the OS:
 *   - POSIX  : setenv(TZ) -> tzset() -> localtime_r() -> tm_gmtoff
 *   - Windows: EnumDynamicTimeZoneInformation + SystemTimeToTzSpecificLocalTimeEx
 * Use it only if you want libmuslim to compute the offset for you; otherwise
 * keep supplying the offset to `calculate_prayer_times` yourself.
 *
 * Single-header usage — in exactly ONE translation unit:
 *     #define MUSLIM_TIMEZONE_IMPLEMENTATION
 *     #include "timezone.h"
 * Everywhere else just #include "timezone.h".
 *
 * On glibc, tm_gmtoff requires _GNU_SOURCE / _DEFAULT_SOURCE. The
 * implementation block defines _GNU_SOURCE if it is not already set, so make
 * sure nothing includes <time.h> ahead of the implementation include in that
 * one TU.
 */

#ifndef MUSLIM_TIMEZONE_H
#define MUSLIM_TIMEZONE_H

/* The POSIX implementation reads `struct tm`'s tm_gmtoff field, a BSD/GNU
 * extension that glibc only exposes when a feature-test macro is set BEFORE
 * <time.h> is first included. Define one here so the offset is correct even
 * under -std=c11. This requires timezone.h to be included before any system
 * <time.h> in the translation unit. */
#if !defined(_WIN32) && !defined(_GNU_SOURCE) && !defined(_DEFAULT_SOURCE) && \
    !defined(_BSD_SOURCE)
#define _DEFAULT_SOURCE 1
#endif

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return the UTC offset, in hours, for the IANA zone `tz_name` at instant
 * `when` (Unix epoch seconds, UTC). DST is already applied: e.g. for
 * "Europe/London" this returns 0.0 in winter and 1.0 during British Summer
 * Time. Pass the result straight into `calculate_prayer_times`.
 *
 * Returns 0.0 if `tz_name` is NULL or cannot be resolved by the host (on
 * Windows, if the zone is outside the bundled IANA<->Windows table).
 */
double parse_timezone_offset(const char *tz_name, time_t when);

/*
 * Write the host system's IANA timezone name (e.g. "Asia/Jakarta") into `buf`.
 * Returns 0 on success, -1 on failure (in which case `buf` is set to "UTC"
 * when there is room). `cap` is the size of `buf` in bytes.
 */
int get_system_timezone(char *buf, size_t cap);

#ifdef __cplusplus
}
#endif

/* ===================================================================== */
/* Implementation                                                        */
/* ===================================================================== */

#ifdef MUSLIM_TIMEZONE_IMPLEMENTATION

#if defined(_WIN32)

/* ---- Windows implementation -------------------------------------------- *
 * Win32 timezone APIs use Windows zone names ("Egypt Standard Time"), not
 * IANA names ("Africa/Cairo"). We translate via a CLDR-derived table. Zones
 * outside this table resolve to 0.0 / "UTC"; extend IANA_TO_WIN as needed.   */

#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0602
#undef _WIN32_WINNT
#define _WIN32_WINNT \
  0x0602 // Windows 8: EnumDynamicTimeZoneInformation, SystemTimeToTzSpecificLocalTimeEx
#endif

#include <string.h>
#include <wchar.h>
#include <windows.h>

typedef struct {
  const char *iana;
  const wchar_t *win;
} MuslimIanaWinPair;

/* Roughly alphabetical by IANA name. Where multiple IANA names share a
 * Windows zone, the canonical one is listed FIRST so the reverse mapping
 * returns the expected name (e.g. "Asia/Tokyo" rather than "Asia/Jayapura"). */
static const MuslimIanaWinPair MUSLIM_IANA_TO_WIN[] = {
    {"Africa/Cairo", L"Egypt Standard Time"},
    {"Africa/Johannesburg", L"South Africa Standard Time"},
    {"Africa/Nairobi", L"E. Africa Standard Time"},
    {"America/Anchorage", L"Alaskan Standard Time"},
    {"America/Argentina/Buenos_Aires", L"Argentina Standard Time"},
    {"America/Bogota", L"SA Pacific Standard Time"},
    {"America/Chicago", L"Central Standard Time"},
    {"America/Denver", L"Mountain Standard Time"},
    {"America/Los_Angeles", L"Pacific Standard Time"},
    {"America/New_York", L"Eastern Standard Time"},
    {"America/Santiago", L"Pacific SA Standard Time"},
    {"America/Sao_Paulo", L"E. South America Standard Time"},
    {"America/St_Johns", L"Newfoundland Standard Time"},
    {"Asia/Jakarta", L"SE Asia Standard Time"},
    {"Asia/Bangkok", L"SE Asia Standard Time"},
    {"Asia/Colombo", L"Sri Lanka Standard Time"},
    {"Asia/Dhaka", L"Bangladesh Standard Time"},
    {"Asia/Dubai", L"Arabian Standard Time"},
    {"Asia/Tokyo", L"Tokyo Standard Time"},
    {"Asia/Jayapura", L"Tokyo Standard Time"},
    {"Asia/Kabul", L"Afghanistan Standard Time"},
    {"Asia/Karachi", L"Pakistan Standard Time"},
    {"Asia/Kathmandu", L"Nepal Standard Time"},
    {"Asia/Kolkata", L"India Standard Time"},
    {"Asia/Singapore", L"Singapore Standard Time"},
    {"Asia/Kuala_Lumpur", L"Singapore Standard Time"},
    {"Asia/Makassar", L"Singapore Standard Time"},
    {"Asia/Riyadh", L"Arab Standard Time"},
    {"Asia/Yangon", L"Myanmar Standard Time"},
    {"Australia/Adelaide", L"Cen. Australia Standard Time"},
    {"Australia/Sydney", L"AUS Eastern Standard Time"},
    {"Etc/UTC", L"UTC"},
    {"UTC", L"UTC"},
    {"Europe/Berlin", L"W. Europe Standard Time"},
    {"Europe/Istanbul", L"Turkey Standard Time"},
    {"Europe/London", L"GMT Standard Time"},
    {"Europe/Paris", L"Romance Standard Time"},
    {"Pacific/Apia", L"Samoa Standard Time"},
    {"Pacific/Auckland", L"New Zealand Standard Time"},
    {"Pacific/Honolulu", L"Hawaiian Standard Time"},
    {"Pacific/Kiritimati", L"Line Islands Standard Time"},
};

static const wchar_t *muslim_iana_to_windows_zone(const char *tz_name) {
  if (!tz_name)
    return NULL;
  for (size_t i = 0; i < sizeof(MUSLIM_IANA_TO_WIN) / sizeof(MUSLIM_IANA_TO_WIN[0]); ++i) {
    if (strcmp(MUSLIM_IANA_TO_WIN[i].iana, tz_name) == 0)
      return MUSLIM_IANA_TO_WIN[i].win;
  }
  return NULL;
}

static const char *muslim_windows_zone_to_iana(const wchar_t *win_zone) {
  if (!win_zone)
    return NULL;
  for (size_t i = 0; i < sizeof(MUSLIM_IANA_TO_WIN) / sizeof(MUSLIM_IANA_TO_WIN[0]); ++i) {
    if (wcscmp(MUSLIM_IANA_TO_WIN[i].win, win_zone) == 0)
      return MUSLIM_IANA_TO_WIN[i].iana;
  }
  return NULL;
}

double parse_timezone_offset(const char *tz_name, time_t when) {
  const wchar_t *win_zone = muslim_iana_to_windows_zone(tz_name);
  if (!win_zone)
    return 0.0;

  // Find the DYNAMIC_TIME_ZONE_INFORMATION whose key matches.
  DYNAMIC_TIME_ZONE_INFORMATION dtzi;
  DWORD idx = 0;
  int found = 0;
  while (EnumDynamicTimeZoneInformation(idx++, &dtzi) == ERROR_SUCCESS) {
    if (wcscmp(dtzi.TimeZoneKeyName, win_zone) == 0) {
      found = 1;
      break;
    }
  }
  if (!found)
    return 0.0;

  // time_t (Unix epoch seconds, UTC) -> FILETIME (100ns ticks since 1601-01-01).
  // 11644473600 seconds separate 1601-01-01 from 1970-01-01.
  ULONGLONG ticks = ((ULONGLONG)when + 11644473600ULL) * 10000000ULL;
  FILETIME utc_ft;
  utc_ft.dwLowDateTime = (DWORD)(ticks & 0xFFFFFFFFULL);
  utc_ft.dwHighDateTime = (DWORD)(ticks >> 32);

  SYSTEMTIME utc_st;
  if (!FileTimeToSystemTime(&utc_ft, &utc_st))
    return 0.0;

  SYSTEMTIME local_st;
  if (!SystemTimeToTzSpecificLocalTimeEx(&dtzi, &utc_st, &local_st))
    return 0.0;

  // Treat local_st as if it were UTC to recover a tick count; the delta
  // against utc_ft is exactly the offset (DST already baked in by the API).
  FILETIME local_ft;
  if (!SystemTimeToFileTime(&local_st, &local_ft))
    return 0.0;

  ULONGLONG utc_ticks = ((ULONGLONG)utc_ft.dwHighDateTime << 32) | utc_ft.dwLowDateTime;
  ULONGLONG local_ticks = ((ULONGLONG)local_ft.dwHighDateTime << 32) | local_ft.dwLowDateTime;
  LONGLONG diff = (LONGLONG)local_ticks - (LONGLONG)utc_ticks;

  // 10^7 ticks/sec * 3600 sec/hr = 3.6 * 10^10 ticks/hr.
  return (double)diff / 36000000000.0;
}

int get_system_timezone(char *buf, size_t cap) {
  if (!buf || cap < 2)
    return -1;

  DYNAMIC_TIME_ZONE_INFORMATION dtzi;
  DWORD rc = GetDynamicTimeZoneInformation(&dtzi);
  if (rc == TIME_ZONE_ID_INVALID) {
    if (cap >= 4)
      memcpy(buf, "UTC", 4);
    else
      buf[0] = '\0';
    return -1;
  }

  const char *iana = muslim_windows_zone_to_iana(dtzi.TimeZoneKeyName);
  if (!iana) {
    if (cap >= 4)
      memcpy(buf, "UTC", 4);
    else
      buf[0] = '\0';
    return -1;
  }

  size_t n = strlen(iana);
  if (n + 1 > cap) {
    buf[0] = '\0';
    return -1;
  }
  memcpy(buf, iana, n + 1);
  return 0;
}

#else /* !_WIN32 */

/* ---- POSIX implementation ---------------------------------------------- *
 * Uses the system tzdb (typically /usr/share/zoneinfo) via libc:
 *   setenv(TZ) -> tzset() -> localtime_r() -> tm_gmtoff.
 * DST and historical zone changes are honored automatically.
 * (The tm_gmtoff feature-test macro is set at the top of this header, before
 * <time.h>, so it is already in effect here.)                                */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

double parse_timezone_offset(const char *tz_name, time_t when) {
  if (!tz_name)
    return 0.0;

  // Save the current TZ so we never leak our setenv to other callers.
  const char *old_tz = getenv("TZ");
  char *saved = old_tz ? strdup(old_tz) : NULL;

  setenv("TZ", tz_name, 1);
  tzset();

  struct tm lt;
  localtime_r(&when, &lt);
  double offset = (double)lt.tm_gmtoff / 3600.0;

  if (saved) {
    setenv("TZ", saved, 1);
    free(saved);
  } else {
    unsetenv("TZ");
  }
  tzset();

  return offset;
}

static int muslim_copy_zone_tail(const char *path, char *buf, size_t cap) {
  // Find the substring "/zoneinfo/" and take everything after it.
  const char *needle = "/zoneinfo/";
  const char *p = strstr(path, needle);
  if (!p)
    return -1;
  p += strlen(needle);
  size_t n = strlen(p);
  if (n == 0 || n + 1 > cap)
    return -1;
  memcpy(buf, p, n + 1);
  return 0;
}

int get_system_timezone(char *buf, size_t cap) {
  if (!buf || cap < 2)
    return -1;

  // Primary: readlink("/etc/localtime") -> /usr/share/zoneinfo/<Area>/<Zone>.
  char link[512];
  ssize_t n = readlink("/etc/localtime", link, sizeof(link) - 1);
  if (n > 0) {
    link[n] = '\0';
    if (muslim_copy_zone_tail(link, buf, cap) == 0)
      return 0;
  }

  // Fallback: /etc/timezone (Debian/Ubuntu) contains "Area/Zone\n".
  FILE *f = fopen("/etc/timezone", "r");
  if (f) {
    char line[128];
    char *got = fgets(line, sizeof(line), f);
    fclose(f);
    if (got) {
      size_t len = strlen(line);
      while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';
      if (len > 0 && len + 1 <= cap) {
        memcpy(buf, line, len + 1);
        return 0;
      }
    }
  }

  // Last resort: "UTC".
  if (cap >= 4) {
    memcpy(buf, "UTC", 4);
  } else {
    buf[0] = '\0';
  }
  return -1;
}

#endif /* _WIN32 */

#endif /* MUSLIM_TIMEZONE_IMPLEMENTATION */

#endif /* MUSLIM_TIMEZONE_H */
