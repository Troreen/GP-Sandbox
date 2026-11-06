# Material emission

MaterialDescription and .mat files accept an optional `emissionTexture` path.
Paths are relative to the .mat file, like albedo/normal/material textures. For example:

```json
"emissionTexture": "../Textures/MyEmission.dds"
```

GraphicsEngine::CreateMaterial loads/binds all four texture paths itself. Slot t3 is
EmissionTexture: a linear RGB color source, not a red-only mask multiplied by albedo.
Missing emission maps use the black default. Material hooks receive the sampled
color in MaterialPixelParameters.Emission and may tint, mask, or multiply it above 1.
The RGB values pass through the floating-point G-buffer and HDR lighting target for
opaque materials; forward lit/unlit materials also add emission before tonemapping.
The Emission debug view shows the final material emission in color. Bloom reads the
HDR scene before tonemapping, so sufficiently strong emission produces a halo.

## Chest material

The historical chest assets provide albedo, normal, and packed AO/roughness/metalness
textures. Their alpha channels contain no useful emission mask, and there is no
separate supplied emission map. The cyan ornamental regions are painted in albedo.
Both chest .mat files therefore reuse T_Chest_C.dds as their emission source.
ChestMaterial.hlsli selects cyan regions using the contrast of green/blue against red,
then multiplies their source color by MB_EmissionStrength (default 8). Wood and metal
remain non-emissive. MB_Tint still controls surface tint/opacity; translucent chests
share the same emission behavior. No source texture was edited or regenerated.

To tune the glow, change MB_EmissionStrength in Content/Shaders/ChestMaterial.hlsli
and rebuild. A dedicated authored emission map can later replace this color selection
without changing the renderer. The selection is specific to the chest artwork, not
a rule applied to other engine materials.

## Regression check

After building a configuration, run:

```powershell
python Tests/CheckEmission.py Debug
python Tests/CheckEmission.py Release
```

The check runs the actual renderer using --emission-check. It captures forward-only
chest emission, then opaque emission after resize, and rejects black emission views.
It also compares an identical frozen scene with bloom off/on, excluding ImGui pixels.
The initial reproduction failed with zero visible emission pixels. With the fix,
the Debug run finds 1662 forward emission pixels and 146 opaque emission pixels;
bloom brightens 6958 scene pixels. Shader-only success is not the acceptance signal:
the assertions inspect the GPU framebuffer. Smoke validation also checks the DirectX
debug message queue for errors. The separate normal --smoke run retains its original
behavior and exercises the standard debug views and toggles.
