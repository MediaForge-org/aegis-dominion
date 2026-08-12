# Rendering foundation

`render::Layer` fixes the order: Terrain, Water, Road, Environment, Zones, Enemies, Towers, Projectiles, Effects, World UI, Screen UI and Modal UI. `RenderContext` identifies the active pass, `WorldRenderer` provides the current screen integration boundary and `RenderQueue` executes submitted work in layer order independent of submission order.

The current SFML gameplay objects still draw themselves inside the world pass. This foundation does not claim that they are renderer-independent. The next extraction should produce immutable render snapshots from simulation state and submit those to a 2D renderer; `MapDocument` and `src/core` remain reusable by a future 3D renderer.
