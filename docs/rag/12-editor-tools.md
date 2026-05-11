# Editor Tools

The engine includes an in-game editor for level design, testing, and debugging.

## Editor Mode

```cpp
#include "editor/in_game_editor.h"

// Create editor
vge::InGameEditor editor;
editor.Initialize(&world, &renderer, &ui);

// Toggle editor mode
void Update(float dt) {
    if (input.GetKeyDown(Key::F1)) {
        editor.Toggle();
        input.SetCursorLocked(!editor.IsActive());
    }
    
    if (editor.IsActive()) {
        editor.Update(dt);
    }
}

void Render() {
    if (editor.IsActive()) {
        editor.Render();
    }
}
```

## Block Editing

```cpp
// Place block
editor.SetSelectedBlock(vge::BlockType::Stone);

// In editor mode:
// Left click - Place block
// Right click - Remove block
// Middle click - Pick block type
// Scroll - Change block type

// Brush settings
editor.SetBrushSize(3);        // 3x3 brush
editor.SetBrushShape(BrushShape::Sphere);  // or Cube, Cylinder

// Fill area
editor.FillArea(start, end, vge::BlockType::Water);

// Replace blocks
editor.Replace(vge::BlockType::Dirt, vge::BlockType::Grass);

// Clear area
editor.ClearArea(start, end);
```

## Entity Spawning

```cpp
#include "editor/entity_spawner.h"

// Spawn entity at position
vge::EntityID enemy = editor.SpawnEntity("enemy", position);
vge::EntityID item = editor.SpawnEntity("item", position);
vge::EntityID prop = editor.SpawnEntity("tree", position);

// Entity properties
editor.SelectEntity(entity);
editor.SetEntityPosition(entity, newPos);
editor.SetEntityRotation(entity, newRot);
editor.SetEntityScale(entity, newScale);

// Delete entity
editor.DeleteEntity(entity);

// Duplicate entity
vge::EntityID copy = editor.DuplicateEntity(entity);
```

## Voxel Editor

```cpp
#include "voxel/voxel_editor.h"

// Sculpting tools
editor.SetTool(VoxelTool::Sculpt);
editor.SetToolSize(5.0f);
editor.SetToolStrength(0.5f);

// Smooth terrain
editor.SmoothArea(center, radius);

// Raise/Lower terrain
editor.RaiseTerrain(center, radius, amount);
editor.LowerTerrain(center, radius, amount);

// Paint blocks
editor.PaintArea(center, radius, vge::BlockType::Grass);

// Erode terrain
editor.Erode(center, radius, iterations);
```

## Debugging Tools

```cpp
// Toggle debug visualization
editor.SetShowChunkBorders(true);
editor.SetShowCollisionBoxes(true);
editor.SetShowNavMesh(true);
editor.SetShowLightRanges(true);
editor.SetShowFPS(true);

// Wireframe mode
editor.SetWireframe(true);

// Free camera (noclip)
editor.SetFreeCamera(true);

// Time control
editor.SetTimeOfDay(0.5f);  // Noon
editor.PauseTime(true);
editor.SetTimeScale(10.0f);  // Fast forward

// Weather control
editor.SetWeather(WeatherType::Clear);
editor.SetWeather(WeatherType::Rain);
editor.SetWeather(WeatherType::Storm);
```

## Console Commands

```cpp
#include "ui/console_commands.h"

// Register custom commands
console.RegisterCommand("spawn", [](const std::vector<std::string>& args) {
    if (args.size() < 2) {
        console.Log("Usage: spawn [entity_type] [x] [y] [z]");
        return;
    }
    
    std::string type = args[1];
    vge::Vec3 pos = player.GetPosition() + player.GetForward() * 5.0f;
    
    if (args.size() >= 5) {
        pos.x = std::stof(args[2]);
        pos.y = std::stof(args[3]);
        pos.z = std::stof(args[4]);
    }
    
    editor.SpawnEntity(type, pos);
    console.Log("Spawned " + type + " at " + pos.ToString());
});

console.RegisterCommand("setblock", [](const std::vector<std::string>& args) {
    if (args.size() < 5) {
        console.Log("Usage: setblock [x] [y] [z] [block_type]");
        return;
    }
    
    int x = std::stoi(args[1]);
    int y = std::stoi(args[2]);
    int z = std::stoi(args[3]);
    vge::BlockType block = StringToBlockType(args[4]);
    
    world.SetBlock(x, y, z, block);
    console.Log("Set block at " + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z));
});

console.RegisterCommand("save", [](const std::vector<std::string>& args) {
    std::string filename = (args.size() > 1) ? args[1] : "save.dat";
    saveSystem.SaveWorld(world, filename);
    console.Log("World saved to " + filename);
});

console.RegisterCommand("load", [](const std::vector<std::string>& args) {
    std::string filename = (args.size() > 1) ? args[1] : "save.dat";
    saveSystem.LoadWorld(world, filename);
    console.Log("World loaded from " + filename);
});

console.RegisterCommand("tp", [](const std::vector<std::string>& args) {
    if (args.size() < 4) {
        console.Log("Usage: tp [x] [y] [z]");
        return;
    }
    
    float x = std::stof(args[1]);
    float y = std::stof(args[2]);
    float z = std::stof(args[3]);
    
    player.Teleport(vge::Vec3(x, y, z));
    console.Log("Teleported to " + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z));
});

console.RegisterCommand("give", [](const std::vector<std::string>& args) {
    if (args.size() < 2) {
        console.Log("Usage: give [item] [count]");
        return;
    }
    
    std::string item = args[1];
    int count = (args.size() > 2) ? std::stoi(args[2]) : 1;
    
    player.GetInventory().AddItem(item, count);
    console.Log("Gave " + std::to_string(count) + "x " + item);
});

console.RegisterCommand("fly", [](const std::vector<std::string>& args) {
    player.SetFlying(!player.IsFlying());
    console.Log(player.IsFlying() ? "Flight enabled" : "Flight disabled");
});

console.RegisterCommand("god", [](const std::vector<std::string>& args) {
    player.SetGodMode(!player.IsGodMode());
    console.Log(player.IsGodMode() ? "God mode enabled" : "God mode disabled");
});

console.RegisterCommand("time", [](const std::vector<std::string>& args) {
    if (args.size() < 2) {
        console.Log("Usage: time [0-1] (0=midnight, 0.25=sunrise, 0.5=noon, 0.75=sunset)");
        return;
    }
    
    float time = std::stof(args[1]);
    dayNightCycle.SetTime(time);
    console.Log("Time set to " + std::to_string(time));
});

console.RegisterCommand("weather", [](const std::vector<std::string>& args) {
    if (args.size() < 2) {
        console.Log("Usage: weather [clear/rain/storm]");
        return;
    }
    
    std::string weather = args[1];
    if (weather == "clear") weatherSystem.SetWeather(WeatherType::Clear);
    else if (weather == "rain") weatherSystem.SetWeather(WeatherType::Rain);
    else if (weather == "storm") weatherSystem.SetWeather(WeatherType::Storm);
    
    console.Log("Weather set to " + weather);
});
```

## Editor UI

```cpp
// Main editor panel
vge::UIElement* editorPanel = ui.CreatePanel("Editor");
editorPanel->SetPosition(10, 10);
editorPanel->SetSize(300, 600);

// Tool selection
vge::UIText* toolLabel = editorPanel->AddChild<vge::UIText>();
toolLabel->SetText("Tool: ");

vge::UIDropdown* toolDropdown = editorPanel->AddChild<vge::UIDropdown>();
toolDropdown->AddOption("Select");
toolDropdown->AddOption("Place");
toolDropdown->AddOption("Remove");
toolDropdown->AddOption("Paint");
toolDropdown->AddOption("Fill");
toolDropdown->AddOption("Smooth");
toolDropdown->OnSelect([](int index, const std::string& value) {
    editor.SetTool((EditorTool)index);
});

// Block selection
vge::UIText* blockLabel = editorPanel->AddChild<vge::UIText>();
blockLabel->SetText("Block: ");

vge::UIGridLayout* blockGrid = editorPanel->AddChild<vge::UIGridLayout>();
blockGrid->SetColumns(4);

for (int i = 0; i < (int)vge::BlockType::Count; ++i) {
    vge::UIButton* blockBtn = blockGrid->AddChild<vge::UIButton>();
    blockBtn->SetSize(40, 40);
    blockBtn->SetBackgroundTexture(GetBlockTexture((vge::BlockType)i));
    blockBtn->OnClick([i]() {
        editor.SetSelectedBlock((vge::BlockType)i);
    });
}

// Brush settings
vge::UIText* sizeLabel = editorPanel->AddChild<vge::UIText>();
sizeLabel->SetText("Brush Size: ");

vge::UISlider* sizeSlider = editorPanel->AddChild<vge::UISlider>();
sizeSlider->SetRange(1, 20);
sizeSlider->SetValue(1);
sizeSlider->OnValueChanged([](float value) {
    editor.SetBrushSize((int)value);
});

// Action buttons
vge::UIButton* undoBtn = editorPanel->AddChild<vge::UIButton>();
undoBtn->SetText("Undo (Ctrl+Z)");
undoBtn->OnClick([]() {
    editor.Undo();
});

vge::UIButton* redoBtn = editorPanel->AddChild<vge::UIButton>();
redoBtn->SetText("Redo (Ctrl+Y)");
redoBtn->OnClick([]() {
    editor.Redo();
});

vge::UIButton* saveBtn = editorPanel->AddChild<vge::UIButton>();
saveBtn->SetText("Save");
saveBtn->OnClick([]() {
    editor.Save();
});

vge::UIButton* loadBtn = editorPanel->AddChild<vge::UIButton>();
loadBtn->SetText("Load");
loadBtn->OnClick([]() {
    editor.Load();
});
```

## Gizmos

```cpp
// Draw gizmos in editor
editor.DrawGizmo(GizmoType::Translate, selectedEntity);
editor.DrawGizmo(GizmoType::Rotate, selectedEntity);
editor.DrawGizmo(GizmoType::Scale, selectedEntity);

// Grid
editor.DrawGrid(100, 1.0f);  // 100x100 grid, 1 unit spacing

// Selection box
editor.DrawSelectionBox(start, end);

// Measurement tool
editor.DrawMeasurement(start, end);
```

## Undo/Redo System

```cpp
// Editor automatically records actions for undo/redo
editor.PlaceBlock(pos, block);  // Recorded
editor.RemoveBlock(pos);         // Recorded
editor.MoveEntity(entity, pos);  // Recorded

// Undo last action
editor.Undo();

// Redo undone action
editor.Redo();

// Clear history
editor.ClearHistory();

// History limit
editor.SetMaxHistorySize(100);
```
