#!/usr/bin/env python3
"""Convert an Arduino UNO R4 WiFi sketch .bin into an .ota update file,
for the board's built-in OTAUpdate library. Python 3 standard library only.

Usage:
  python3 make_ota.py <sketch.bin> <out.ota>   build an update file
  python3 make_ota.py check <file.ota>         check header/CRC and unpack

.ota layout (all little-endian):
  u32 length   (bytes after this 8-byte prefix)
  u32 crc32    (zlib CRC-32 of those bytes)
  u32 magic    0x23411002 (UNO R4 WiFi USB VID 0x2341 / PID 0x1002)
  8 bytes      version; byte 7 = 0x40 means "LZSS compressed"
  LZSS data    (Okumura LZSS: 11-bit window, 4-bit length, threshold 1)
"""
import sys, zlib, struct

EI, EJ, P = 11, 4, 1
N, F = 1 << EI, (1 << EJ) + P

class BitWriter:
    def __init__(self): self.out = bytearray(); self.buf = 0; self.mask = 0x80
    def bit(self, b):
        if b: self.buf |= self.mask
        self.mask >>= 1
        if self.mask == 0: self.out.append(self.buf); self.buf = 0; self.mask = 0x80
    def bits(self, v, n):
        m = 1 << (n - 1)
        while m: self.bit(v & m); m >>= 1
    def flush(self):
        if self.mask != 0x80: self.out.append(self.buf)
        return bytes(self.out)

def encode(data):
    # Window starts filled with spaces, write position N-F (as in the reference).
    buf = bytearray(b' ' * (N - F)) + data
    w = BitWriter()
    pos, end = N - F, len(buf)
    # index of 3-byte prefixes for speed
    table = {}
    def add(i):
        if i + 2 < end: table.setdefault(bytes(buf[i:i+3]), []).append(i)
    for i in range(0, pos): add(i)
    while pos < end:
        best_len, best_off = 1, 0
        maxlen = min(F, end - pos)
        for c in reversed(table.get(bytes(buf[pos:pos+3]), [])):
            if c < pos - N + F: break   # outside window (decoder keeps N-F behind)
            l = 0
            while l < maxlen and buf[c + l] == buf[pos + l]: l += 1
            if l > best_len: best_len, best_off = l, c
            if l == maxlen: break
        if best_len <= P:
            w.bit(1); w.bits(buf[pos], 8); step = 1
        else:
            w.bit(0); w.bits(best_off & (N - 1), EI); w.bits(best_len - (P + 1), EJ); step = best_len
        for i in range(pos, pos + step): add(i)
        pos += step
    return w.flush()

def decode(data, limit=None):
    win = bytearray(b' ' * N); r = N - F; out = bytearray()
    bitpos, total = 0, len(data) * 8
    def get(n):
        nonlocal bitpos
        v = 0
        for _ in range(n):
            if bitpos >= total: raise EOFError
            v = (v << 1) | ((data[bitpos >> 3] >> (7 - (bitpos & 7))) & 1); bitpos += 1
        return v
    try:
        while True:
            if get(1):
                c = get(8); out.append(c); win[r] = c; r = (r + 1) & (N - 1)
            else:
                i = get(EI); j = get(EJ)
                for k in range(j + P + 1):
                    c = win[(i + k) & (N - 1)]; out.append(c); win[r] = c; r = (r + 1) & (N - 1)
    except EOFError:
        pass
    return bytes(out)

def wrap(lz):
    body = struct.pack('<I', 0x23411002) + bytes([0, 0, 0, 0, 0, 0, 0, 0x40]) + lz
    return struct.pack('<II', len(body), zlib.crc32(body) & 0xffffffff) + body

def check(path):
    d = open(path, 'rb').read()
    ln, crc = struct.unpack('<II', d[:8]); body = d[8:]
    print('length ok' if ln == len(body) else 'LENGTH MISMATCH', '| crc ok' if zlib.crc32(body) & 0xffffffff == crc else '| CRC MISMATCH',
          '| magic %#x' % struct.unpack('<I', body[:4])[0], '| version', body[4:12].hex())
    return decode(body[12:])

if __name__ == '__main__':
    if sys.argv[1] == 'check':
        raw = check(sys.argv[2]); sp, rv = struct.unpack('<II', raw[:8])
        print('decoded %d bytes; stack %#x reset %#x' % (len(raw), sp, rv))
    else:
        src, dst = sys.argv[1], sys.argv[2]
        raw = open(src, 'rb').read(); lz = encode(raw)
        rt = decode(lz)
        assert rt[:len(raw)] == raw, 'round-trip failed'
        open(dst, 'wb').write(wrap(lz))
        print('%s: %d bytes -> %d bytes .ota (round-trip ok)' % (dst, len(raw), len(lz) + 20))
