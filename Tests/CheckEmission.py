"""Render the real chest materials and reject a black emission debug pass."""
from pathlib import Path
import struct
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
configuration = sys.argv[1] if len(sys.argv) > 1 else "Debug"
runtime = root / "Bin" / configuration
subprocess.run([str(runtime / "Sandbox.exe"), "--emission-check"], cwd=runtime, check=True, timeout=60)
for frame, description in [(5, "translucent chests only (forward)"), (15, "opaque chest after resize (deferred)")]:
    data = (runtime / "Captures" / f"frame-{frame}.bmp").read_bytes()
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    assert height < 0, "Expected top-down framebuffer capture"
    bright = 0
    for y in range(abs(height)):
        for x in range(400, width):  # Exclude the ImGui panel entirely.
            pixel = offset + (y * width + x) * 4
            bright += max(data[pixel:pixel + 3]) > 20
    print(f"{description}: {bright} visible emission pixels", flush=True)
    assert bright >= 40, f"Missing chest emission in {description} path"
# Compare a frozen, identically lit scene with bloom off/on. Ignore the UI.
off = (runtime / "Captures/frame-25.bmp").read_bytes()
on = (runtime / "Captures/frame-35.bmp").read_bytes()
assert len(off) == len(on), "Bloom comparison requires matching viewport sizes"
offset = struct.unpack_from("<I", on, 10)[0]
width, height = struct.unpack_from("<ii", on, 18)
brightened = 0
for y in range(abs(height)):
    for x in range(400, width):
        pixel = offset + (y * width + x) * 4
        brightened += sum(on[pixel:pixel + 3]) > sum(off[pixel:pixel + 3]) + 9
print(f"Bloom: {brightened} pixels brightened outside the UI", flush=True)
assert brightened >= 40, "Bloom does not visibly affect the emissive scene"
print("PASS: forward/deferred chest emission, resize, and visible bloom")
