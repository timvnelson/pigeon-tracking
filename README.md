# pigeon-tracking

Arduino automation for a homing-pigeon loft. Loft doors open at sunrise and
close after sunset, and more features are planned: RFID bird tracking, feeders,
leak sensors, separate mobile release cages, and a Home Assistant dashboard. See
[AGENTS.md](AGENTS.md) for the full design and the hardware it uses.

## How this project is built

Contributors are encouraged to use AI coding agents (Claude Code, or any
other) to write code and docs. Every agent follows the same rules, set out in
[AGENTS.md](AGENTS.md). For transparency, every commit and pull request names
the AI tool and model that helped write it, so the history shows which agents
built the project.

**A human is always in charge of the results:**
- People decide what gets built.
- A person tests every change on real hardware before it's merged.
- Any data the system collects (bird tracking, sensor readings, battery
  status) is checked by a person against what really happened before anyone
  relies on it.

AI writes the code; humans verify that it works in the loft.

## Setting up your own loft

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) and, in the
   Boards Manager, the **Arduino UNO R4 Boards** package. No other libraries
   are needed.
2. In the `loft/` folder, copy `loft_settings.example.h` to `loft_settings.h`.
3. Edit `loft_settings.h` with your loft's name, location, time zone, Wi-Fi
   and number of doors, then delete its `#error` line. This file is
   git-ignored, so your details stay on your computer.
4. Wire the relay module as listed in AGENTS.md (doors on D2, D3, ...), open
   `loft/loft.ino`, select **Arduino UNO R4 WiFi**, and upload.
5. Open the Serial Monitor at 115200 baud and type `status` to check the time
   and the next open/close time. Type `help` for the test commands.
