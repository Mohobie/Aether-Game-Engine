# Aether Voxel Game - Development Roadmap

**Project:** Aether Game Engine Demo Game  
**Status:** In Progress  
**Last Updated:** 2026-05-13

---

## Core Gameplay Systems

### 1. World Persistence
- [x] Save/load world state to disk
- [x] Player position persistence
- [x] Multiple save slots
- [x] Auto-save on interval
- [ ] Player inventory persistence

### 2. Proper Terrain Generation
- [x] Perlin noise with octaves for realistic hills
- [x] Biome system (plains, forest, desert, mountains, ocean)
- [x] Cave generation with tunnel systems
- [x] Ore veins underground
- [x] River/lake generation

### 3. Day/Night Cycle
- [ ] Sun/moon movement
- [ ] Dynamic lighting changes
- [ ] Mob spawning at night
- [ ] Bed/sleep mechanic to skip night

### 4. Health & Survival
- [x] Health system (damage/healing)
- [x] Hunger system (eating/starvation)
- [x] Breathing/drowning system
- [x] Death/respawn system
- [x] Experience/leveling system
- [ ] Fall damage
- [ ] Lava damage
- [x] Health/hunger UI (hearts/food bars)

### 5. Inventory System
- [x] Hotbar (9 slots)
- [x] Full inventory grid (3x9)
- [x] Item stacking
- [x] Tool durability
- [x] Item pickup/drop

### 6. Crafting
- [x] Crafting grid (2x2 inventory, 3x3 table)
- [x] Recipe system
- [ ] Crafting table block
- [x] Tool/weapon crafting

### 7. Block Variety
- [x] 20+ block types (ores, gems, flowers, etc.)
- [x] Block hardness/mining speed
- [x] Proper tool requirements (wood/stone/iron pickaxe)
- [x] Block drops (not always same block)

### 8. Tools & Weapons
- [x] Pickaxe, axe, shovel, sword
- [x] Wood/stone/iron/gold/diamond tiers
- [x] Attack damage
- [x] Durability system

---

## Mobs & AI

### 9. Passive Mobs
- [x] Mob system framework
- [x] Pigs (wander, flee)
- [x] Cows, chickens
- [x] Drop meat/leather/feathers

### 10. Hostile Mobs
- [x] Zombies (night only)
- [x] Skeletons with bow shooting
- [x] Spiders (can climb walls)
- [x] Creepers (explode near player)

### 11. Mob AI
- [x] Pathfinding around obstacles
- [x] Aggro/detection range
- [x] Day/night behavior changes
- [ ] Spawning rules (light level, biome)

---

## Visual Polish

### 12. Textures
- [ ] Block texture atlas
- [ ] Item textures
- [ ] GUI elements
- [ ] Crosshair, hotbar, hearts

### 13. Particles
- [x] Block break particles
- [x] Footstep particles
- [x] Torch smoke
- [x] Explosion particles

### 14. Lighting
- [x] Torch placement with light radius
- [x] Smooth lighting between blocks
- [x] Ambient occlusion

### 15. Sky & Weather
- [x] Clouds
- [x] Rain/snow weather
- [x] Fog in distance
- [x] Stars at night

---

## Audio

### 16. Sound Effects
- [x] Audio engine with 3D positioning
- [x] Volume groups (master, music, SFX)
- [x] Block break/place sounds
- [x] Footsteps (different per block)
- [x] Mob sounds
- [x] Tool swing sounds
- [x] Damage sounds

### 17. Music
- [x] Audio engine supports music playback
- [x] Ambient background music
- [x] Day/night music variations
- [x] Combat music when near mobs

---

## World Features

### 18. Structures
- [x] Trees (oak, birch, pine variants)
- [x] Villages with houses
- [x] Dungeons underground
- [ ] Mineshafts
- [x] Caves with stalactites

### 19. Flora
- [x] Grass/flowers on surface
- [x] Tall grass
- [x] Cacti in desert
- [ ] Lily pads on water
- [ ] Mushrooms in caves

### 20. Ores & Resources
- [ ] Coal, iron, gold, diamond, emerald
- [ ] Ore distribution by depth
- [ ] Smelting in furnace

---

## Game Modes

### 21. Survival Mode
- [x] Health/hunger
- [x] Crafting required
- [x] Mob threats
- [x] Resource gathering

### 22. Creative Mode
- [x] Flight mode
- [x] Infinite blocks
- [x] Instant break
- [x] No damage/hunger

### 22. Creative Mode
- [ ] Infinite blocks
- [ ] Flight
- [ ] No damage
- [ ] Instant break

### 23. Achievements
- [x] Basic achievements (mine first block, etc.)
- [x] Progress tracking
- [x] Unlock notifications

### 24. Adventure Mode
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
- [x] Title screen with background
- [x] Singleplayer/Multiplayer options
- [x] Settings menu
- [ ] Key bindings config

### 26. In-Game UI
- [x] Hotbar with selected item highlight
- [x] Health/hunger bars
- [x] Crosshair
- [x] Debug info (F3)
- [x] Pause menu

### 27. Settings
- [x] Graphics quality
- [x] Render distance
- [x] Mouse sensitivity
- [x] Volume controls
- [x] Fullscreen/windowed

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
