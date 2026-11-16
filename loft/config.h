// Loft controller configuration. Change values here, not in the code.
#pragma once
#include <stdint.h>

// ---- Location (rounded on purpose: the repo is public) ----------------------
// Rounding to whole degrees shifts sunrise/sunset by well under a minute.
static const double LOFT_LATITUDE  = 34.00;   // degrees, north positive
static const double LOFT_LONGITUDE = -118.00; // degrees, east positive

// ---- Local time (used ONLY for printing; all scheduling is done in UTC) -----
static const int16_t TZ_STANDARD_OFFSET_MIN = -8 * 60; // Pacific Standard Time
static const bool    TZ_USE_US_DST          = true;    // US daylight-saving rules

// ---- Doors ------------------------------------------------------------------
// Each Ripom D3786 door opens when its power input is live and closes when the
// power is removed (Light Sensitive mode). One relay per door switches 5 V.
static const uint8_t NUM_DOORS = 5;
static const uint8_t DOOR_RELAY_PINS[NUM_DOORS] = {2, 3, 4, 5, 6}; // -> relay IN1..IN5

// The ELEGOO 8-channel relay module is active-LOW: LOW = relay ON.
static const bool RELAY_ACTIVE_LOW = true;

// Each door can draw up to 1 A. Turn them on one at a time, this far apart,
// so the 5 V / 5 A UPS doesn't see a 5 A surge all at once.
static const uint32_t DOOR_STAGGER_MS = 5000;

// When to open and close, relative to the calculated sunrise and sunset.
// Positive = later. Closing a little after sunset gives late birds time to get home.
static const int16_t OPEN_OFFSET_MIN  = 0;   // minutes after sunrise
static const int16_t CLOSE_OFFSET_MIN = 30;  // minutes after sunset

// ---- Time keeping -----------------------------------------------------------
// The UNO R4's internal RTC drifts, so re-sync from the internet regularly.
static const uint32_t NTP_RESYNC_INTERVAL_S = 6UL * 3600UL;  // every 6 hours
static const uint32_t NTP_RETRY_INTERVAL_S  = 5UL * 60UL;    // after a failed try

// Any clock reading earlier than this is treated as "time not set".
static const uint32_t MIN_VALID_EPOCH = 1767225600UL; // 2026-01-01 00:00:00 UTC
