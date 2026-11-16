// Switches the 5 V power to each door through its relay.
// Power ON = door charges and opens. Power OFF = door closes on its battery.
#pragma once
#include <stdint.h>

class DoorController {
 public:
  // Puts every relay in the OFF (door closed) state as safely as possible.
  void begin();

  // Sets what the doors should be doing. Opening is staggered (one door every
  // DOOR_STAGGER_MS) to avoid a current surge; closing happens immediately.
  void setOpen(bool open);
  bool targetOpen() const { return target_; }

  // Call every loop iteration.
  void update();

  bool isPowered(uint8_t door) const;
  uint8_t poweredCount() const;

 private:
  void writeRelay(uint8_t door, bool on);

  bool target_ = false;
  bool powered_[8] = {false};
  uint32_t lastOnMs_ = 0;
  bool anyTurnedOn_ = false;
};
