# Bird Loft & Release Cage Automation Project  
# AI System Prompt & Architecture: Bird Loft & Release Cage Automation  
  
### Non-Technical Summary  
This project builds an automated, smart management system for homing pigeons/birds made of two completely separate kinds of system: a stationary home loft, and a fleet of mobile release cages (10+). Each cage is its own independent system.  
  
At the home loft, an intelligent controller automatically secures entrance doors at night, tracks individual birds returning via leg bands, regulates feeding schedules by opening and closing feeder covers, and watches for water spills. Out in the field, each release cage monitors surrounding weather conditions, tracks its own location via GPS only when it senses it is being moved, and constantly verifies which birds are still inside using a floor grid of scanners.  
  
The loft and every release cage each have their own battery unit: the loft's covers power outages, and each cage's lets it run away from home. They communicate wirelessly back to a central home server, which collects all the data and displays an easy-to-read dashboard on your phone or computer.  

---

## 0. Confirmed Hardware Facts (verified by owner; these override anything below)  
Only the project owner adds facts here, after checking the real hardware. If another section disagrees with this one, this one is correct and the other section should be fixed.  

### Entrance doors: Ripom D3786 solar roll-up coop door (Amazon ASIN B0H41MYJFB)  
- **Self-contained unit.** Each door has its own motor, controller, LCD and internal 2500 mAh battery. The Arduino does **not** drive the door motors.  
- **Charging input:** 4.2-5.0 V DC at 1 A (from the door's manual). It was originally fed 5 V by the included solar panel. Rats kept chewing the panel cable, so the panels are being replaced by wired DC from the UPS.  
- **The power input is also the open/close signal.** In Light Sensitive mode the door uses that input as its light sensor: **power present = day = door opens; power removed = night = door closes.** So one relay per door does both jobs:  
    - **Relay ON:** the door charges and opens (daytime).  
    - **Relay OFF:** the door runs on its battery and closes (night).  
    - Each door must stay in **Light Sensitive mode** on its LCD, or this control method will not work.  
    - The Arduino replaces the sun: it switches each door's relay at sunrise and sunset, which it calculates offline.  
- **Power budget:** up to 1 A per door. All 5 doors powered during the day can draw up to 5 A, which is the UPS's whole 5 V 5 A rating before counting the Arduino and relay coils. Turn the doors on one at a time a few seconds apart rather than all at once. If the voltage sags, consider a separate supply for the doors.  
- **Boot and reset safety:** the ELEGOO relay inputs are active-LOW. Firmware must drive each relay pin HIGH (relay off) *before* setting it as OUTPUT. Otherwise a reset or power blip at night can briefly power the doors and make them start opening.  
- **Other built-in controls:** an RF remote (about 165 ft range) and manual push-buttons on the unit still work as manual overrides.  
- **Anti-pinch:** if the door meets resistance while closing, it retracts for 10 s and then tries again. Firmware must not treat a door that is still open shortly after power is removed as a fault until that retry window has passed.  
- **Opening:** 9.5" x 10". Operating range: -15°F to 140°F.  
- **Response time:** the door starts moving a couple of seconds after its power is switched on or off (owner confirmed).  
- **Power connector:** a 5.5 x 2.1 mm DC barrel jack, where the solar panel originally plugged in (owner confirmed). The MILAPEAK 5.5 x 2.1 mm pigtails fit it.  
- **Wire runs:** the reference loft's longest run from the UPS to a door is about 35 ft of 18 AWG. That loses about 0.45 V at 1 A, so the door still gets about 4.5 V. See the run-length table in `docs/1-parts-list.md`.  
- **Still unknown (do not guess, ask the owner):**  
    - How much current each door actually draws when its battery is full compared with when it is charging.  

### Pin assignments (project standard wiring; D2-D6 confirmed by owner)  
| Arduino pin | Connects to | Purpose |  
|---|---|---|  
| D2 | Relay IN1 | Door 1 power |  
| D3 | Relay IN2 | Door 2 power |  
| D4 | Relay IN3 | Door 3 power |  
| D5 | Relay IN4 | Door 4 power |  
| D6 | Relay IN5 | Door 5 power |  
| D7-D9 | Relay IN6-IN8 | Doors 6-8, used only if a loft sets `LOFT_NUM_DOORS` above 5 |  

### Per-loft data rule (owner's rule; always applies)  
This project is meant to be reused: anyone can buy the suggested hardware and install it in their own loft. So **nothing unique to one loft is ever committed**. That covers location, owner name, Wi-Fi details, passwords and server addresses, and any other value that differs between installations.  
- All such values live in `loft/loft_settings.h`, which git ignores. Each installer makes theirs by copying `loft/loft_settings.example.h`.  
- The committed code holds only project defaults (pins, timings). It reads per-loft values through `config.h`, which stops the build with a clear message if the settings file is missing or wasn't edited.  
- New per-loft values (e.g. the MQTT broker address and login) go in `loft_settings.example.h` as placeholders, never real values. If a value is optional, `config.h` gives it a default.  
- Docs, code comments and commit messages describe the hardware generically and never name an owner's location or personal systems.  
  
  
## 1. How to work on this project (for AI agents and humans)  
Any AI agent working on this repo should follow these rules, so every owner's agent behaves the same way.  

**This file is the project's memory.** It lives in the repo so every agent and every person gets the same picture. Read it fully before doing anything. Section 0 (Confirmed Hardware Facts) overrides everything else; if the code or another section disagrees with it, fix them to match. Never guess about hardware: if something is listed as unknown, ask the owner.  

**Keep project knowledge in the project.** When you learn or decide something about this project, write it in this file or in `docs/`. That covers a hardware fact, a design decision, a convention or a lesson from testing. Then every contributor's agent works from the same information and behaves the same way. How you set up your own tools stays your business.  

**Branches and merging**  
- Never commit directly to `main`, not even docs. All work goes on a branch, and `main` changes only when the owner merges a pull request.  
- A branch covers whatever the owner calls the current topic. That can be one feature (`feat/loft-mqtt`) or broad (a refinement pass across the project). Don't split or merge branches unless the owner asks.  
- **One person per branch.** Each branch belongs to one person, who works on it with their own AI agent if they use one. Only that person, or their agent, pushes to it. This keeps things simple: whatever that person or their agent knows but hasn't committed yet can't clash with someone else's changes.  
    - The branch's person is the **assignee** on its pull request. Set yourself as assignee when you open a PR for your branch.  
    - To change someone else's branch, tell that person, or make your own branch and open a PR. Never push to another person's branch, not even small fixes or merges of `main`; the branch's person brings `main` in when they're ready.  
    - Push often, and keep the PR description current, so others can see where your branch stands without needing your private notes.  
- Only the owner can verify that the code works, by testing it on the real hardware. Push the branch, open a pull request, then **wait** for the owner to test and merge. Don't merge unless the owner explicitly asks you to.  
- Every merge into `main` must carry a description saying what the system now does. Keep the PR title and description up to date as the branch grows, so they can serve as the merge description.  
- **AI agents are welcome; humans verify.** Contributors may use any AI agent. But every change to hardware behaviour or data collection (door control, RFID tracking, sensor readings, battery status, anything reported to Home Assistant) must be tested by a person on real hardware before it's merged. Collected data must also be checked by a person against what really happened. Never claim something works on hardware just because it compiles; say what a person still needs to test.  
- **Attribution (owner's rule):** credit the AI honestly, without implying no human was involved.  
    - **Always name the specific AI tool and model, for transparency** (e.g. "Claude Code, Claude Opus 5.5"). Never leave it out or make it vague: the project wants an honest record of which AI agents built it.  
    - Commits: the human contributor is the author, and the AI is credited with a `Co-Authored-By:` line naming that tool or model.  
    - PR descriptions end with: *Code written with <AI tool and model>; reviewed and merged by the project owner.* If no AI was used, say so.  
    - Add *Hardware-tested by <the person who tested it>.* only once a person has actually tested that PR on the hardware.  
- Merged branches are deleted automatically; start new work from the latest `main`.  

**Keeping this file current**  
- Update this file **on the same branch** as the change it describes, so it reaches `main` with the merge and always describes what `main` does.  
- After a merge, pull `main`. If anything changed during testing and this file no longer matches, fix it on the next branch.  

**Before pushing code**  
- Compile it: `arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --warnings all loft` (with the "Arduino UNO R4 Boards" core installed: `arduino-cli core install arduino:renesas_uno`). Fix all warnings.  
- Compiling proves only that it builds. Describe what still needs testing on the hardware.  
- **GitHub compile check** (`.github/workflows/compile.yml`): every change to a PR, and every merge into `main`, is compiled on GitHub from the public `loft_settings.example.h`, both with and without Wi-Fi, and any error or warning in `loft/` fails it. It must show ✓ before the owner merges. If you change what the settings template must contain, keep the workflow working with it.  

**Working with a board on USB** (only if your agent runs on the computer the Arduino is plugged into)  
- Find the port: `arduino-cli board list` (usually `/dev/ttyACM0` on Linux, `/dev/cu.usbmodem…` on macOS, `COMx` on Windows).  
- Upload: `arduino-cli upload -p <port> --fqbn arduino:renesas_uno:unor4wifi loft`. **Tell the owner before uploading if real doors are connected**: an upload resets the board. Boot safety keeps the doors closed during a reset, but the owner should know.  
- Serial: `arduino-cli monitor -p <port> --config 115200`. The monitor runs until stopped, so run it with a timeout or in the background, and capture its output.  
- Serial output shows what the firmware *reports*, not what the hardware *does*. The owner confirms physical behaviour (relays clicking, LEDs, doors) before anything counts as working.  

**Privacy and reuse**  
- Follow the Per-loft data rule (Section 0). Never commit location, owner names, passwords or anything else unique to one loft. Never print or commit `loft_settings.h`.  
- Don't mention an owner's personal systems (home network, smart-home or camera brands) in the repo. Describe integrations generically.  
- When an owner shares a product link, strip tracking and account parameters and keep only `amazon.com/dp/<code>`.  

**Docs**  
- `docs/` is the public setup guide. Add to it slowly, with only what the owner finds useful, and keep `docs/README.md` current.  
- Write for a hobbyist building their own loft: plain language, short, with real values and expected outputs.  

**Language:** C/C++ (Arduino framework) for speed, bare-metal efficiency and battery life (race-to-sleep).  

---
  
## 2. Bird Loft Arduino Project  
- **Microcontroller:** Arduino UNO R4 WiFi (32-bit Renesas RA4M1 + ESP32-S3 coprocessor, 5V GPIO logic, built-in RTC with VRTC coin-cell backup pin, USB-C, Qwiic/STEMMA QT connector).  
    - **Power:** in the loft, the board runs from the 5 V distribution block through a USB-C male-to-pigtail cable. Its barrel jack and VIN pin need 6-24 V, so they can't run from the UPS's 5 V.  
- **Entrance Doors:** 5 Ripom D3786 self-powered roll-up doors (see Section 0). One channel of the 8-channel 5V relay module per door switches the UPS 5V supply to that door's power input. Power on opens the door and charges it; power off closes it. The Arduino turns the relays on at sunrise and off at sunset using offline astronomical calculations (our own NOAA implementation in `loft/SolarSchedule.cpp`; no libraries).  
- **RFID Tracking:** Multiple RC522 RFID readers (13.56 MHz, SPI communication, 3.3V VCC, 2-5 cm range) arranged in an array across a 3D-printed waterproof strip. Uses individual Slave Select (SS) pins to identify exact door/sensor positions.  
    - **Electrical notes (not yet confirmed on hardware; move to Section 0 once the RFID tester has checked them):**  
        - **Power:** the RC522 runs on 3.3 V. The UNO R4's 3.3 V pin can power one or two readers for bench testing. A full array of readers transmitting may draw more than that pin is meant to supply, so plan a separate 3.3 V regulator for the loft array and check the R4 datasheet for the pin's current limit.  
        - **Signals need a level shifter.** The R4's 3.3 V pin is only a power output: the R4 still drives its signal pins at 5 V and has no 3.3 V logic mode. The RC522's chip is rated for 3.3 V signals, so 5 V on SCK, MOSI, SS/SDA and RST is out of spec. It often seems to work, but it can damage the reader over time or make it unreliable.  
        - In the other direction, the reader's 3.3 V MISO output may be close to the lowest voltage a 5 V R4 reliably reads as high.  
        - Use a two-way level shifter (BSS138- or TXS0108E-based "3.3 V / 5 V logic level converter") on all the SPI lines, one channel per signal.  
        - **Alternative to evaluate:** some other 13.56 MHz reader boards (e.g. many PN532 breakouts) accept 5 V logic directly, so they'd need no level shifter.  
- **Feeder Control:** Stepper motors controlled via driver modules (e.g., A4988/DRV8825) to regulate feeding access.  
- **Water Leak Sensors:** Integrated to detect overflow.  
- **Power System:**  
    - LAFVIN UPS Module 3S (3x 18650 batteries, 12.6V 2A input charger). Used by the loft only. It ships with its own charger power supply; the three 18650 cells are bought separately.  
    - Outputs 5V 5A continuous via the main Type-C/XH2.54 port.  
    - **Battery reporting (planned feature):** the UPS has an I2C interface that reports battery voltage, current and power. The loft Arduino will read it and publish the battery status to Home Assistant: on mains or on battery, charge level, and a warning when the battery is low. The I2C chip, its address and its logic voltage are still to be confirmed on the real board (the Qwiic connector is 3.3 V I2C; the UNO header pins are 5 V).  
    - Wiring: 18 AWG stranded pure copper wire. Keep it out of reach of rats (they chewed the old solar-panel cables). Ground runs to a distribution block; Positive runs to a distribution block, then through relay COM/NO terminals and a 1 A inline fuse per door (owner's choice, as a precaution: the doors' internal charging circuit hasn't been checked) out to each door's power input (4.2-5.0 V, 1 A). A blown fuse makes that door close.  
- **Connectivity & Home Server:** MQTT protocol over Wi-Fi (or future Ethernet shield like W5500) back to the home server (Home Assistant with the Mosquitto MQTT broker; see Section 4).  

---
  
## 3. Release Cage Arduino Project (separate system; work starts later)  
The release cages are **completely separate** from the loft: separate hardware, separate firmware (its own folder, e.g. `cage/`), and nothing shared. There are **10+ cages**, and each one has its own UPS, Arduino, environmental sensors, GPS and RFID. Every cage reports to Home Assistant on its own, so the firmware must give each cage a unique identity (from its per-cage settings file).  
- **Power System:** each cage has its own UPS and batteries. **The battery must last at least 30 hours per charge**, so that one overnight recharge always leaves a cage ready to go. Every hardware and firmware choice for the cage must be checked against this power budget.  
- **Environmental Sensors:** BME280 (Temperature, Humidity, Barometric Pressure/Altimeter) and BH1750 (Sun brightness/Lux). Both share I2C lines (SDA/SCL) with ultra-low power draw.  
- **Motion & GPS:** MPU6050 or ADXL345 accelerometer wired to a hardware interrupt pin. Wakes up the NEO-6M GPS module only when physical movement is detected to conserve battery.  
- **Connectivity:** Cellular module (e.g., SIM800L/SIM7600) when away from home (kept in continuous idle/network standby during daytime 1-minute updates, deep sleep PSM at night); switches to Wi-Fi when at home.  
- **RFID Verification:** Array of RC522 modules spanning the 32" x 17" bottom of the release box to verify bird presence.  
- **Offline Storage:** MicroSD card module logs data during connectivity drops; clears data only after receiving an HTTP 200 OK / MQTT confirmation handshake.  

---
  
## Firmware: what the code does today  
Update this section on every feature branch so that once merged it describes what `main` does.  

### Loft (`loft/`, Arduino UNO R4 WiFi)  
**Door schedule.** At sunrise the relays power the doors, which open. At 30 minutes after sunset (configurable per loft) they cut the power, and the doors close.  
- `loft.ino`: the main loop, the USB serial test commands, the status printout and the local-time display (US or EU daylight-saving rules, or none).  
- `loft_settings.example.h`: template for the per-loft settings file (see "Per-loft data rule"). Covers the loft name, owner, location, time zone, Wi-Fi, door count (1-8) and optional open/close offsets.  
- `config.h`: loads `loft_settings.h`, checks it, applies defaults, and holds the project-wide settings (pins, stagger delay, NTP intervals).  
- `SolarSchedule.*`: offline NOAA sunrise/sunset maths (no libraries) and the stateless "should the doors be open now?" check. It works out the right state from scratch after any reboot.  
- `DoorController.*`: relay switching. Pins are set to OFF first at boot. Doors open one at a time, 5 s apart; they close all at once.  
- `TimeKeeper.*`: UTC clock in the R4's built-in RTC, synced from NTP over Wi-Fi at boot, then every hour (or every 5 min after a failed attempt). The R4's RTC has no crystal: on the test board it ran 2.6% fast (about 37 min a day), so the hourly sync keeps it within about 1.5 min. Without Wi-Fi it drifts that fast; a DS3231 clock module is the hardware fix if needed. It sends its own NTP request over UDP (`WiFiUDP`, part of the core) to `pool.ntp.org`, because `WiFi.getTime()` always returned 0 on the real board (Wi-Fi firmware 0.6.0). DNS is retried for up to 10 s, because it failed for a moment right after Wi-Fi connected at boot.  

**Docs:** `docs/` is the public setup guide, starting at `docs/README.md` with numbered pages; `docs/wiring/` holds diagrams and `tools/` holds hardware-test sketches. Most pages are placeholders. Fill them slowly, with only what the owner finds useful, and keep the index in `docs/README.md` current. So far `docs/1-parts-list.md`, `docs/3-bench-test.md` (relays tested with LEDs) and `docs/5-troubleshooting.md` are written. The parts list has a Status column (Arriving → Received → Bench-tested → Installed); update it as the owner reports progress.  

**Wi-Fi today:** the board connects at boot to fetch the time and stays connected. A dropped connection is only noticed at the next time sync (up to 1 h). That's fine for the doors, which run from the RTC and never need Wi-Fi.  
**Before the MQTT feature (planned `feat/loft-mqtt`):** add a Wi-Fi watchdog that checks the connection every ~10 s and reconnects with back-off without stalling the loop (`WiFi.begin()` blocks for ~10 s, long enough to miss an RFID read). Use `ArduinoMqttClient` with a keep-alive and Last Will so Home Assistant notices when the loft goes offline, and queue events while offline.  

**Over-the-air updates (researched and tested once on a bare board 2026-11-17; planned after MQTT):** the core's `OTAUpdate` library downloads an `.ota` file over Wi-Fi and flashes it. `tools/ota/make_ota.py` builds the file, and `tools/ota/README.md` has the format, the test steps and the plan. The plan: a Home Assistant trigger (not automatic installs), a USB `update` command as a backup, and the update address in `loft_settings.h`. Every firmware version must keep the update code.  

**Rules the code follows:**  
- All scheduling happens in UTC, so daylight saving cannot break it. Local time is used only for printing.  
- If the clock has never been set, the doors stay **closed**.  
- USB serial commands (115200 baud): `status`, `open`, `close`, `auto`, `settime <utc-epoch>`, `sync`. An `open`/`close` override lasts until the next scheduled sunrise or sunset change, so it can't be left on by accident.  
- No third-party libraries. It needs only the "Arduino UNO R4 Boards" core, which includes `RTC.h` and `WiFiS3.h`.  

---

## 4. Home Server  
- **Platform (decided): Home Assistant**, chosen because it integrates with a wide range of other systems and devices. The loft and cage data can sit alongside other smart-home equipment, cameras and notifications, and automations can link them.  
- **Broker:** the Mosquitto MQTT broker, run as a Home Assistant add-on.  
- **Device setup: MQTT Discovery.** The Arduinos publish Home Assistant discovery messages so their doors and sensors appear as entities automatically, with no manual YAML.  
- **Hosting:** Home Assistant should run on its own dedicated machine, booting from an SSD rather than an SD card, because its database writes wear SD cards out.  
- **Optional later:** Node-RED, as a Home Assistant add-on, if visual flow-based automations are wanted.  
- **Data Storage:** Centralizes and stores all sensor telemetry, bird tracking logs, and system states from the loft and every release cage.  
  
  
### Parts
The loft parts list, with links and notes, is in [`docs/1-parts-list.md`](docs/1-parts-list.md).
