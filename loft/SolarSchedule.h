// Offline sunrise/sunset calculation (NOAA algorithm, accurate to about 1 minute)
// and the open/closed door schedule built on it. All times are UTC Unix epochs.
#pragma once
#include <stdint.h>

namespace Solar {

// Days since 1970-01-01 for a civil date, and back again.
int32_t daysFromCivil(int32_t year, uint32_t month, uint32_t day);
void civilFromDays(int32_t days, int32_t &year, uint32_t &month, uint32_t &day);

// Sunrise and sunset for the solar day whose noon falls on the given UTC date,
// in minutes after 00:00 UTC of that date. Values can be < 0 or > 1440 (in
// California, sunset falls on the next UTC day). Returns false if the sun
// never rises or never sets that day.
bool sunTimesUtcMinutes(int32_t year, uint32_t month, uint32_t day,
                        double latitude, double longitude,
                        double &sunriseMin, double &sunsetMin);

// Should the doors be open at `nowUtc`, given the configured location and
// offsets? `nextChangeUtc` is set to the next scheduled open or close (0 if
// unknown).
bool doorsShouldBeOpen(uint32_t nowUtc, uint32_t &nextChangeUtc);

}  // namespace Solar
