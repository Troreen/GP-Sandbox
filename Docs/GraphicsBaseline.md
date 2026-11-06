# Graphics cumulative baseline

These requirements recur in the remaining Graphics briefs. Recheck them after each feature; an older commit is useful only if it still satisfies the required baseline.

- [ ] WASD moves relative to camera rotation; Space/Ctrl moves along world up/down; right-mouse drag rotates without camera roll.
- [ ] Load static and skeletal FBX models and play FBX animation.
- [ ] Support textured materials and PSO configuration, shadow mapping for all required light types, physically based lighting, blending, and deferred rendering.
- [ ] Show at least three correctly rendered meshes with materials on a plane for shadow checking.
- [ ] Include directional, point, and spot lights, all casting shadows; handle shadows and ambient lighting correctly.
- [ ] Show at least two translucent meshes. Use deferred rendering for opaque geometry and forward rendering for translucency.
- [ ] Keep earlier required features working. From 7.1 onward retain HDR and tonemapping; from 7.2 retain bloom and its toggle; from 7.3 retain SSAO and its toggle; from 8 retain at least one particle system with two emitters.

The briefs say earlier requirements remain applicable. The opaque/translucent rendering distinction above resolves their repeated general wording that meshes use deferred rendering while translucent objects use forward rendering.

For 7.3, 8, and 9: student-written code must compile without warnings, remove junk files, and upload a ZIP project. Record any submission comment or collaborator information as requested by the portal. 7.1 and 7.2 are confirmation milestones; do not assume they each require a separate project upload.

Sources: [7.1](https://tga.omniway.se/exam/63921885), [7.2](https://tga.omniway.se/exam/63921886), [7.3](https://tga.omniway.se/exam/63921887), [8](https://tga.omniway.se/exam/63921888), [9](https://tga.omniway.se/exam/63921889). Read 3 November 2026.
