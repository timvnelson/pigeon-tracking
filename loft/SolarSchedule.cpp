#include "SolarSchedule.h"
#include "config.h"
#include <math.h>

namespace Solar {

static const double DEG = 3.14159265358979323846 / 180.0;

// Howard Hinnant's civil-date algorithms (proleptic Gregorian calendar).
int32_t daysFromCivil(int32_t y, uint32_t m, uint32_t d) {
  y -= m <= 2;
  const int32_t era = (y >= 0 ? y : y - 399) / 400;
  const uint32_t yoe = (uint32_t)(y - era * 400);
  const uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)doe - 719468;
}

void civilFromDays(int32_t z, int32_t &y, uint32_t &m, uint32_t &d) {
  z += 719468;
  const int32_t era = (z >= 0 ? z : z - 146096) / 146097;
  const uint32_t doe = (uint32_t)(z - era * 146097);
  const uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const uint32_t mp = (5 * doy + 2) / 153;
  d = doy - (153 * mp + 2) / 5 + 1;
  m = mp < 10 ? mp + 3 : mp - 9;
  y = (int32_t)yoe + era * 400 + (m <= 2);
}

SunDay sunTimesUtcMinutes(int32_t year, uint32_t month, uint32_t day,
                          double lat, double lon,
                          double &sunriseMin, double &sunsetMin) {
  // Julian Day at 00:00 UTC, then shifted to approximate local solar noon.
  const double jd0 = daysFromCivil(year, month, day) + 2440587.5;
  const double jd = jd0 + (720.0 - 4.0 * lon) / 1440.0;
  const double T = (jd - 2451545.0) / 36525.0;  // Julian centuries since J2000

  const double L0 = fmod(280.46646 + T * (36000.76983 + T * 0.0003032), 360.0);
  const double M = 357.52911 + T * (35999.05029 - 0.0001537 * T);
  const double e = 0.016708634 - T * (0.000042037 + 0.0000001267 * T);
  const double C = sin(M * DEG) * (1.914602 - T * (0.004817 + 0.000014 * T)) +
                   sin(2 * M * DEG) * (0.019993 - 0.000101 * T) +
                   sin(3 * M * DEG) * 0.000289;
  const double omega = 125.04 - 1934.136 * T;
  const double lambda = L0 + C - 0.00569 - 0.00478 * sin(omega * DEG);
  const double eps0 =
      23.0 + (26.0 + (21.448 - T * (46.815 + T * (0.00059 - T * 0.001813))) / 60.0) / 60.0;
  const double eps = eps0 + 0.00256 * cos(omega * DEG);
  const double decl = asin(sin(eps * DEG) * sin(lambda * DEG));

  const double y = tan(eps * DEG / 2) * tan(eps * DEG / 2);
  const double eqTime =
      4.0 / DEG *
      (y * sin(2 * L0 * DEG) - 2 * e * sin(M * DEG) +
       4 * e * y * sin(M * DEG) * cos(2 * L0 * DEG) -
       0.5 * y * y * sin(4 * L0 * DEG) - 1.25 * e * e * sin(2 * M * DEG));

  // 90.833 degrees = standard sunrise/sunset (refraction + solar disc radius).
  const double cosHA = cos(90.833 * DEG) / (cos(lat * DEG) * cos(decl)) -
                       tan(lat * DEG) * tan(decl);
  if (cosHA > 1.0) return SunDay::AlwaysDown;  // polar night

  const double solarNoon = 720.0 - 4.0 * lon - eqTime;
  if (cosHA < -1.0) {  // midnight sun: daylight from one solar midnight to the next
    sunriseMin = solarNoon - 720.0;
    sunsetMin = solarNoon + 720.0;
    return SunDay::AlwaysUp;
  }
  const double haDeg = acos(cosHA) / DEG;
  sunriseMin = solarNoon - 4.0 * haDeg;
  sunsetMin = solarNoon + 4.0 * haDeg;
  return SunDay::Normal;
}

bool doorsShouldBeOpen(uint32_t nowUtc, uint32_t &nextChangeUtc) {
  // Stateless: build the open windows for yesterday, today and tomorrow (UTC),
  // join any that overlap, and see whether `now` falls inside one. This gives
  // the right answer straight after a reboot, at any time of day.
  struct Window { uint32_t start, end; bool allDay; int8_t day; };
  Window w[3];
  uint8_t n = 0;
  const int32_t today = (int32_t)(nowUtc / 86400UL);

  for (int8_t i = 0; i < 3; i++) {
    const int32_t dd = today - 1 + i;
    int32_t y; uint32_t m, d;
    civilFromDays(dd, y, m, d);
    double riseMin = 0, setMin = 0;
    const SunDay kind = sunTimesUtcMinutes(y, m, d, LOFT_LATITUDE, LOFT_LONGITUDE, riseMin, setMin);
    if (kind == SunDay::AlwaysDown) continue;  // no daylight: stays closed

    const double midnight = (double)dd * 86400.0;
    Window win;
    win.allDay = (kind == SunDay::AlwaysUp);
    win.day = i;
    if (win.allDay) {
      // No sunrise or sunset to offset from. Widen the window by an hour on
      // each side so that consecutive all-day windows overlap and the doors
      // never close for a moment between them.
      win.start = (uint32_t)(midnight + (riseMin - 60.0) * 60.0);
      win.end   = (uint32_t)(midnight + (setMin + 60.0) * 60.0);
    } else {
      win.start = (uint32_t)(midnight + (riseMin + OPEN_OFFSET_MIN) * 60.0);
      win.end   = (uint32_t)(midnight + (setMin + CLOSE_OFFSET_MIN) * 60.0);
    }
    if (win.end > win.start) w[n++] = win;
  }

  // The windows are in date order. Walk through them, joining overlapping
  // ones into continuous open periods.
  nextChangeUtc = 0;
  for (uint8_t i = 0; i < n;) {
    const uint32_t start = w[i].start;
    uint32_t end = w[i].end;
    uint8_t last = i;
    for (uint8_t j = i + 1; j < n && w[j].start <= end; j++) {
      if (w[j].end > end) end = w[j].end;
      last = j;
    }
    if (nowUtc >= start && nowUtc < end) {
      // Open now. If the period runs into tomorrow's all-day window, it may
      // carry on past what we calculated, so the closing time is unknown.
      const bool openEnded = (w[last].day == 2 && w[last].allDay);
      nextChangeUtc = openEnded ? 0 : end;
      return true;
    }
    if (start > nowUtc) {  // the first period still to come
      nextChangeUtc = start;
      return false;
    }
    i = last + 1;
  }
  return false;
}

}  // namespace Solar
