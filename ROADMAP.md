# Aether Voxel Game - Development Roadmap

**Project:** Aether Game Engine Demo Game  
**Status:** In Progress  
**Last Updated:** 2026-05-13

---

## Core Gameplay Systems

### 1. World Persistence
- [ ] Save/load world state to disk
- [ ] Player position/inventory persistence
- [ ] Multiple save slots
- [ ] Auto-save on interval

### 2. Proper Terrain Generation
- [x] Perlin noise with octaves for realistic hills
- [x] Biome system (plains, forest, desert, mountains, ocean)
- [ ] Cave generation with tunnel systems
- [ ] Ore veins underground
- [ ] River/lake generation

### 3. Day/Night Cycle
- [ ] Sun/moon movement
- [ ] Dynamic lighting changes
- [ ] Mob spawning at night
- [ ] Bed/sleep mechanic to skip night

### 4. Health & Survival
- [ ] Health bar (hearts)
- [ ] Hunger bar (food meter)
- [ ] Fall damage
- [ ] Drowning in water
- [ ] Lava damage
- [ ] Death/respawn system

### 5. Inventory System
- [ ] Hotbar (9 slots)
- [ ] Full inventory grid (3x9 or 4x9)
- [ ] Item stacking
- [ ] Tool durability
- [ ] Item pickup/drop

### 6. Crafting
- [ ] Crafting grid (2x2 inventory, 3x3 table)
- [ ] Recipe system
- [ ] Crafting table block
- [ ] Tool/weapon crafting

### 7. Block Variety
- [ ] 20+ block types (ores, gems, flowers, etc.)
- [ ] Block hardness/mining speed
- [ ] Proper tool requirements (wood/stone/iron pickaxe)
- [ ] Block drops (not always same block)

### 8. Tools & Weapons
- [ ] Pickaxe, axe, shovel, sword
- [ ] Wood/stone/iron/gold/diamond tiers
- [ ] Attack damage
- [ ] Durability system

---

## Mobs & AI

### 9. Passive Mobs
- [ ] Pigs, cows, chickens
- [ ] AI: wander, flee when hit
- [ ] Drop meat/leather/feathers

### 10. Hostile Mobs
- [ ] Zombies (night only)
- [ ] Skeletons with bow shooting
- [ ] Spiders (can climb walls)
- [ ] Creepers (explode near player)

### 11. Mob AI
- [ ] Pathfinding around obstacles
- [ ] Aggro/detection range
- [ ] Day/night behavior changes
- [ ] Spawning rules (light level, biome)

---

## Visual Polish

### 12. Textures
- [ ] Block texture atlas
- [ ] Item textures
- [ ] GUI elements
- [ ] Crosshair, hotbar, hearts

### 13. Particles
- [ ] Block break particles
- [ ] Footstep particles
- [ ] Torch smoke
- [ ] Explosion particles

### 14. Lighting
- [ ] Torch placement with light radius
- [ ] Smooth lighting between blocks
- [ ] Ambient occlusion

### 15. Sky & Weather
- [ ] Clouds
- [ ] Rain/snow weather
- [ ] Fog in distance
- [ ] Stars at night

---

## Audio

### 16. Sound Effects
- [ ] Block break/place sounds
- [ ] Footsteps (different per block)
- [ ] Mob sounds
- [ ] Tool swing sounds
- [ ] Damage sounds

### 17. Music
- [ ] Ambient background music
- [ ] Day/night music variations
- [ ] Combat music when near mobs

---

## World Features

### 18. Structures
- [ ] Trees (oak, birch, pine variants)
- [ ] Villages with houses
- [ ] Dungeons underground
- [ ] Mineshafts
- [ ] Caves with stalactites

### 19. Flora
- [ ] Grass/flowers on surface
- [ ] Tall grass
- [ ] Cacti in desert
- [ ] Lily pads on water
- [ ] Mushrooms in caves

### 20. Ores & Resources
- [ ] Coal, iron, gold, diamond, emerald
- [ ] Ore distribution by depth
- [ ] Smelting in furnace

---

## Game Modes

### 21. Survival Mode
- [ ] Health/hunger
- [ ] Crafting required
- [ ] Mob threats
- [ ] Resource gathering

### 22. Creative Mode
- [ ] Infinite blocks
- [ ] Flight
- [ ] No damage
- [ ] Instant break

### 23. Adventure Mode
- [ ] Story objectives
- [ ] Quest system
- [ ] NPCs with dialog

---

## Multiplayer (Future)

### 24. Network Play
- [ ] LAN multiplayer
- [ ] Server hosting
- [ ] Player skins
- [ ] Chat system

---

## UI/UX

### 25. Main Menu
- [ ] Title screen with background
- [ ] Singleplayer/Multiplayer options
- [ ] Settings menu
- [ ] Key bindings config

### 26. In-Game UI
- [ ] Hotbar with selected item highlight
- [ ] Health/hunger bars
- [ ] Crosshair
- [ ] Debug info (F3)
- [ ] Pause menu

### 27. Settings
- [ ] Graphics quality
- [ ] Render distance
- [ ] Mouse sensitivity
- [ ] Volume controls
- [ ] Fullscreen/windowed

---

## Completed Features

- [x] Basic voxel world rendering
- [x] First-person camera with mouse look
- [x] WASD movement with collision
- [x] Block breaking and placing
- [x] Cross-chunk face culling fix
- [x] Transparency support (leaves, water)
- [x] Large world generation (100x100)
- [x] Procedural terrain with noise
- [x] Basic tree generation
- [x] Multiple block types (stone, dirt, grass, wood, leaves, sand, water, bedrock)
- [x] Player controller with gravity and jumping

---

## Notes

- Priority: Focus on Core Gameplay Systems first (1-8)
- Visual Polish can come after core mechanics are solid
- Multiplayer is a future stretch goal
- Keep performance in mind - optimize as we add features

---

*Last updated by: Aether*  
*Next review: After completing Core Gameplay Systems*
