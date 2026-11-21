// Tests for the offline sunrise/sunset maths and the door schedule
// (loft/SolarSchedule.cpp), run on a computer: no board needed.
// Build and run with tests/host/run.sh.
#include <cmath>
#include <cstdio>
#include "SolarSchedule.h"
#include "config.h"

double g_lat, g_lon;
static int failures = 0;

#define CHECK(cond, ...)                         \
  do {                                           \
    if (!(cond)) {                               \
      failures++;                                \
      std::printf("FAIL %s:%d: ", __FILE__, __LINE__); \
      std::printf(__VA_ARGS__);                  \
      std::printf("\n");                         \
    }                                            \
  } while (0)

static uint32_t epochOf(int32_t y, uint32_t m, uint32_t d, uint32_t hour) {
  return (uint32_t)Solar::daysFromCivil(y, m, d) * 86400u + hour * 3600u;
}

// Firmware sunrise (or sunset) nearest to `ref`, in seconds from it.
static double nearestDiffS(double lat, double lon, uint32_t ref, bool rise) {
  const int32_t day = (int32_t)(ref / 86400u);
  double best = 1e9;
  for (int32_t dd = day - 1; dd <= day + 1; dd++) {
    int32_t y; uint32_t m, d;
    Solar::civilFromDays(dd, y, m, d);
    double riseMin, setMin;
    if (Solar::sunTimesUtcMinutes(y, m, d, lat, lon, riseMin, setMin) != Solar::SunDay::Normal) continue;
    const double t = dd * 86400.0 + (rise ? riseMin : setMin) * 60.0;
    if (std::fabs(t - ref) < std::fabs(best)) best = t - ref;
  }
  return best;
}

// Reference times from the astral 3.2 Python library, generated once.
struct Ref { const char *city; double lat, lon; uint32_t sunrise, sunset; };
static const Ref REFS[] = {
  {"Los Angeles", 34.05, -118.24, 1774014995u, 1773972225u},  // 2026-03-20
  {"Los Angeles", 34.05, -118.24, 1782045737u, 1782011232u},  // 2026-06-21
  {"Los Angeles", 34.05, -118.24, 1790170926u, 1790128153u},  // 2026-09-23
  {"Los Angeles", 34.05, -118.24, 1797864894u, 1797814020u},  // 2026-12-21
  {"Los Angeles", 34.05, -118.24, 1802615938u, 1802568886u},  // 2027-02-14
  {"New York", 40.71, -74.01, 1774004371u, 1774048078u},  // 2026-03-20
  {"New York", 40.71, -74.01, 1782033919u, 1782001816u},  // 2026-06-21
  {"New York", 40.71, -74.01, 1790160292u, 1790203869u},  // 2026-09-23
  {"New York", 40.71, -74.01, 1797855411u, 1797888694u},  // 2026-12-21
  {"New York", 40.71, -74.01, 1802605901u, 1802644155u},  // 2027-02-14
  {"Mexico City", 19.43, -99.13, 1774010438u, 1773967626u},  // 2026-03-20
  {"Mexico City", 19.43, -99.13, 1782043167u, 1782004627u},  // 2026-06-21
  {"Mexico City", 19.43, -99.13, 1790166350u, 1790123544u},  // 2026-09-23
  {"Mexico City", 19.43, -99.13, 1797858367u, 1797811371u},  // 2026-12-21
  {"Mexico City", 19.43, -99.13, 1802610354u, 1802565313u},  // 2027-02-14
  {"Honolulu", 21.31, -157.86, 1774024522u, 1773981725u},  // 2026-03-20
  {"Honolulu", 21.31, -157.86, 1782057036u, 1782018954u},  // 2026-06-21
  {"Honolulu", 21.31, -157.86, 1790180447u, 1790137632u},  // 2026-09-23
  {"Honolulu", 21.31, -157.86, 1797872689u, 1797825249u},  // 2026-12-21
  {"Honolulu", 21.31, -157.86, 1802624559u, 1802579296u},  // 2027-02-14
  {"London", 51.51, -0.13, 1773986619u, 1774030394u},  // 2026-03-20
  {"London", 51.51, -0.13, 1782013407u, 1782073273u},  // 2026-06-21
  {"London", 51.51, -0.13, 1790142503u, 1790186186u},  // 2026-09-23
  {"London", 51.51, -0.13, 1797840247u, 1797868383u},  // 2026-12-21
  {"London", 51.51, -0.13, 1802589429u, 1802625179u},  // 2027-02-14
  {"Madrid", 40.42, -3.70, 1773987517u, 1774031190u},  // 2026-03-20
  {"Madrid", 40.42, -3.70, 1782017100u, 1782071294u},  // 2026-06-21
  {"Madrid", 40.42, -3.70, 1790143407u, 1790187014u},  // 2026-09-23
  {"Madrid", 40.42, -3.70, 1797838476u, 1797871869u},  // 2026-12-21
  {"Madrid", 40.42, -3.70, 1802589015u, 1802627293u},  // 2027-02-14
  {"Tokyo", 35.68, 139.69, 1774039463u, 1773996739u},  // 2026-03-20
  {"Tokyo", 35.68, 139.69, 1782069966u, 1782036003u},  // 2026-06-21
  {"Tokyo", 35.68, 139.69, 1790195433u, 1790152630u},  // 2026-09-23
  {"Tokyo", 35.68, 139.69, 1797889657u, 1797838267u},  // 2026-12-21
  {"Tokyo", 35.68, 139.69, 1802640546u, 1802593267u},  // 2027-02-14
  {"Sydney", -33.87, 151.21, 1774036735u, 1773994003u},  // 2026-03-20
  {"Sydney", -33.87, 151.21, 1782075624u, 1782024815u},  // 2026-06-21
  {"Sydney", -33.87, 151.21, 1790192566u, 1790149904u},  // 2026-09-23
  {"Sydney", -33.87, 151.21, 1797878481u, 1797843911u},  // 2026-12-21
  {"Sydney", -33.87, 151.21, 1802633400u, 1802594944u},  // 2027-02-14
  {"Auckland", -36.85, 174.76, 1774031073u, 1773988368u},  // 2026-03-20
  {"Auckland", -36.85, 174.76, 1782070449u, 1782018684u},  // 2026-06-21
  {"Auckland", -36.85, 174.76, 1790186906u, 1790144260u},  // 2026-09-23
  {"Auckland", -36.85, 174.76, 1797872324u, 1797838760u},  // 2026-12-21
  {"Auckland", -36.85, 174.76, 1802627485u, 1802589559u},  // 2027-02-14
  {"Reykjavik", 64.15, -21.94, 1773991738u, 1774035781u},  // 2026-03-20
  {"Reykjavik", 64.15, -21.94, 1782010574u, 1782000163u},  // 2026-06-21
  {"Reykjavik", 64.15, -21.94, 1790147635u, 1790191481u},  // 2026-09-23
  {"Reykjavik", 64.15, -21.94, 1797852194u, 1797866908u},  // 2026-12-21
  {"Reykjavik", 64.15, -21.94, 1802597264u, 1802627836u},  // 2027-02-14
};

static void testCivilDates() {
  CHECK(Solar::daysFromCivil(1970, 1, 1) == 0, "1970-01-01");
  CHECK(Solar::daysFromCivil(2026, 1, 1) == 20454, "2026-01-01");
  for (int32_t z = 0; z < Solar::daysFromCivil(2100, 1, 1); z++) {
    int32_t y; uint32_t m, d;
    Solar::civilFromDays(z, y, m, d);
    if (Solar::daysFromCivil(y, m, d) != z) { CHECK(false, "round trip of day %d", z); break; }
  }
}

static void testAgainstReference() {
  double worst = 0;
  for (const Ref &r : REFS) {
    const double rise = nearestDiffS(r.lat, r.lon, r.sunrise, true);
    const double set = nearestDiffS(r.lat, r.lon, r.sunset, false);
    CHECK(std::fabs(rise) <= 120, "%s sunrise off by %.0f s", r.city, rise);
    CHECK(std::fabs(set) <= 120, "%s sunset off by %.0f s", r.city, set);
    worst = std::fmax(worst, std::fmax(std::fabs(rise), std::fabs(set)));
  }
  std::printf("sunrise/sunset vs reference: %zu places/dates, worst %.0f s\n",
              sizeof(REFS) / sizeof(REFS[0]), worst);
}

// Steps through a whole year a minute at a time and checks that the reported
// next change is always when the doors actually change.
struct ScanResult { int opens; int closes; uint32_t openMinutes; };
static ScanResult scanYear(const char *name, double lat, double lon) {
  g_lat = lat; g_lon = lon;
  const uint32_t t0 = epochOf(2026, 1, 1, 0), t1 = epochOf(2027, 1, 1, 0);
  ScanResult res = {0, 0, 0};
  uint32_t next = 0, prevNext = 0;
  int prev = -1;
  for (uint32_t t = t0; t < t1; t += 60) {
    const bool open = Solar::doorsShouldBeOpen(t, next);
    CHECK(next == 0 || next > t, "%s: next change %u not after now %u", name, next, t);
    if (prev != -1 && (int)open != prev) {
      // The change happened in (t-60, t]; the last prediction must say so.
      CHECK(prevNext > t - 60 && prevNext <= t, "%s: change at %u, predicted %u", name, t, prevNext);
      open ? res.opens++ : res.closes++;
    }
    res.openMinutes += open;
    prev = open; prevNext = next;
  }
  return res;
}

static void testYearScans() {
  struct { const char *name; double lat, lon; } places[] = {
      {"Los Angeles", 34.05, -118.24}, {"London", 51.51, -0.13}, {"Tokyo", 35.68, 139.69},
      {"Auckland", -36.85, 174.76},    {"Honolulu", 21.31, -157.86}};
  for (auto &p : places) {
    const ScanResult r = scanYear(p.name, p.lat, p.lon);
    // One opening a day (2026-01-01 starts mid-cycle in some places).
    CHECK(r.opens >= 364 && r.opens <= 366, "%s: %d openings in a year", p.name, r.opens);
  }
}

// Above the Arctic Circle: open all day in the midnight sun, closed in the
// polar night, and the doors never close for a moment in between.
static void testMidnightSun() {
  g_lat = 69.65; g_lon = 18.96;  // Tromso, Norway
  uint32_t next;
  CHECK(Solar::doorsShouldBeOpen(epochOf(2026, 6, 21, 11), next), "Tromso midsummer noon should be open");
  CHECK(Solar::doorsShouldBeOpen(epochOf(2026, 6, 21, 23), next), "Tromso midsummer midnight should be open");
  CHECK(!Solar::doorsShouldBeOpen(epochOf(2026, 12, 21, 11), next), "Tromso midwinter noon should be closed");
  for (uint32_t t = epochOf(2026, 6, 1, 0); t < epochOf(2026, 7, 15, 0); t += 60) {
    if (!Solar::doorsShouldBeOpen(t, next)) { CHECK(false, "Tromso closed at %u in the midnight sun", t); break; }
  }
  scanYear("Tromso", 69.65, 18.96);  // next-change consistency through the transitions
}

int main() {
  testCivilDates();
  testAgainstReference();
  testYearScans();
  testMidnightSun();
  if (failures) {
    std::printf("%d check(s) FAILED\n", failures);
    return 1;
  }
  std::printf("all host tests passed\n");
  return 0;
}
