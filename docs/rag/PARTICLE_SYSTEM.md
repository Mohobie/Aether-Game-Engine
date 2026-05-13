# Particle System

## Quick Reference

```cpp
vge::ParticleSystem particles;

// Configure emitter
vge::EmitterConfig config;
config.spawnPosition = vge::Vec3(0, 10, 0);
config.spawnRate = 100.0f;
config.particleLife = 2.0f;
config.startColor = vge::Vec4(0.6f, 0.7f, 0.9f, 0.8f);
config.endColor = vge::Vec4(0.6f, 0.7f, 0.9f, 0.0f);

particles.SetConfig(config);
particles.StartEmission();

// Update in game loop
particles.Update(deltaTime);
```

## Features

### Particle Properties
- **Position/Velocity:** 3D movement with physics
- **Life:** Birth to death timeline
- **Size:** Base size with variation and over-life changes
- **Color:** RGBA with start/end interpolation
- **Rotation:** Angular velocity support
- **Alpha:** Transparency fading

### Emitter Types
- **Continuous:** Steady stream of particles
- **Burst:** All particles at once
- **Loop:** Repeat burst or continuous

## Configuration

```cpp
vge::EmitterConfig config;

// Spawn
config.spawnPosition = vge::Vec3(0, 10, 0);
config.spawnArea = vge::Vec3(5, 0, 5);  // Random spawn area

// Movement
config.spawnVelocity = vge::Vec3(0, -5, 0);
config.velocityRandomness = vge::Vec3(1, 1, 1);
config.gravity = vge::Vec3(0, -9.8f, 0);
config.drag = 0.1f;

// Appearance
config.particleSize = 0.1f;
config.sizeRandomness = 0.05f;
config.sizeOverLife = 0.0f;  // Shrink to nothing
config.startColor = vge::Vec4(1, 1, 1, 0.8f);
config.endColor = vge::Vec4(1, 1, 1, 0.0f);
config.fadeOverLife = true;
config.shrinkOverLife = true;

// Timing
config.spawnRate = 100.0f;  // Particles per second
config.particleLife = 2.0f;
config.lifeRandomness = 0.5f;

// Limits
config.maxParticles = 1000;
config.burstMode = false;
config.loop = true;
```

## Presets

### Rain
```cpp
config.spawnPosition = vge::Vec3(0, 50, 0);
config.spawnArea = vge::Vec3(50, 0, 50);
config.spawnVelocity = vge::Vec3(0, -20, 0);
config.gravity = vge::Vec3(0, 0, 0);  // No extra gravity
config.particleLife = 2.5f;
config.startColor = vge::Vec4(0.5f, 0.6f, 0.8f, 0.6f);
```

### Smoke
```cpp
config.spawnPosition = vge::Vec3(0, 0, 0);
config.spawnVelocity = vge::Vec3(0, 2, 0);
config.gravity = vge::Vec3(0, 0.5f, 0);  // Rise up
config.drag = 0.5f;
config.particleLife = 3.0f;
config.startColor = vge::Vec4(0.3f, 0.3f, 0.3f, 0.5f);
config.endColor = vge::Vec4(0.5f, 0.5f, 0.5f, 0.0f);
```

### Explosion
```cpp
config.burstMode = true;
config.burstCount = 50;
config.spawnVelocity = vge::Vec3(0, 5, 0);
config.velocityRandomness = vge::Vec3(10, 10, 10);
config.particleLife = 1.0f;
config.startColor = vge::Vec4(1, 0.5f, 0, 1);
config.endColor = vge::Vec4(0.5f, 0, 0, 0);
```

## Rendering

```cpp
// ASCII rendering
particles.RenderASCII(screenW, screenH, camera);

// Framebuffer rendering
particles.RenderFB(framebuffer, camera);
```

## Files
- `src/rendering/particles/particle_system.h`
- `src/rendering/particles/particle_system.cpp`
