# Over-the-air (OTA) updates: research notes

**Status:** tested once on a bare UNO R4 WiFi (2026-11-17). It's not in the loft firmware yet; see "Planned" below.

## How OTA works on the UNO R4 WiFi

The board package ("Arduino UNO R4 Boards") includes an `OTAUpdate` library. It is a **download**, not a push: the running sketch asks the Wi-Fi chip to download an `.ota` file from a web address. It then checks the file and flashes it, and the board restarts into the new code.

```cpp
OTAUpdate ota;
ota.begin("/update.bin");
// ota.setCACert(root_ca);         // only for https:// addresses
ota.download(url, "/update.bin");  // returns size in bytes, or < 0 on error
ota.verify();                      // checks the file's CRC
ota.update("/update.bin");         // flashes and restarts
```

- It needs Wi-Fi firmware 0.3.0 or newer. The tested board had 0.6.0.
- A plain `http://` address on the local network works, with no certificate needed.
- The Wi-Fi chip prints two `E (...) uart: uart_get_buffered_data_len ... uart driver error` lines while flashing. They're harmless: the update still completed.
- The sketch that gets installed must contain this code too, or the next update has to be done over USB.

## The .ota file

The Arduino tools don't produce `.ota` files, so `make_ota.py` builds them from the `.bin` that `arduino-cli compile --output-dir` writes. The format was worked out from Arduino's own update file (`https://downloads.arduino.cc/ota/UNOR4WIFI_Animation.ota`): `make_ota.py check` passes on that file and unpacks it to the size the board reported.

| Bytes | Content (little-endian) |
|---|---|
| 0-3 | Length of everything after byte 7 |
| 4-7 | CRC-32 (zlib) of everything after byte 7 |
| 8-11 | Board ID `0x23411002` (USB VID 0x2341, PID 0x1002 = UNO R4 WiFi) |
| 12-19 | Version: all zero except byte 19 = `0x40` ("compressed") |
| 20- | The `.bin`, LZSS-compressed (11-bit window, 4-bit length, window starts filled with spaces) |

The loft firmware (86 KB) compresses to about 70 KB, and the conversion takes about a second on a Raspberry Pi.

## Repeating the test

1. Build the firmware and convert it:
   ```
   arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --output-dir build loft
   python3 tools/ota/make_ota.py build/loft.ino.bin serve/loft.ota
   ```
2. Serve it on the local network from the computer (use its own IP address):
   ```
   python3 -m http.server 8000 --bind <computer-ip> --directory serve
   ```
3. Open the board package's example (Arduino IDE: File > Examples > OTAUpdate > OTA). Set your Wi-Fi details. Change the address to `http://<computer-ip>:8000/loft.ota` and remove the `setCACert` step. Upload it over USB.
4. Open the Serial Monitor. Expected: `downloaded bytes: 86020` (or similar), a count while flashing, then the loft firmware's `[boot]` line and its normal status.
5. Stop the web server.

## Planned

OTA is planned to come after MQTT (see AGENTS.md):
- **Trigger from Home Assistant:** an "Update firmware" button, or Home Assistant's standard update entity showing "update available". The owner chooses when to install, e.g. at night with the doors closed.
- **USB `update` command** as a backup, and for testing the download-and-flash step before MQTT exists.
- **The update address lives in `loft_settings.h`** (per-loft data). The board only downloads from that address.
- **No automatic installs:** a timer may *check* for a newer version and report it, but never installs on its own.
- The firmware never updates while doors are moving. After the restart, boot safety turns the doors off first, then the schedule reopens them if it's daytime.
