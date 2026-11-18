#include "TimeKeeper.h"
#include "config.h"
#include <Arduino.h>
#include <RTC.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>

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
  if (epoch < MIN_VALID_EPOCH) return false;
  RTCTime t((time_t)epoch);
  RTC.setTime(t);
  s_valid = true;
  return true;
}

#if HAVE_WIFI_SECRETS
// Ask an NTP server for the time ourselves. WiFi.getTime() (the ESP32
// co-processor's built-in NTP) returned 0 on a real UNO R4 WiFi with
// co-processor firmware 0.6.0, even though a plain UDP request to the same
// pool worked, so we don't use it. Returns Unix epoch seconds, or 0.
static uint32_t ntpQuery() {
  // Right after Wi-Fi connects, DNS can fail for a few seconds (seen at boot
  // on the real board), so retry before giving up.
  IPAddress server;
  bool resolved = false;
  for (uint8_t i = 0; i < 10 && !resolved; i++) {
    resolved = (WiFi.hostByName(NTP_SERVER, server) == 1);
    if (!resolved) delay(1000);
  }
  if (!resolved) {
    Serial.println(F("[time] NTP: DNS lookup failed"));
    return 0;
  }
  WiFiUDP udp;
  if (!udp.begin(2390)) return 0;
  uint32_t result = 0;
  for (uint8_t attempt = 0; attempt < NTP_TRIES && result == 0; attempt++) {
    uint8_t pkt[48] = {0};
    pkt[0] = 0x23;  // leap indicator 0, version 4, mode 3 (client)
    udp.beginPacket(server, 123);
    udp.write(pkt, sizeof(pkt));
    udp.endPacket();

    const uint32_t start = millis();
    while (millis() - start < NTP_TIMEOUT_MS) {
      if (udp.parsePacket() >= 48) {
        udp.read(pkt, sizeof(pkt));
        const uint8_t mode = pkt[0] & 0x07;
        const uint8_t leap = pkt[0] >> 6;
        const uint8_t stratum = pkt[1];
        // A real answer comes from a server (mode 4), synced (leap != 3),
        // and not a "kiss-of-death" (stratum 0).
        if (mode == 4 && leap != 3 && stratum != 0) {
          const uint32_t ntpSecs = ((uint32_t)pkt[40] << 24) | ((uint32_t)pkt[41] << 16) |
                                   ((uint32_t)pkt[42] << 8) | (uint32_t)pkt[43];
          const uint32_t epoch = ntpSecs - 2208988800UL;  // 1900 -> 1970
          if (epoch >= MIN_VALID_EPOCH) result = epoch;
        }
        break;
      }
      delay(10);
    }
  }
  udp.stop();
  return result;
}
#endif

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
  const uint32_t epoch = ntpQuery();
  if (epoch != 0) {
    setUtc(epoch);
    s_lastSync = epoch;
    Serial.print(F("[time] NTP sync OK, epoch "));
    Serial.println(epoch);
    return true;
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
