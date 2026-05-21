import struct
import zlib
from pathlib import Path

root = Path(__file__).resolve().parents[1] / "tests/fixtures/03-handshake-zb-chunked"
chunks = []
for name in ("000.bin", "001.bin"):
    data = (root / name).read_bytes()[12:]
    off = 0
    if data[0] == 0x65:
        off += 4
    if data[off : off + 4] == bytes([0, 0, 0x5A, 0x42]):
        off += 4
    body = data[off:]
    co, ts, cs = struct.unpack_from("<III", body, 0)
    chunks.append((co, body[12 : 12 + cs]))
    print(name, co, ts, cs)

buf = bytearray(max(co + len(c) for co, c in chunks) + 1)
for co, c in chunks:
    buf[co : co + len(c)] = c
out = zlib.decompress(bytes(buf))
print("inflated", len(out), "head", out[:40])
print("Synchronize at", out.find(b"Synchronize"))
