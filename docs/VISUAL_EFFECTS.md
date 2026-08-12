# Visual effects

The Phase-4 effect pool tracks position, velocity, lifetime, size, rotation, angular velocity, drag and optional gravity. Burst and trail emission cover impacts, muzzle particles and missile/cryo trails; rings and tracers remain dedicated inexpensive primitives.

Damage profiles are visually separated: kinetic uses warm sparks/shells, explosive uses orange bursts/rings and camera shake, energy uses cyan bolts/glow, electric uses green chained tracers, cryo uses pale shards/trails, and rail uses a purple charge ring/tracer with a restrained impulse. Deaths emit fragments/sparks, with larger boss bursts. Heavy splash and rail events request a short decaying camera offset; HUD and modal passes never shake.

Lighting uses additive radial fans and consistent soft drop shadows. There is no shader requirement or per-frame RenderTexture creation. A future bloom pass can consume the same emissive IDs/effect profiles after profiling.
