// Bird Loft controller: Arduino UNO R4 WiFi
//
// Feature: entrance doors follow the sun.
//   The 5 Ripom D3786 doors (in Light Sensitive mode) open when their power
//   input is live and close when it is removed. This sketch switches that
//   power through one relay per door: ON from sunrise (+OPEN_OFFSET_MIN) to
//   sunset (+CLOSE_OFFSET_MIN), OFF otherwise. Sunrise and sunset are
//   calculated offline from the date and the location in loft_settings.h.
//
// Safety rule: if the clock has not been set, the doors stay CLOSED.
//
// USB serial (115200 baud) commands, for testing:
//   status          show the time, schedule and door state
//   open / close    override the schedule until its next open/close time
//   auto            cancel the override and follow the schedule
//   settime <epoch> set the clock by hand (UTC Unix seconds, e.g. from
//                   https://www.unixtimestamp.com)
//   sync            fetch the time from the internet now (needs Wi-Fi in loft_settings.h)
//   help            list commands

#include "config.h"
#include "DoorController.h"
#include "SolarSchedule.h"
#include "TimeKeeper.h"

static DoorController doors;

enum class Override : uint8_t { None, Open, Close };
static Override s_override = Override::None;
static int8_t   s_lastScheduled = -1;   // -1 = not yet evaluated
static uint32_t s_nextChangeUtc = 0;
static uint32_t s_lastEvalMs = 0;

// ---------------------------------------------------------------- time display

static int32_t lastSundayOf(int32_t year, uint32_t month) {
  const int32_t last = (month == 12 ? Solar::daysFromCivil(year + 1, 1, 1)
                                    : Solar::daysFromCivil(year, month + 1, 1)) - 1;
  return last - ((last + 4) % 7);  // 1970-01-01 was a Thursday
}

static uint32_t usDstStartUtc(int32_t year) {  // 2nd Sunday in March, 02:00 standard time
  const int32_t d = Solar::daysFromCivil(year, 3, 1);
  const int32_t firstSun = d + (7 - ((d + 4) % 7)) % 7;  // 1970-01-01 was a Thursday
  return (uint32_t)(firstSun + 7) * 86400UL + 2UL * 3600UL - TZ_STANDARD_OFFSET_MIN * 60L;
}

static uint32_t usDstEndUtc(int32_t year) {    // 1st Sunday in November, 02:00 daylight time
  const int32_t d = Solar::daysFromCivil(year, 11, 1);
  const int32_t firstSun = d + (7 - ((d + 4) % 7)) % 7;
  return (uint32_t)firstSun * 86400UL + 2UL * 3600UL - (TZ_STANDARD_OFFSET_MIN + 60) * 60L;
}

static int32_t localOffsetSeconds(uint32_t utc, bool &isDst) {
  isDst = false;
  int32_t y; uint32_t m, d;
  Solar::civilFromDays((int32_t)(utc / 86400UL), y, m, d);
  if (DST_RULE == DST_RULE_US) {
    isDst = utc >= usDstStartUtc(y) && utc < usDstEndUtc(y);
  } else if (DST_RULE == DST_RULE_EU) {  // both changes at 01:00 UTC
    isDst = utc >= (uint32_t)lastSundayOf(y, 3) * 86400UL + 3600UL &&
            utc <  (uint32_t)lastSundayOf(y, 10) * 86400UL + 3600UL;
  }
  return (TZ_STANDARD_OFFSET_MIN + (isDst ? 60 : 0)) * 60L;
}

static void print2(uint32_t v) {
  if (v < 10) Serial.print('0');
  Serial.print(v);
}

static void printLocalTime(uint32_t utc) {
  bool dst;
  const int64_t local = (int64_t)utc + localOffsetSeconds(utc, dst);
  int32_t y; uint32_t m, d;
  Solar::civilFromDays((int32_t)(local / 86400), y, m, d);
  const uint32_t secs = (uint32_t)(local % 86400);
  Serial.print(y); Serial.print('-'); print2(m); Serial.print('-'); print2(d);
  Serial.print(' '); print2(secs / 3600); Serial.print(':'); print2((secs / 60) % 60);
  Serial.print(dst ? F(" daylight") : F(" standard"));
}

// ---------------------------------------------------------------- scheduling

// Returns 1 (open) or 0 (closed) according to the clock and the sun.
static int8_t scheduledState() {
  if (!TimeKeeper::isValid()) {
    s_nextChangeUtc = 0;
    return 0;  // fail-safe: unknown time -> doors closed
  }
  return Solar::doorsShouldBeOpen(TimeKeeper::nowUtc(), s_nextChangeUtc) ? 1 : 0;
}

static void evaluate() {
  const int8_t sched = scheduledState();
  if (sched != s_lastScheduled) {
    if (s_lastScheduled != -1) {
      Serial.println(sched ? F("[sched] sunrise: time to OPEN") : F("[sched] sunset: time to CLOSE"));
      if (s_override != Override::None) {
        Serial.println(F("[sched] manual override ended; following schedule again"));
        s_override = Override::None;
      }
    }
    s_lastScheduled = sched;
  }
  bool open = sched == 1;
  if (s_override == Override::Open)  open = true;
  if (s_override == Override::Close) open = false;
  doors.setOpen(open);
}

// ---------------------------------------------------------------- serial commands

static void printStatus() {
  Serial.print(F("---- ")); Serial.print(F(LOFT_NAME)); Serial.println(F(" status ----"));
  if (TimeKeeper::isValid()) {
    const uint32_t now = TimeKeeper::nowUtc();
    Serial.print(F("Time:        ")); printLocalTime(now);
    Serial.print(F("  (UTC epoch ")); Serial.print(now); Serial.println(')');
    const uint32_t sync = TimeKeeper::lastSyncUtc();
    Serial.print(F("Last NTP:    "));
    if (sync) { printLocalTime(sync); Serial.println(); } else Serial.println(F("never (set by hand)"));
    Serial.print(F("Schedule:    ")); Serial.println(s_lastScheduled == 1 ? F("OPEN (daytime)") : F("CLOSED (night)"));
    if (s_nextChangeUtc) {
      Serial.print(F("Next change: ")); printLocalTime(s_nextChangeUtc); Serial.println();
    }
  } else {
    Serial.println(F("Time:        NOT SET -> doors held CLOSED"));
    Serial.println(TimeKeeper::hasWifiCredentials()
                       ? F("             waiting for Wi-Fi/NTP; or use: settime <epoch>")
                       : F("             no Wi-Fi in loft_settings.h; use: settime <epoch>"));
  }
  Serial.print(F("Override:    "));
  Serial.println(s_override == Override::Open ? F("OPEN") : s_override == Override::Close ? F("CLOSE") : F("none"));
  Serial.print(F("Doors:       "));
  for (uint8_t i = 0; i < NUM_DOORS; i++) {
    Serial.print(i + 1); Serial.print(doors.isPowered(i) ? F("=ON ") : F("=off "));
  }
  Serial.println();
}

static void handleCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();
  if (cmd.length() == 0) return;
  if (cmd == "status") {
    printStatus();
  } else if (cmd == "open") {
    s_override = Override::Open;
    Serial.println(F("[cmd] override: OPEN until the next scheduled change ('auto' to cancel)"));
  } else if (cmd == "close") {
    s_override = Override::Close;
    Serial.println(F("[cmd] override: CLOSE until the next scheduled change ('auto' to cancel)"));
  } else if (cmd == "auto") {
    s_override = Override::None;
    Serial.println(F("[cmd] following the schedule"));
  } else if (cmd.startsWith("settime ")) {
    const uint32_t epoch = (uint32_t)strtoul(cmd.substring(8).c_str(), nullptr, 10);
    if (TimeKeeper::setUtc(epoch)) {
      Serial.print(F("[cmd] clock set to ")); printLocalTime(epoch); Serial.println();
    } else {
      Serial.println(F("[cmd] rejected: expected a UTC Unix time after 2026-01-01"));
    }
  } else if (cmd == "sync") {
    if (!TimeKeeper::hasWifiCredentials()) Serial.println(F("[cmd] no Wi-Fi configured in loft_settings.h"));
    else TimeKeeper::syncNow();
  } else {
    Serial.println(F("Commands: status | open | close | auto | settime <epoch> | sync"));
  }
  evaluate();
}

static void pollSerial() {
  static String line;
  while (Serial.available()) {
    const char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      handleCommand(line);
      line = "";
    } else if (line.length() < 40) {
      line += c;
    }
  }
}

// ---------------------------------------------------------------- main

void setup() {
  doors.begin();  // FIRST: make sure every door is unpowered (closed)
  Serial.begin(115200);
  const uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 2000) {}  // brief wait for USB; don't block without it
  Serial.print(F("\n[boot] Bird Loft controller: ")); Serial.print(F(LOFT_NAME));
  if (LOFT_OWNER[0]) { Serial.print(F(" (")); Serial.print(F(LOFT_OWNER)); Serial.print(')'); }
  Serial.println();
  TimeKeeper::begin();
  TimeKeeper::update();  // first NTP attempt (if Wi-Fi is configured)
  evaluate();
  printStatus();
}

void loop() {
  pollSerial();
  TimeKeeper::update();
  if (millis() - s_lastEvalMs >= 1000) {
    s_lastEvalMs = millis();
    evaluate();
  }
  doors.update();
}
