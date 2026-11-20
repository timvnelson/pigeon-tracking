# Bench test: relays with LEDs instead of doors

Before installing anything in the loft, check the Arduino and relay module on
a breadboard. An LED and a resistor on each relay's output act as a fake door:

**LED on = door powered = open. LED off = closed.**

No transistors are needed; the relay is already the switch.

## Wiring each relay output (doors 1-5)

```
5 V ──► relay COM
        relay NO ──► 330 Ω resistor ──► LED long leg (+)
                                          LED short leg (−) ──► GND
```

- **Use NO (normally open), not NC.** NO is disconnected when the relay is
  off, so a dead or unpowered relay means a closed door. That's the safe
  direction, and the real doors should be wired the same way.
- **Always include the resistor.** An LED on 5 V without one burns out
  instantly. Anything from 220 Ω to 1 kΩ works; 330 Ω gives about 9 mA.

## Wiring the relay module

- IN1-IN5 go to Arduino pins D2-D6.
- Module GND goes to Arduino GND.
- Power the module's VCC from the UPS 5 V output, not the Arduino's 5 V pin,
  and connect all the grounds together. Each relay coil draws about 70-80 mA,
  so five at once (~0.4 A) is too much to pull through the Arduino from USB.
- Leave the module's JD-VCC jumper in place (it joins VCC and JD-VCC in the
  power corner). Then the module's single VCC powers both the input side and
  the relay coils.

| Relay module pin | Connect to |
|---|---|
| Power corner **VCC** | UPS 5 V (+) |
| Power corner **GND** | UPS ground (−) |
| Input header **IN1-IN5** | Arduino D2-D6 |
| Input header **GND** | Arduino GND |
| Input header **VCC** | **Nothing** |

- **Leave the input header's VCC empty.** With the jumper on, it's already
  joined to the UPS 5 V. Wiring the Arduino's 5 V to it would join two power
  supplies, and with the board on a computer's USB that can push current back
  into the computer.
- **The Arduino's GND must connect to the module's GND.** The relay inputs
  only work when the Arduino and the UPS share a ground.

## What to check

Open the Serial Monitor at 115200 baud.

1. **`open`:** the LEDs light one at a time, 5 seconds apart, and each relay
   clicks. The module's own red channel LEDs should match your LEDs.
2. **`close`:** all LEDs go off at once.
3. **`auto`:** the LEDs follow the real schedule (on during the day).
4. **Reset safety:** in the evening, with the doors scheduled closed, press
   the reset button or unplug and replug the Arduino. **No LED should
   flicker, even briefly.** This is the most important test.
5. **A sunrise or sunset change without waiting:**
   - Type `status` to see the next change time.
   - Use `settime <epoch>` to jump the clock to a minute before it. Get the
     number from https://www.unixtimestamp.com, entered as UTC.
   - Within a minute the LEDs should switch on their own.
   - The clock corrects itself at the next internet time sync (within
     an hour), or right away if you type `sync`.
