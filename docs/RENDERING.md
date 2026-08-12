# Rendering foundation

`render::Layer` fixes the order: Terrain, Water, Road, Environment, Zones, Enemies, Towers, Projectiles, Effects, World UI, Screen UI and Modal UI. `RenderLayer.hpp` is SFML-free. `RenderContext` records the active pass and `RenderQueue` remains available for independently submitted work.

`GameScreen::buildRenderSnapshot()` extracts map, enemy, tower and projectile snapshots. `WorldRenderer` consumes them in the fixed order; no targeting, damage or wave logic exists in the renderer. Effects remain a bounded presentation pool and draw in Effects after snapshot projectiles.

Custom terrain uses repeat-enabled material textures plus deterministic seeded broad patches. Water has a deep surface, animated line noise and a shoreline. Roads use sampled Catmull-Rom points and layered triangle strips for shadow, shoulder, base, wear and markings. Seeded decorations avoid roads, water, blocked zones, spawn and core; authored `MapDocument` decorations take precedence.

Lighting is lightweight: consistent soft drop shadows, additive radial glow for emissive actors/projectiles/endpoints, ambient multiplication and a restrained world vignette. It allocates no shaders or render textures per frame. Heavy splash/rail events request short decaying world-only camera shake; screen and modal UI remain stable.

See `ROAD_RENDERING.md`, `VISUAL_EFFECTS.md` and `ENVIRONMENT_SYSTEM.md`.
