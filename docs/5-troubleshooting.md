# Troubleshooting

Common problems and how to fix them.

## Upload fails with "Permission denied" (Linux)

```
Error: ... /dev/ttyACM0: Permission denied
```

Your user isn't allowed to use serial ports. Add yourself to the `dialout` group, then **log out and back in** (or reboot) so it takes effect:

```
sudo usermod -aG dialout $USER
```

Check it worked: `id` should list `dialout`, and `ls -l /dev/ttyACM0` shows the port belongs to group `dialout`.

## The clock never sets ("NTP: no time received")

The loft sets its clock from the internet at boot. If `status` keeps showing `Time: NOT SET`, the doors stay closed (that's the safe default).

- `[time] Wi-Fi connect failed`: check the Wi-Fi name and password in `loft_settings.h`. The UNO R4 WiFi only joins **2.4 GHz** networks.
- `[time] NTP: DNS lookup failed`: the board joined Wi-Fi but can't look up `pool.ntp.org`. Check that the network gives out a DNS server and has internet access.
- `[time] NTP: no time received`: the network may be blocking outgoing UDP port 123 (NTP).

Until it's fixed you can set the clock by hand over USB: get the current UTC time with `date +%s` on Linux/macOS, then type `settime <that number>` in the Serial Monitor.

Note: the firmware asks the NTP server directly. It doesn't use the board's built-in `WiFi.getTime()`, which always returned 0 on a real UNO R4 WiFi (Wi-Fi firmware 0.6.0) even when NTP worked.
