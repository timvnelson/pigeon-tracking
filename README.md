# pigeon-tracking

Arduino automation for a homing-pigeon loft. Loft doors open at sunrise and
close after sunset, and more features are planned: RFID bird tracking, feeders,
leak sensors, separate mobile release cages, and a Home Assistant dashboard. See
[AGENTS.md](AGENTS.md) for the full design and the hardware it uses.

## How this project is built

The project owner, a pigeon keeper, designs the system and decides what gets
built. The owner also tests every change on the real hardware in the loft
before merging it. The code and docs are written with
[Claude Code](https://claude.com/claude-code), an AI coding assistant, under
the owner's direction. That's why commits list Claude as a co-author.

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
