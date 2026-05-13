#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/ore_generator.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "rendering/sky/day_night_cycle.h"
#include "platform/window.h"
#include "platform/input_manager.h"
#include "core/player_controller.h"
#include "core/save_game.h"
#include "core/crafting.h"
#include "game/block_interaction.h"
#include "debug/debug_renderer.h"
#include "audio/audio_engine.h"
#include "editor/in_game_editor.h"
#include <iostream>
#include <cmath>

// Mouse tracking for camera
static double lastMouseX = 0, lastMouseY = 0;
static bool firstMouse = true;

void MouseCallback(double x, double y, vge::Input& input, vge::Window& window) {
    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
    }
    
    double dx = x - lastMouseX;
    double dy = y - lastMouseY;
    
    lastMouseX = x;
    lastMouseY = y;
    
    input.SetMouseDelta(static_cast<float>(dx), static_cast<float>(dy));
}

int main() {
    // 1. Create window
    vge::Window window;
    if (!window.Initialize(1280, 720, "My Voxel Game - Large World")) {
        std::cerr << "Failed to initialize window" << std::endl;
        return 1;
    }
    
    // 2. Create renderer
    vge::Renderer renderer;
    if (!renderer.Initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return 1;
    }
    renderer.SetViewport(0, 0, 1280, 720);
    
    // 3. Create camera
    vge::Camera camera;
    
    // 4. Register blocks
    vge::BlockRegistry& registry = vge::BlockRegistry::GetInstance();
    
    vge::BlockDef stoneDef;
    stoneDef.id = "stone";
    stoneDef.name = "Stone";
    stoneDef.solid = true;
    stoneDef.opaque = true;
    stoneDef.hardness = 2.0f;
    stoneDef.color = vge::Vec3(0.62f, 0.62f, 0.62f);
    registry.RegisterBlock(stoneDef);
    
    vge::BlockDef dirtDef;
    dirtDef.id = "dirt";
    dirtDef.name = "Dirt";
    dirtDef.solid = true;
    dirtDef.opaque = true;
    dirtDef.hardness = 1.0f;
    dirtDef.color = vge::Vec3(0.47f, 0.33f, 0.27f);
    registry.RegisterBlock(dirtDef);
    
    vge::BlockDef grassDef;
    grassDef.id = "grass";
    grassDef.name = "Grass";
    grassDef.solid = true;
    grassDef.opaque = true;
    grassDef.hardness = 1.0f;
    grassDef.color = vge::Vec3(0.30f, 0.69f, 0.31f);
    registry.RegisterBlock(grassDef);
    
    vge::BlockDef woodDef;
    woodDef.id = "wood";
    woodDef.name = "Wood";
    woodDef.solid = true;
    woodDef.opaque = true;
    woodDef.hardness = 1.5f;
    woodDef.color = vge::Vec3(0.55f, 0.43f, 0.39f);
    registry.RegisterBlock(woodDef);
    
    vge::BlockDef leavesDef;
    leavesDef.id = "leaves";
    leavesDef.name = "Leaves";
    leavesDef.solid = true;
    leavesDef.opaque = false;
    leavesDef.hardness = 0.5f;
    leavesDef.color = vge::Vec3(0.18f, 0.49f, 0.20f);
    registry.RegisterBlock(leavesDef);
    
    vge::BlockDef sandDef;
    sandDef.id = "sand";
    sandDef.name = "Sand";
    sandDef.solid = true;
    sandDef.opaque = true;
    sandDef.hardness = 1.0f;
    sandDef.color = vge::Vec3(1.0f, 0.96f, 0.62f);
    registry.RegisterBlock(sandDef);
    
    vge::BlockDef waterDef;
    waterDef.id = "water";
    waterDef.name = "Water";
    waterDef.solid = false;
    waterDef.opaque = false;
    waterDef.hardness = 0.0f;
    waterDef.color = vge::Vec3(0.13f, 0.53f, 0.95f);
    registry.RegisterBlock(waterDef);
    
    vge::BlockDef bedrockDef;
    bedrockDef.id = "bedrock";
    bedrockDef.name = "Bedrock";
    bedrockDef.solid = true;
    bedrockDef.opaque = true;
    bedrockDef.hardness = 10.0f;
    bedrockDef.color = vge::Vec3(0.26f, 0.26f, 0.26f);
    registry.RegisterBlock(bedrockDef);
    
    // Ore blocks
    vge::BlockDef coalOreDef;
    coalOreDef.id = "coal_ore";
    coalOreDef.name = "Coal Ore";
    coalOreDef.solid = true;
    coalOreDef.opaque = true;
    coalOreDef.hardness = 3.0f;
    coalOreDef.color = vge::Vec3(0.15f, 0.15f, 0.15f);
    registry.RegisterBlock(coalOreDef);
    
    vge::BlockDef ironOreDef;
    ironOreDef.id = "iron_ore";
    ironOreDef.name = "Iron Ore";
    ironOreDef.solid = true;
    ironOreDef.opaque = true;
    ironOreDef.hardness = 3.0f;
    ironOreDef.color = vge::Vec3(0.73f, 0.55f, 0.45f);
    registry.RegisterBlock(ironOreDef);
    
    vge::BlockDef goldOreDef;
    goldOreDef.id = "gold_ore";
    goldOreDef.name = "Gold Ore";
    goldOreDef.solid = true;
    goldOreDef.opaque = true;
    goldOreDef.hardness = 3.0f;
    goldOreDef.color = vge::Vec3(1.0f, 0.84f, 0.0f);
    registry.RegisterBlock(goldOreDef);
    
    vge::BlockDef diamondOreDef;
    diamondOreDef.id = "diamond_ore";
    diamondOreDef.name = "Diamond Ore";
    diamondOreDef.solid = true;
    diamondOreDef.opaque = true;
    diamondOreDef.hardness = 3.0f;
    diamondOreDef.color = vge::Vec3(0.0f, 0.8f, 0.8f);
    registry.RegisterBlock(diamondOreDef);
    
    vge::BlockDef emeraldOreDef;
    emeraldOreDef.id = "emerald_ore";
    emeraldOreDef.name = "Emerald Ore";
    emeraldOreDef.solid = true;
    emeraldOreDef.opaque = true;
    emeraldOreDef.hardness = 3.0f;
    emeraldOreDef.color = vge::Vec3(0.0f, 0.9f, 0.3f);
    registry.RegisterBlock(emeraldOreDef);
    
    // Decorative blocks
    vge::BlockDef flowerDef;
    flowerDef.id = "flower";
    flowerDef.name = "Flower";
    flowerDef.solid = false;
    flowerDef.opaque = false;
    flowerDef.hardness = 0.0f;
    flowerDef.color = vge::Vec3(1.0f, 0.5f, 0.8f);
    registry.RegisterBlock(flowerDef);
    
    vge::BlockDef tallGrassDef;
    tallGrassDef.id = "tall_grass";
    tallGrassDef.name = "Tall Grass";
    tallGrassDef.solid = false;
    tallGrassDef.opaque = false;
    tallGrassDef.hardness = 0.0f;
    tallGrassDef.color = vge::Vec3(0.4f, 0.7f, 0.3f);
    registry.RegisterBlock(tallGrassDef);
    
    vge::BlockDef cactusDef;
    cactusDef.id = "cactus";
    cactusDef.name = "Cactus";
    cactusDef.solid = true;
    cactusDef.opaque = true;
    cactusDef.hardness = 0.5f;
    cactusDef.color = vge::Vec3(0.2f, 0.6f, 0.2f);
    registry.RegisterBlock(cactusDef);
    
    // 5. Create world and generate large terrain
    vge::World world;
    world.SetSeed(12345);
    
    std::cout << "[Game] Generating world..." << std::endl;
    // Generate a large hilly world (size = 50 means 100x100 blocks)
    vge::WorldGenerator::GenerateHillyWorld(world, 50);
    
    // Generate ores
    std::cout << "[Game] Generating ores..." << std::endl;
    vge::OreGenerator oreGen;
    
    vge::OreType coalOre;
    coalOre.blockId = "coal_ore";
    coalOre.blockType = registry.GetBlockId("coal_ore");
    coalOre.rarity = 0.02f;
    coalOre.minHeight = 5;
    coalOre.maxHeight = 60;
    coalOre.veinSize = 12;
    coalOre.veinsPerChunk = 3;
    oreGen.RegisterOre(coalOre);
    
    vge::OreType ironOre;
    ironOre.blockId = "iron_ore";
    ironOre.blockType = registry.GetBlockId("iron_ore");
    ironOre.rarity = 0.015f;
    ironOre.minHeight = 5;
    ironOre.maxHeight = 45;
    ironOre.veinSize = 8;
    ironOre.veinsPerChunk = 2;
    oreGen.RegisterOre(ironOre);
    
    vge::OreType goldOre;
    goldOre.blockId = "gold_ore";
    goldOre.blockType = registry.GetBlockId("gold_ore");
    goldOre.rarity = 0.008f;
    goldOre.minHeight = 5;
    goldOre.maxHeight = 30;
    goldOre.veinSize = 6;
    goldOre.veinsPerChunk = 1;
    oreGen.RegisterOre(goldOre);
    
    vge::OreType diamondOre;
    diamondOre.blockId = "diamond_ore";
    diamondOre.blockType = registry.GetBlockId("diamond_ore");
    diamondOre.rarity = 0.005f;
    diamondOre.minHeight = 5;
    diamondOre.maxHeight = 16;
    diamondOre.veinSize = 4;
    diamondOre.veinsPerChunk = 1;
    oreGen.RegisterOre(diamondOre);
    
    oreGen.GenerateOresInWorld(world, 50);
    std::cout << "[Game] World generation complete!" << std::endl;
    
    // 6. Create input
    vge::Input input;
    
    // Set up mouse callback
    window.SetCursorCallback([&input, &window](double x, double y) {
        MouseCallback(x, y, input, window);
    });
    
    // 7. Create player controller
    vge::PlayerController player;
    // Find a good starting position on top of terrain
    player.SetPosition(vge::Vec3(0, 20, 0));
    
    // 8. Create block interaction
    vge::BlockInteraction blockInteraction;
    blockInteraction.Initialize(world);
    
    // 9. Create audio
    vge::AudioEngine audio;
    audio.Initialize();
    
    // 10. Create editor
    vge::InGameEditor editor(&world, &camera, &input, &renderer);
    editor.Initialize();
    
    // 11. Create day/night cycle
    vge::DayNightCycle dayNightCycle;
    dayNightCycle.SetDayLength(1200.0f); // 20 minutes per day
    renderer.SetDayNightCycle(&dayNightCycle);
    
    // 12. Create crafting system
    vge::CraftingSystem craftingSystem;
    
    // Add basic recipes
    vge::CraftingRecipe plankRecipe;
    plankRecipe.recipeID = "planks";
    plankRecipe.displayName = "Wooden Planks";
    plankRecipe.outputItemID = "wood";
    plankRecipe.outputCount = 4;
    plankRecipe.shapeless = true;
    plankRecipe.ingredients.push_back(vge::RecipeIngredient("wood", 1));
    craftingSystem.AddRecipe(plankRecipe);
    
    vge::CraftingRecipe stickRecipe;
    stickRecipe.recipeID = "sticks";
    stickRecipe.displayName = "Sticks";
    stickRecipe.outputItemID = "stick";
    stickRecipe.outputCount = 4;
    stickRecipe.shapeless = true;
    stickRecipe.ingredients.push_back(vge::RecipeIngredient("wood", 2));
    craftingSystem.AddRecipe(stickRecipe);
    
    // 13. Create save game manager
    vge::SaveGameManager saveManager;
    saveManager.Initialize("saves");
    
    // 13. Create debug renderer
    vge::DebugRenderer& debug = vge::GetDebugRenderer();
    debug.Initialize();
    
    // Lock cursor for first-person controls
    window.SetCursorMode(true);
    
    // 12. Game loop
    bool running = true;
    float deltaTime = 1.0f / 60.0f;
    
    // Block selection for placing
    int selectedBlock = 1; // Start with stone
    std::string blockTypes[] = {"stone", "dirt", "grass", "wood", "leaves", "sand", "water", "coal_ore", "iron_ore", "gold_ore", "diamond_ore", "flower", "tall_grass", "cactus"};
    
    while (running) {
        // Handle window events
        window.PollEvents();
        if (window.ShouldClose()) {
            running = false;
        }
        
        // Update input
        input.Update();
        
        // Toggle editor with Escape
        if (input.IsKeyJustPressed(vge::KeyCode::Escape)) {
            editor.Toggle();
            window.SetCursorMode(!editor.IsActive());
        }
        
        // Block selection with number keys
        if (input.IsKeyJustPressed(vge::KeyCode::Key1)) selectedBlock = 0;
        if (input.IsKeyJustPressed(vge::KeyCode::Key2)) selectedBlock = 1;
        if (input.IsKeyJustPressed(vge::KeyCode::Key3)) selectedBlock = 2;
        if (input.IsKeyJustPressed(vge::KeyCode::Key4)) selectedBlock = 3;
        if (input.IsKeyJustPressed(vge::KeyCode::Key5)) selectedBlock = 4;
        if (input.IsKeyJustPressed(vge::KeyCode::Key6)) selectedBlock = 5;
        if (input.IsKeyJustPressed(vge::KeyCode::Key7)) selectedBlock = 6;
        
        if (editor.IsActive()) {
            // Editor mode
            editor.Update(deltaTime, input);
        } else {
            // Game mode - player movement
            player.Update(deltaTime, input, world);
            
            // Update camera to follow player (first-person)
            camera.SetPosition(player.GetPosition() + vge::Vec3(0, 1.6f, 0));
            camera.SetRotation(player.GetYaw(), player.GetPitch(), 0);
            
            // Block breaking (left click - Q key for now)
            if (input.IsKeyJustPressed(vge::KeyCode::Q)) {
                player.BreakBlock(world);
            }
            
            // Block placing (E key)
            if (input.IsKeyJustPressed(vge::KeyCode::E)) {
                player.PlaceBlock(world, registry.GetBlockId(blockTypes[selectedBlock]));
            }
            
            // Day/night cycle controls (use number keys for now)
            if (input.IsKeyJustPressed(vge::KeyCode::Key1)) {
                dayNightCycle.SkipToDawn();
            }
            if (input.IsKeyJustPressed(vge::KeyCode::Key2)) {
                dayNightCycle.SkipToNoon();
            }
            if (input.IsKeyJustPressed(vge::KeyCode::Key3)) {
                dayNightCycle.SkipToDusk();
            }
            if (input.IsKeyJustPressed(vge::KeyCode::Key4)) {
                dayNightCycle.SkipToMidnight();
            }
        }
        
        // Update day/night cycle
        dayNightCycle.Update(deltaTime);
        
        // Render
        renderer.BeginFrame();
        
        renderer.RenderWorld(world, camera);
        
        // Render crosshair
        renderer.RenderCrosshair(1280, 720);
        
        // Render editor visuals if active
        if (editor.IsActive()) {
            editor.Render();
        }
        
        // Render debug visualization
        debug.Render(camera);
        debug.Update(deltaTime);
        debug.Clear();
        
        renderer.EndFrame();
        window.SwapBuffers();
    }
    
    // Cleanup
    editor.Shutdown();
    audio.Shutdown();
    renderer.Shutdown();
    window.Shutdown();
    
    return 0;
}
