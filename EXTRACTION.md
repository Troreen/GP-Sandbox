# Extraction provenance

Copied from local AGP at commit `c681e32fb04bb20cd4a773c8be2d20c4d09505c5`.
The AGP working tree was clean when inspected. No AGP sources were modified.

This project owns its copy. It does not link to AGP, AGP-Content, their build outputs,
Unreal exports, FMOD, AI systems, or the game framework. Renderer input now uses
RenderCamera, RenderLight and MeshInstance, with direct shared Mesh/MaterialInterface
resources and an application-owned pose. Transform math and camera/light rendering
behavior were extracted from the framework without its ownership/lifecycle system.

Original source notices and third-party license files are retained. The bundled
TGA FBX importer and school demo FBX files are coursework dependencies; review their
redistribution terms before publishing this project publicly. A repository license
has not been invented for the original code or assets.

## Historical scene restoration

The active hard-coded scene and its 17 source assets were restored from d583fa1.
See Docs/ReferenceScene.md and Docs/ReferenceSceneAssets.json. The current renderer,
ImGui controls, bloom and SSAO remain in use.
