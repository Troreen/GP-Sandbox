# Extraction verification ? 5 November 2026

- Debug | x64: built with VS 2026 / v145, zero warnings and zero errors.
- Release | x64: built with VS 2026 / v145, zero warnings and zero errors.
- Both configurations: forty-frame smoke run exited successfully; DirectX debug layer
  available, device status healthy, zero debug-layer errors (no warning messages recorded).
- Additional Debug smoke run started with the repository root as working directory,
  confirming asset/shader lookup does not depend on starting in Bin/Debug.
- Smoke exercises asset/material loading, skeletal animation advancement, opaque and
  translucent rendering, shadow/effect toggles, viewport resize, animation pause,
  normal debug view, and return to lit rendering with ACES.
- Actual framebuffer captures inspected: lit textured scene with shadows, bloom,
  skeletal character, and translucent meshes; world-normal view after resize;
  restored lit rendering after re-enabling effects.
- Dependency scan: no GameFramework, ServiceLocator, AISystems, FMOD, JSON dependency,
  or original AGP checkout path in standalone source/build projects. The only deployed
  third-party runtime DLL is libfbxsdk.dll.
- AGP working tree unchanged. Sandbox is a separate local Git repository on main,
  with no remote, commits, or staged files.

## Limits

Native desktop automation failed with ?Computer Use native pipe is unavailable?.
Mouse-look, held keyboard movement, UI clicks, focus-loss cursor release, minimize,
and interactive window closing were not verified hands-on. The camera implements
foreground-window gating, ImGui capture gating, world-up movement, clamped pitch,
cursor capture/recentering, and cursor restoration on release/shutdown.
The automatic run does not simulate native input or validate every material variant.
This verification establishes the extracted baseline; it is not an assignment grade.

## Repeat

Run `Build.bat -Configuration Debug -Smoke` or the Release equivalent.
Build logs are in the repository root. Captures and smoke-report.txt live under
Bin/<configuration>/Captures. Source and assets are independent of AGP and AGP-Content.

## d583fa1 scene restoration

Rebuilt Debug and Release after restoring the scene: zero compiler warnings/errors
and successful smoke runs with the debug layer available and zero DirectX errors.
Verified all 17 historical FBX/DDS asset checksums against the extraction manifest.
Inspected lit captures with the marble floor and all three textured chests, and the
world-normal capture with translucent chests hidden, revealing TGA Bro and the color
checker. Camera, transforms, materials, opacities, light settings, breathing clip and
environment texture follow the historical scene. Current ImGui panel and bloom/SSAO
controls remain unchanged. See ReferenceScene.md for the exact scene values.

## Emission diagnosis and color-checker rotation

The original GPU emission check reproduced a fully black emission debug pass (zero
visible scene pixels). Engine-only texture loading support still correctly produced
black until chest emission was explicitly authored. The chest has no dedicated
emission data in the supplied maps: the original material lacked an emission binding,
and its hook only multiplied albedo by tint. Optional RGB emission binding now lives
in GraphicsEngine::CreateMaterial; the chest hook selects cyan inlays as its mask.
The emission debug view now reads final RGB material emission, including custom hooks.
Forward and deferred regression captures pass, and a frozen bloom comparison passes.
The checker rotation is now (yaw 0, pitch 0, roll -90). Existing texture blob checksums
remain intact. See Emission.md for repeatable commands and results.
