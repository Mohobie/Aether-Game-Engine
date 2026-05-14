# Visual Scripting System

## Overview

The Visual Scripting System provides a node-based programming interface for non-programmers to create game logic. Similar to Unreal Blueprints or Unity Visual Scripting, it allows connecting nodes with visual wires to define behavior.

## Architecture

### Core Components

| Component | Description |
|-----------|-------------|
| **VisualScriptNode** | Base class for all nodes |
| **VisualScriptGraph** | Container for nodes and connections |
| **VisualScriptRuntime** | Executes graphs at runtime |
| **NodeRegistry** | Factory for creating nodes by type |
| **Pin** | Connection points on nodes |

### Pin Types

| Type | Color | Description |
|------|-------|-------------|
| **Flow** | White | Execution flow |
| **Bool** | Red | Boolean value |
| **Int** | Green | Integer |
| **Float** | Blue | Floating point |
| **String** | Pink | Text |
| **Vec3** | Yellow | 3D vector |
| **Color** | Purple | RGBA color |
| **Entity** | Orange | Entity reference |
| **Asset** | Cyan | Asset reference |
| **Any** | Gray | Dynamic type |

## Built-in Nodes

### Events

| Node | Description | Outputs |
|------|-------------|---------|
| **On Start** | Called when game starts | Exec |
| **On Tick** | Called every frame | Exec, Delta Time |
| **On Key Pressed** | Called when key pressed | Exec, Pressed |

### Flow Control

| Node | Description | Inputs | Outputs |
|------|-------------|--------|---------|
| **Branch** | If/else | Exec, Condition | True, False |
| **Sequence** | Execute in order | Exec | Then, Next |
| **Delay** | Wait N seconds | Exec, Duration | Completed |
| **For Loop** | Loop N times | Exec, Start, End, Step | Loop Body, Index, Completed |

### Math

| Node | Description | Inputs | Outputs |
|------|-------------|--------|---------|
| **Add** | A + B | A, B | Result |
| **Multiply** | A * B | A, B | Result |
| **Vector** | Vector operations | A, B | Result |

### Logic

| Node | Description | Inputs | Outputs |
|------|-------------|--------|---------|
| **Compare** | Compare values | A, B | Result |
| **Boolean** | AND/OR/NOT | A, B | Result |

### Entity

| Node | Description | Inputs | Outputs |
|------|-------------|--------|---------|
| **Get Position** | Get entity position | Entity | Position |
| **Set Position** | Set entity position | Exec, Entity, Position | Then |

### World

| Node | Description | Inputs | Outputs |
|------|-------------|--------|---------|
| **Raycast** | Cast a ray | Origin, Direction, Distance | Hit, Hit Point, Hit Normal |

### Rendering

| Node | Description | Inputs | Outputs |
|------|-------------|--------|---------|
| **Spawn Particle** | Spawn particles | Exec, Position, Color, Count | Then |

### Audio

| Node | Description | Inputs | Outputs |
|------|-------------|--------|---------|
| **Play Sound** | Play sound effect | Exec, Sound, Volume, Position | Then |

## Usage Example

```cpp
#include "scripting/visual_script.h"

// Create a graph
auto graph = std::make_shared<vge::VisualScriptGraph>("PlayerController");

// Create nodes
auto startNode = graph->CreateNode("EventStart");
auto tickNode = graph->CreateNode("EventTick");
auto getPosNode = graph->CreateNode("GetPosition");
auto setPosNode = graph->CreateNode("SetPosition");
auto addNode = graph->CreateNode("Add");

// Connect nodes
graph->ConnectNodes(tickNode->id, "exec", getPosNode->id, "entity");
graph->ConnectNodes(getPosNode->id, "position", addNode->id, "a");
graph->ConnectNodes(addNode->id, "result", setPosNode->id, "position");

// Create runtime and execute
vge::VisualScriptRuntime runtime(graph.get());
runtime.Start();

// In game loop
runtime.Update(deltaTime);
```

## Creating Custom Nodes

```cpp
class MyCustomNode : public vge::VisualScriptNode {
public:
    MyCustomNode() {
        type = "MyCustom";
        displayName = "My Custom Node";
        category = NodeCategory::Custom;
        
        inputPins.push_back(Pin("input", "Input", PinType::Float, PinDirection::Input, 0.0f));
        outputPins.push_back(Pin("output", "Output", PinType::Float, PinDirection::Output, 0.0f));
    }
    
    void Execute(VisualScriptRuntime& runtime) override {
        float input = GetInputValue<float>("input", 0.0f);
        float result = input * 2.0f; // Custom logic
        SetOutputValue("output", result);
    }
};

// Register
NodeRegistry::GetInstance().RegisterNode({
    "MyCustom", "My Custom Node", "Does something custom",
    NodeCategory::Custom, "🔧",
    {{"input", "Input", PinType::Float, PinDirection::Input, 0.0f}},
    {{"output", "Output", PinType::Float, PinDirection::Output, 0.0f}},
    {},
    []() { return std::make_shared<MyCustomNode>(); }
});
```

## Integration with ECS

```cpp
// Add visual script component to entity
auto scriptComp = entity->AddComponent<VisualScriptComponent>();
scriptComp->graph = graph;
scriptComp->autoStart = true;

// System updates
void VisualScriptSystem::Update(float dt) {
    for (auto& comp : visualScriptComponents) {
        if (comp->autoStart && !comp->runtime->IsRunning()) {
            comp->Start();
        }
        comp->Update(dt);
    }
}
```

## Performance Considerations

- Graphs are compiled to an intermediate representation
- Only event nodes trigger execution
- Data flows only when inputs change
- Consider using C++ for performance-critical code

## Future Improvements

- [ ] JSON serialization/deserialization
- [ ] Visual editor UI
- [ ] Debug visualization
- [ ] Breakpoints and step-through
- [ ] Variable watch window
- [ ] Hot reload
