// Loft controller configuration.
//
// Two layers:
//   loft_settings.h : unique to each loft (location, owner, Wi-Fi...). Private
//                     and git-ignored. Make it from loft_settings.example.h.
//   this file       : project defaults shared by every loft (pins, timings).
#pragma once
#include <stdint.h>

// Daylight-saving rules that loft_settings.h can choose from.
#define DST_RULE_NONE 0
#define DST_RULE_US   1  // 2nd Sunday March to 1st Sunday November, 02:00 local
#define DST_RULE_EU   2  // last Sunday March to last Sunday October, 01:00 UTC

#if !__has_include("loft_settings.h")
#error "Missing loft/loft_settings.h: copy loft_settings.example.h to loft_settings.h and fill in your loft's details."
#endif
#include "loft_settings.h"

// ---- Required per-loft settings ---------------------------------------------
#if !defined(LOFT_LATITUDE) || !defined(LOFT_LONGITUDE)
#error "loft_settings.h must define LOFT_LATITUDE and LOFT_LONGITUDE."
#endif
#if !defined(LOFT_TZ_STANDARD_OFFSET_MIN)
#error "loft_settings.h must define LOFT_TZ_STANDARD_OFFSET_MIN."
#endif
static_assert(LOFT_LATITUDE >= -90.0 && LOFT_LATITUDE <= 90.0,
              "LOFT_LATITUDE must be between -90 and 90 (south is negative).");
static_assert(LOFT_LONGITUDE >= -180.0 && LOFT_LONGITUDE <= 180.0,
              "LOFT_LONGITUDE must be between -180 and 180 (west is negative).");
static_assert(LOFT_LATITUDE != 0.0 || LOFT_LONGITUDE != 0.0,
              "LOFT_LATITUDE and LOFT_LONGITUDE are still 0.00: set your loft's location in loft_settings.h.");

// ---- Optional per-loft settings, with defaults ------------------------------
#ifndef LOFT_NAME
#define LOFT_NAME "Loft"
#endif
#ifndef LOFT_OWNER
#define LOFT_OWNER ""
#endif
#ifndef LOFT_DST_RULE
#define LOFT_DST_RULE DST_RULE_NONE
#endif
#ifndef LOFT_NUM_DOORS
#define LOFT_NUM_DOORS 5
#endif
#ifndef LOFT_OPEN_OFFSET_MIN
#define LOFT_OPEN_OFFSET_MIN 0
#endif
#ifndef LOFT_CLOSE_OFFSET_MIN
#define LOFT_CLOSE_OFFSET_MIN 30
#endif
#if defined(LOFT_WIFI_SSID) && defined(LOFT_WIFI_PASSWORD)
#define LOFT_HAS_WIFI 1
#else
#define LOFT_HAS_WIFI 0
#endif

static const int16_t TZ_STANDARD_OFFSET_MIN = LOFT_TZ_STANDARD_OFFSET_MIN;
static const uint8_t DST_RULE = LOFT_DST_RULE;

// When to open and close, relative to the calculated sunrise and sunset.
// Positive = later. Closing a little after sunset gives late birds time to get home.
static const int16_t OPEN_OFFSET_MIN  = LOFT_OPEN_OFFSET_MIN;
static const int16_t CLOSE_OFFSET_MIN = LOFT_CLOSE_OFFSET_MIN;

// ---- Doors (project standard wiring) ----------------------------------------
// Each Ripom D3786 door opens when its power input is live and closes when the
// power is removed (Light Sensitive mode). One relay per door switches 5 V.
static const uint8_t NUM_DOORS = LOFT_NUM_DOORS;
static const uint8_t DOOR_RELAY_PINS[8] = {2, 3, 4, 5, 6, 7, 8, 9}; // -> relay IN1..IN8
static_assert(LOFT_NUM_DOORS >= 1 && LOFT_NUM_DOORS <= 8, "LOFT_NUM_DOORS must be 1 to 8");

// The ELEGOO 8-channel relay module is active-LOW: LOW = relay ON.
static const bool RELAY_ACTIVE_LOW = true;

// Each door can draw up to 1 A. Turn them on one at a time, this far apart,
// so the 5 V / 5 A UPS doesn't see a 5 A surge all at once.
static const uint32_t DOOR_STAGGER_MS = 5000;

// ---- Time keeping -----------------------------------------------------------
// The UNO R4's internal RTC drifts, so re-sync from the internet regularly.
static const uint32_t NTP_RESYNC_INTERVAL_S = 6UL * 3600UL;  // every 6 hours
static const uint32_t NTP_RETRY_INTERVAL_S  = 5UL * 60UL;    // after a failed try

// Any clock reading earlier than this is treated as "time not set".
static const uint32_t MIN_VALID_EPOCH = 1767225600UL; // 2026-01-01 00:00:00 UTC
// "settime" refuses anything from this date on, to catch a mistyped number.
static const uint32_t MAX_VALID_EPOCH = 4102444800UL; // 2100-01-01 00:00:00 UTC

// ---- Manual overrides -------------------------------------------------------
// An "open"/"close" typed while the clock isn't set can't end at the next
// sunrise or sunset, so it ends after this long instead.
static const uint32_t OVERRIDE_NO_CLOCK_MAX_MS = 60UL * 60UL * 1000UL;  // 1 hour
