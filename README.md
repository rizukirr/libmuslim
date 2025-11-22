# libmuslim

A lightweight C library for calculating Islamic prayer times using the Indonesian Ministry of Religious Affairs (Kemenag RI) calculation method.

## Features

- Accurate prayer time calculations based on Kemenag RI standards
- Astronomical calculations using Jean Meeus algorithms
- Cross-platform support (Linux, macOS, Windows)
- Android-ready shared library builds
- CLI tool for quick calculations
- Accuracy within ±1 minute compared to official Kemenag schedules

## How It Works

### Kemenag Calculation Method

The library implements the official Indonesian Ministry of Religious Affairs (Kemenag RI) method for calculating prayer times. This method uses specific astronomical parameters that differ from other Islamic calculation methods:

**Solar Altitude Angles:**
- **Fajr (Subuh):** -20° (sun is 20° below the eastern horizon)
- **Sunrise (Terbit):** -0.833° (accounting for atmospheric refraction)
- **Dhuhr (Dzuhur):** Solar noon (when the sun crosses the meridian)
- **Asr (Ashar):** When shadow length equals object length + noon shadow (Shafi'i madhhab)
- **Maghrib:** Same as sunset with -0.833° correction
- **Isha (Isya):** -18° (sun is 18° below the western horizon)

**Ihtiyat (Precautionary Adjustments):**

To ensure prayers are never performed before their actual time, Kemenag adds safety margins:
- All prayer times: +2 minutes
- Sunrise: -2 minutes (exception to encourage timely Fajr prayer)

**Special Calculations:**
- **Dhuha prayer:** Begins 28 minutes after sunrise (after ihtiyat adjustment)
- **Time formatting:** Uses ceiling rounding (always rounds up) rather than standard rounding

### Calculation Steps

1. Convert Gregorian date to Julian Day
2. Calculate solar position (declination and equation of time)
3. Determine solar transit time (true noon)
4. Compute hour angles for each prayer based on solar altitude
5. Convert hour angles to local time
6. Apply ihtiyat adjustments
7. Format times with ceiling rounding

For detailed mathematical formulas and worked examples, see [`docs/KEMENAG_METHOD.md`](docs/KEMENAG_METHOD.md).

## Building

### Desktop/CLI Build

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run example (Bekasi, November 21, 2025)
./build/libmuslim 2025 11 21 -6.2851291 106.9814968 7.0
```

### Android Build

```bash
# Requires ANDROID_HOME environment variable
./android.sh

# Output: libs/arm64-v8a/libmuslim.so
#         libs/x86_64/libmuslim.so
```

## Usage

### C API

```c
#include "prayertimes.h"

struct PrayerTimes times = calculate_prayer_times(
    2025,           // year
    11,             // month
    21,             // day
    -6.2851291,     // latitude (negative = South)
    106.9814968,    // longitude (positive = East)
    7.0             // timezone offset (WIB = UTC+7)
);

char buffer[16];
format_time_hm(times.fajr, buffer, sizeof(buffer));
printf("Fajr: %s\n", buffer);
```

### CLI Tool

```bash
./build/libmuslim <year> <month> <day> <latitude> <longitude> <timezone>
```

**Example output:**
```
Prayer Times for 2025-11-21 at (-6.285129, 106.981497), UTC+7.00

Fajr    = 04:05
Sunrise = 05:22
Dhuha   = 05:50
Dhuhr   = 11:41
Asr     = 15:04
Maghrib = 17:54
Isha    = 19:07
```

## Verification

The calculations have been verified against official Kemenag RI data sources and match within ±1 minute accuracy. See the worked examples in `docs/KEMENAG_METHOD.md` for detailed verification.

## License

```
Copyright 2025 Rizki Rachmad Rayyan

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

## Support

If you find this library useful, consider supporting its development:

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/rizukirr)

## Documentation

- [Kemenag Calculation Method](docs/KEMENAG_METHOD.md) - Detailed mathematical documentation (Indonesian)
- [Medium - Understanding Islamic Prayer Time Calculations: The Mathematics Behind LibMuslim Library](https://medium.com/@rizkirakasiwi09/understanding-islamic-prayer-time-calculations-the-mathematics-behind-libmuslim-library-ee169e3e97c3)

## Contributing

Contributions are welcome! Please ensure any changes to calculation methods are verified against official Kemenag data sources.
