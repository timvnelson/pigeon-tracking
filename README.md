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

### Working with an AI coding agent

The most useful kind of agent here is a **command-line (terminal) coding
agent running on the computer the Arduino is plugged into**. With the free
[Arduino CLI](https://arduino.github.io/arduino-cli/) installed, the agent can:

- compile the firmware for the UNO R4 WiFi
- upload it to the board over USB
- read the board's serial messages and type test commands such as `status`

This makes testing much faster. But the agent only knows what the board
*reports*; you still watch what the hardware actually *does* (LEDs, relays,
doors). A chatbot in a web browser can suggest code, but it can't compile it
or reach your board.

To set up, install the Arduino CLI, then the board package:

```
arduino-cli core install arduino:renesas_uno
```

Most agents read [AGENTS.md](AGENTS.md) automatically; if yours doesn't, ask
it to read that file first.

### Automatic compile check

Every change to a pull request, and every merge into `main`, is compiled on
GitHub automatically, and the pull request shows a ✓ or ✗. The check:

- builds from the public settings template, so the code is known to work for
  a new owner, not just on one person's computer
- builds both with and without Wi-Fi
- fails on any error or compiler warning

It's a sanity check that the code *builds*. It can't test the hardware, so a
person still tests every change on a real board before merging.

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
