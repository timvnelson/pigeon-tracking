#include "DoorController.h"
#include "config.h"
#include <Arduino.h>

static_assert(NUM_DOORS <= 8, "relay module has 8 channels");

// The UNO R4 core's digitalWrite() takes a PinStatus (LOW/HIGH), not an int.
static inline PinStatus levelFor(bool on) {
  return (on == RELAY_ACTIVE_LOW) ? LOW : HIGH;
}

void DoorController::begin() {
  // Boot safety: the relay module is active-LOW, and a pin that briefly reads
  // LOW at start-up would power a door at night and start opening it. Until
  // this runs the pins are inputs (floating), and the module treats a floating
  // input as OFF.
  //
  // On the UNO R4 core, pinMode(OUTPUT) rewrites the pin's whole config
  // register with the output level cleared, so it drives LOW (relay ON) until
  // the next digitalWrite, even if HIGH was written first. Instead, make the
  // pin an output and set its OFF level in one register write.
  const uint32_t offLevelCfg =
      (levelFor(false) == HIGH) ? (uint32_t)IOPORT_CFG_PORT_OUTPUT_HIGH : 0UL;
  for (uint8_t i = 0; i < NUM_DOORS; i++) {
    const uint8_t pin = DOOR_RELAY_PINS[i];
    R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin,
                    (uint32_t)IOPORT_CFG_PORT_DIRECTION_OUTPUT | offLevelCfg);
    digitalWrite(pin, levelFor(false));  // belt and braces
    powered_[i] = false;
  }
  target_ = false;
}

void DoorController::setOpen(bool open) { target_ = open; }

void DoorController::writeRelay(uint8_t door, bool on) {
  digitalWrite(DOOR_RELAY_PINS[door], levelFor(on));
  powered_[door] = on;
  Serial.print(F("[door] door "));
  Serial.print(door + 1);
  Serial.println(on ? F(" power ON (opening)") : F(" power OFF (closing)"));
}

void DoorController::update() {
  if (!target_) {
    for (uint8_t i = 0; i < NUM_DOORS; i++) {
      if (powered_[i]) writeRelay(i, false);
    }
    anyTurnedOn_ = false;
    return;
  }
  // Opening: power the next unpowered door once the stagger delay has passed.
  const uint32_t nowMs = millis();
  if (anyTurnedOn_ && (nowMs - lastOnMs_) < DOOR_STAGGER_MS) return;
  for (uint8_t i = 0; i < NUM_DOORS; i++) {
    if (!powered_[i]) {
      writeRelay(i, true);
      lastOnMs_ = nowMs;
      anyTurnedOn_ = true;
      return;
    }
  }
}

bool DoorController::isPowered(uint8_t door) const {
  return door < NUM_DOORS && powered_[door];
}

uint8_t DoorController::poweredCount() const {
  uint8_t n = 0;
  for (uint8_t i = 0; i < NUM_DOORS; i++) n += powered_[i];
  return n;
}
