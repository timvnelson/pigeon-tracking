#include "TimeKeeper.h"
#include "config.h"
#include <Arduino.h>
#include <RTC.h>
#include <WiFiS3.h>

// Wi-Fi details come from loft_settings.h (LOFT_WIFI_SSID / _PASSWORD).
// Without them the loft still works, but the clock has to be set by hand
// over USB ("settime").
#define HAVE_WIFI_SECRETS LOFT_HAS_WIFI

namespace TimeKeeper {

static bool     s_valid = false;
static uint32_t s_lastSync = 0;
#if HAVE_WIFI_SECRETS
static uint32_t s_lastAttemptMs = 0;
static bool     s_attempted = false;
static uint32_t s_waitS = 0;
#endif

static uint32_t rtcEpoch() {
  RTCTime t;
  RTC.getTime(t);
  return (uint32_t)t.getUnixTime();
}

void begin() {
  RTC.begin();
  // After a reset with power kept on, the RTC may still hold good time.
  if (RTC.isRunning() && rtcEpoch() >= MIN_VALID_EPOCH) s_valid = true;
}

bool hasWifiCredentials() { return HAVE_WIFI_SECRETS; }

bool isValid() { return s_valid && rtcEpoch() >= MIN_VALID_EPOCH; }

uint32_t nowUtc() { return rtcEpoch(); }

uint32_t lastSyncUtc() { return s_lastSync; }

bool setUtc(uint32_t epoch) {
  if (epoch < MIN_VALID_EPOCH || epoch >= MAX_VALID_EPOCH) return false;
  RTCTime t((time_t)epoch);
  RTC.setTime(t);
  s_valid = true;
  return true;
}

bool syncNow() {
#if HAVE_WIFI_SECRETS
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("[time] Wi-Fi module not found"));
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("[time] connecting to Wi-Fi \""));
    Serial.print(LOFT_WIFI_SSID);
    Serial.println('"');
    WiFi.begin(LOFT_WIFI_SSID, LOFT_WIFI_PASSWORD);  // blocks for up to ~10 s
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[time] Wi-Fi connect failed"));
    return false;
  }
  // The ESP32 co-processor fetches NTP time; it can take a few seconds.
  for (int i = 0; i < 15; i++) {
    const uint32_t epoch = WiFi.getTime();
    if (epoch >= MIN_VALID_EPOCH) {
      setUtc(epoch);
      s_lastSync = epoch;
      Serial.print(F("[time] NTP sync OK, epoch "));
      Serial.println(epoch);
      return true;
    }
    delay(1000);
  }
  Serial.println(F("[time] NTP: no time received"));
  return false;
#else
  return false;
#endif
}

void update() {
#if HAVE_WIFI_SECRETS
  // Try at boot, then every NTP_RESYNC_INTERVAL_S after a success, or every
  // NTP_RETRY_INTERVAL_S after a failure. If a sync fails the loft keeps
  // running on the RTC, which is only slightly off.
  const uint32_t nowMs = millis();
  if (s_attempted && (nowMs - s_lastAttemptMs) / 1000UL < s_waitS) return;
  s_attempted = true;
  const bool ok = syncNow();
  s_lastAttemptMs = millis();
  s_waitS = ok ? NTP_RESYNC_INTERVAL_S : NTP_RETRY_INTERVAL_S;
#endif
}

}  // namespace TimeKeeper
