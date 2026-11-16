# Parts list (loft)

The hardware for the loft controller. The release cages use different hardware
and will get their own list.

**Status** shows where the reference loft's prototype is with each part:
*Arriving* → *Received* → *Bench-tested* → *Installed*.

## Main parts

| Part | Qty | Status | Notes |
|---|---|---|---|
| [Arduino UNO R4 WiFi (ABX00087)](https://www.amazon.com/dp/B0C8V88Z9D) | 1 | Arriving 2026-11-17 | The loft controller. In the loft, power it from the 5 V distribution block through its USB-C port (see the pigtail below). |
| [LAFVIN UPS Module 3S, 5 V 5 A](https://www.amazon.com/dp/B0G2SJPWLR) | 1 | Arriving 2026-11-17 | Battery backup for everything in the loft. Comes with its own charger power supply. Needs three 18650 cells (below). Can report its battery status over I2C. |
| [ELEGOO 8-channel 5 V relay module](https://www.amazon.com/dp/B09ZQRLD95) | 1 | Arriving 2026-11-17 | One relay per door. Active-LOW inputs, a status LED per channel. Contacts rated 10 A. |
| [Ripom D3786 solar roll-up coop door](https://www.amazon.com/dp/B0H41MYJFB) | 1 per door (up to 8) | Received; out of service until wired power is installed (rats chewed the solar-panel cables). Not part of the bench prototype | See "Doors" below. |

## Wiring

| Part | Status | Notes |
|---|---|---|
| [18 AWG red/black zip wire, 100 ft](https://www.amazon.com/dp/B0CYFQM79X) | Arriving 2026-11-17 | UPS to the doors. See "Wire runs" below. |
| [MILAPEAK 5.5 x 2.1 mm DC barrel pigtails (18 AWG)](https://www.amazon.com/dp/B072BXB2Y8) | Arriving 2026-11-17 | Plug into each door's power jack. |
| [APIELE 15 A screw terminal blocks](https://www.amazon.com/dp/B0D7VRSH1G) | Arriving 2026-11-17 | Distribution blocks for 5 V and ground. |
| [Lever wire connectors, 2-pin](https://www.amazon.com/dp/B0H9YN2RV3) | Arriving 2026-11-17 | Quick splices in 18 AWG wire. |

## Bought separately

| Part | Status | Notes |
|---|---|---|
| Three 18650 cells for the UPS | Received | Buy a reputable brand (Samsung, LG, Molicel, Sony/Murata) from a reliable seller; cheap cells with inflated ratings are common and can be unsafe. |
| USB-C data cable | Received | Connects a computer to the Arduino, for uploading code and the Serial Monitor. Also powers the board during the bench test. |
| USB-C male to bare-wire pigtail | To buy | Powers the Arduino in the loft from the 5 V distribution block. Don't use the Arduino's barrel jack: it needs 6-24 V, so the UPS's 5 V won't run it. Check the pigtail's + and − with a multimeter before plugging it in. Unplug it whenever a computer is plugged in to upload code. |
| Jumper wires, Arduino to relay inputs | Received | |
| Breadboard, LEDs, 330 Ω resistors | Received | For the [bench test](3-bench-test.md). |

## Doors

The Ripom D3786 is a self-contained door with its own motor, controller and
2500 mAh battery. It's sold with a solar panel; this project replaces the panel
with wired 5 V from the UPS.

- **The power input is the open/close signal.** In **Light Sensitive mode**,
  power on means day, so the door opens; power off means night, so it closes.
  Set every door to Light Sensitive mode on its LCD.
- **Power jack:** 5.5 x 2.1 mm barrel, where the solar panel plugged in. It
  takes 4.2-5.0 V at up to 1 A.
- Each door can draw up to 1 A, so five doors can need the UPS's whole 5 A.
  The firmware switches them on one at a time.
- Keep the wire out of reach of rats: they chew through the solar-panel cable.

## Wire runs

Long runs of thin wire lose voltage, and the door needs at least 4.2 V. Below
is the voltage left at the door from a 5.0 V supply at 1 A, with 18 AWG wire
(counting both the + and − wires):

| One-way run | Voltage lost | At the door |
|---|---|---|
| 10 ft | 0.13 V | 4.87 V |
| 20 ft | 0.26 V | 4.74 V |
| 35 ft | 0.45 V | 4.55 V |
| 50 ft | 0.64 V | 4.36 V (marginal) |
| 60 ft | 0.77 V | 4.23 V (too close) |

Relay contacts and connectors lose a little more. **For runs over about 40 ft,
use 16 AWG** (about a third less loss) **or 14 AWG** (60% less loss).
