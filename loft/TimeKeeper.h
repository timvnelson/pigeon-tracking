// Keeps UTC time: the UNO R4's internal RTC, corrected from the internet (NTP
// via the on-board Wi-Fi). Time can also be set by hand over USB serial.
#pragma once
#include <stdint.h>

namespace TimeKeeper {

void begin();
void update();               // call often; re-syncs from NTP when due
bool isValid();              // false until the clock has been set
uint32_t nowUtc();           // Unix epoch seconds (UTC)
bool setUtc(uint32_t epoch); // manual set; rejects values before 2026 or from 2100
bool syncNow();              // try NTP right now; true on success
uint32_t lastSyncUtc();      // 0 if never synced from NTP
bool hasWifiCredentials();

}  // namespace TimeKeeper
