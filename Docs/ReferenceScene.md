# Reference scene: d583fa1

The sandbox scene now reproduces the active scene built by
Source/Application/ModelViewer/ModelViewer.cpp::LoadScene in AGP commit d583fa1.
The Graphics lab ImGui panel, bloom, SSAO, and the current renderer are retained.
Rendering can therefore differ from the historical renderer; scene data and assets
come from the historical commit.

| Object | Position | Rotation (yaw, pitch, roll degrees) | Scale | Material |
| --- | --- | --- | --- | --- |
| Floor | 0, 0, 260 | 0, -90, 0 | 1100, 1100, 1100 | FloorMaterial |
| Chest | 135, 0, 285 | 0, 0, 0 | 1, 1, 1 | ChestMaterial |
| Translucent chest 1 | -200, 0, -100 | 0, 0, 0 | 1, 1, 1 | ChestMaterial_Alpha, opacity 0.35 |
| Translucent chest 2 | 0, 0, -100 | 0, 0, 0 | 1, 1, 1 | ChestMaterial_Alpha, opacity 0.75 |
| Color checker | -145, 40, 365 | 0, 0, -90 | 1, 1, 1 | ColorCheckerMaterial |
| TGA Bro | 0, 0, 250 | 180, 0, 0 | 1, 1, 1 | CharacterMaterial |

TGA Bro loops A_C_TGA_Bro_Idle_Brething.fbx (historical spelling).
The floor uses the original PrimitiveMeshBuilder implementation. The historical
smooth-sphere position was unused, so no sphere is added. Commented-out lights
are also excluded.

Camera starts at (0, 260, -950), aimed at (25, 0, 260), with 90-degree horizontal FOV,
near plane 1, and far plane 50000. Reset camera restores this view.

| Light | Position | Color | Intensity | Other settings |
| --- | --- | --- | --- | --- |
| Directional | -450, 650, -350 | 1, 0.96, 0.9 | 10 | Aimed at scene focus |
| Warm character point | -90, 180, 150 | 1, 0.42, 0.22 | 20 | Radius 760 |
| Spot | 430, 430, -210 | 0.55, 0.7, 1 | 30 | Radius 1200; cones 18/34 degrees; aimed at scene focus |

All use shadows. The original T_Shipyard.dds cubemap supplies environment lighting,
without drawing a skybox. The sandbox retains its current shadow/rendering implementation.
The 10,000 scaling of point/spot intensities matches the historical renderer.

## Exact source assets

ReferenceSceneAssets.json lists 17 FBX/DDS files extracted directly from Git blobs,
with SHA-256 checksums. These are byte-identical to d583fa1, rather than copies of
possibly changed assets from the current Content folder.

Material descriptions retain original values with paths relocated under Content.
Material shaders retain original behavior with MaterialBuffer moved from b3 to b4,
as required by the current renderer. No replacement checker texture or colored
primitive material is used by this scene.

## Requested changes after restoration

The color checker now uses Z rotation -90 degrees (yaw/pitch zero).
The original FBX and DDS assets remain unchanged. Chest materials now opt into
emission using the cyan inlays in T_Chest_C.dds; see Emission.md. Bloom and SSAO
remain available in the original Graphics lab panel.
