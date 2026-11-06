# GP Sandbox

Standalone C++20 / DirectX 11 graphics lab extracted from AGP. The hard-coded scene restores the active objects, transforms, materials, textures,
lighting, environment map, and initial camera from AGP commit `d583fa1`. The renderer
comes from the current AGP checkout, with bloom and SSAO enabled. See [EXTRACTION.md](EXTRACTION.md) for its exact source commit.

There is no game framework, World/Actor/component lifecycle, Unreal scene loading,
audio, AI, service locator, or dependency on another checkout. MeshInstance, RenderCamera,
and RenderLight are application-owned rendering objects with direct transforms.

## Open and run

Open `GraphicsSandbox.sln` in Visual Studio 2026. Build **Debug | x64**, select
**Sandbox** as the startup project, and press F5. Requires Desktop development with C++,
the **v145** toolset, and a Windows SDK. Dependencies used here are already bundled;
do not run AGP's setup scripts. `GenerateProject.bat` regenerates projects after changing
source membership or Premake settings.

From PowerShell, regardless of working directory:

```powershell
& 'C:\Users\tarik\Documents\GitHub\GP-Sandbox\Build.bat'
& 'C:\Users\tarik\Documents\GitHub\GP-Sandbox\Bin\Debug\Sandbox.exe'
```

`Build.bat` invokes the build helper with a process-local execution-policy override; it does not change system settings.
`Run.bat` also starts an existing Debug build. Content resolves relative to the executable,
and material/engine shaders are deployed beside it at build time. Build again after
editing shaders. This project uses the included minimal Content folder, not AGP-Content.

## Controls

- WASD: move relative to camera orientation.
- Space / Ctrl: move along world up / down. Shift: move faster.
- Hold right mouse: look without roll; pitch is clamped and the cursor is captured.
- Escape: quit. The Graphics lab panel resets the camera and changes camera speed.
- The panel toggles animation, transparency, shadows, bloom, and SSAO, and selects
  tonemapping and G-buffer debug views.

## Where to work

- `Source/Sandbox/DemoScene.cpp`: the entire hard-coded scene, material creation,
  three light types, and simple FBX animation sampling. Start here for assignments.
- `Source/Sandbox/FlyCamera.cpp`: free-fly controls.
- `Source/Sandbox/Main.cpp`: window, frame loop, ImGui, resize, and smoke captures.
- `Source/Engine/GraphicsEngine`: renderer, RHI, GPU resources, materials, and post effects.
- `Source/Engine/GraphicsEngine/Scene`: standalone rendering objects and transform math.
- `Source/Engine/GraphicsEngine/Assets/MeshLibrary.*`: static/skeletal FBX and clip loading.
- `Source/Engine/GraphicsEngine/Shaders`: shared graphics shaders.
- `Content/Materials`: the reference scene's material descriptions and texture bindings.
- `Content/Shaders/{Floor,Chest,Character,ColorChecker}Material.hlsli`: original material
  code, adapted to the current renderer's constant-buffer slot.
- `Source/Sandbox/PrimitiveMeshBuilder.*`: original procedural meshes, including the exact
  reference floor topology and UVs.
- [Docs/ReferenceScene.md](Docs/ReferenceScene.md): original scene values and asset provenance.

The demo owns six mesh instances: the marble floor, one opaque chest, two translucent
chests, the color checker, and TGA Bro playing the original breathing animation. Opaque geometry uses deferred rendering;
translucency uses forward rendering. Directional, point, and spot lights cast shadows.
The retained renderer includes PBR materials, normal/material texture slots, HDR,
multiple tonemappers, bloom, SSAO, environment maps, sprites, text, and debug lines.

Snapshots borrow scene objects for immediate rendering. Keep them alive until commands
have been recorded/executed. Meshes and materials are shared resources; every MeshInstance
owns its own 128-joint animation pose. Missing material slots use the fallback material.
No animation state machine is required for direct clip playback.

## Emission

[Docs/Emission.md](Docs/Emission.md) describes the optional RGB emission texture,
material controls, and GPU regression check. The chest's cyan ornaments now emit HDR
color and feed bloom. Adjust `MB_EmissionStrength` in `Content/Shaders/ChestMaterial.hlsli`.
The color checker now uses -90 degrees around Z.

## Verification and coursework

See [Docs/Verification.md](Docs/Verification.md) for the checks performed and their limits.

```powershell
.\Build.bat -Configuration Debug -Smoke
.\Build.bat -Configuration Release -Smoke
```

Smoke mode loads all demo assets, advances the animation, renders forty frames, resizes
the window, disables/re-enables effects/shadows/translucency, pauses animation, and changes
debug view/tonemapper. It exits automatically and saves actual GPU framebuffer captures
and a DirectX validation report in `Bin/<configuration>/Captures`. It fails on device
removal or debug-layer errors. Install Windows Graphics Tools for debug-layer validation;
without it the app runs with reduced diagnostics. Smoke mode does not simulate camera input.

[Docs/GraphicsBaseline.md](Docs/GraphicsBaseline.md) contains the saved course baseline.
This extraction is an assignment starting point, not a claim that all assignments are
complete. Particle systems and instanced rendering remain assignment work. The demo binds the original albedo, normal, and packed material maps for every material.

Build output, logs, and captures are ignored by Git. Source, generated Visual Studio
projects, required FBX/DDS assets, shader sources, and bundled dependencies are included.
This is an independent local repository with no remote, commits, or staged files.
Port improvements to AGP deliberately when useful; neither project automatically updates
the other. Review school dependency/asset redistribution terms before creating a public remote.
