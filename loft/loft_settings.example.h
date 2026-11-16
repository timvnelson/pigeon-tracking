// =============================================================================
//  PER-LOFT SETTINGS: everything unique to one installation.
//
//  1. Copy this file to "loft_settings.h" in the same folder.
//  2. Fill in your own details below.
//  3. Delete the #error line.
//
//  loft_settings.h is listed in .gitignore, so your location, name and
//  passwords never reach GitHub. Never put these details anywhere else in
//  the code.
// =============================================================================
#pragma once

#error "Fill in your loft's details in loft_settings.h, then delete this line."

// ---- Identity (shown on the serial monitor; later used for Home Assistant) --
#define LOFT_NAME   "My Loft"
#define LOFT_OWNER  "Your Name"

// ---- Location ---------------------------------------------------------------
// Decimal degrees: north and east are positive, south and west negative.
// Two decimal places is plenty for sunrise/sunset (accurate to about 1 minute).
#define LOFT_LATITUDE   0.00
#define LOFT_LONGITUDE  0.00

// ---- Time zone (used only for printing times; scheduling runs in UTC) -------
// Offset of your STANDARD (winter) time from UTC, in minutes.
//   US Pacific -8*60, Mountain -7*60, Central -6*60, Eastern -5*60
//   UK 0, Central Europe 1*60
#define LOFT_TZ_STANDARD_OFFSET_MIN  (-8 * 60)
// Daylight saving: DST_RULE_NONE, DST_RULE_US or DST_RULE_EU
#define LOFT_DST_RULE  DST_RULE_US

// ---- Wi-Fi (for the clock now, and the home server later) ------------------
// Without Wi-Fi the doors still work, but you must set the clock over USB
// with "settime <epoch>" after every power loss. To run without Wi-Fi,
// comment out these two lines.
#define LOFT_WIFI_SSID      "your-network-name"
#define LOFT_WIFI_PASSWORD  "your-wifi-password"

// ---- Doors ------------------------------------------------------------------
// How many doors are wired to relays IN1, IN2, ... (1 to 8).
#define LOFT_NUM_DOORS  5

// ---- Optional: change the defaults (remove the // to use) ------------------
// #define LOFT_OPEN_OFFSET_MIN   0    // minutes after sunrise to open
// #define LOFT_CLOSE_OFFSET_MIN  30   // minutes after sunset to close
