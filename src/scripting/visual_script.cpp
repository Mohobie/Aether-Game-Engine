#include "visual_script.h"
#include "core/logger.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace vge {

// ============================================
// Pin
// ============================================

bool Pin::CanConnectTo(const Pin& other) const {
    // Can't connect same direction
    if (direction == other.direction) return false;
    
    // Flow pins only connect to flow pins
    if (type == PinType::Flow || other.type == PinType::Flow) {
        return type == PinType::Flow && other.type == PinType::Flow;
    }
    
    // Any type can connect to anything
    if (type == PinType::Any || other.type == PinType::Any) return true;
    
    // Same type connects
    return type == other.type;
}

// ============================================
// VisualScriptNode
// ============================================

VisualScriptNode::VisualScriptNode()
    : id("")
    , type("Unknown")
    , displayName("Unknown")
    , category(NodeCategory::Custom)
    , x(0)
    , y(0)
    , executed(false)
{
}

Pin* VisualScriptNode::GetInputPin(const std::string& pinId) {
    for (auto& pin : inputPins) {
        if (pin.id == pinId) return &pin;
    }
    return nullptr;
}

Pin* VisualScriptNode::GetOutputPin(const std::string& pinId) {
    for (auto& pin : outputPins) {
        if (pin.id == pinId) return &pin;
    }
    return nullptr;
}

const Pin* VisualScriptNode::GetInputPin(const std::string& pinId) const {
    for (const auto& pin : inputPins) {
        if (pin.id == pinId) return &pin;
    }
    return nullptr;
}

const Pin* VisualScriptNode::GetOutputPin(const std::string& pinId) const {
    for (const auto& pin : outputPins) {
        if (pin.id == pinId) return &pin;
    }
    return nullptr;
}

bool VisualScriptNode::ConnectOutput(const std::string& outputPinId,
                                       VisualScriptNode& targetNode,
                                       const std::string& targetPinId) {
    Pin* outputPin = GetOutputPin(outputPinId);
    Pin* targetPin = targetNode.GetInputPin(targetPinId);
    
    if (!outputPin || !targetPin) return false;
    if (!outputPin->CanConnectTo(*targetPin)) return false;
    if (!outputPin->allowMultipleConnections && !outputPin->connections.empty()) return false;
    
    outputPin->connections.push_back(targetNode.id + ":" + targetPinId);
    targetPin->connections.push_back(id + ":" + outputPinId);
    
    return true;
}

bool VisualScriptNode::DisconnectOutput(const std::string& outputPinId,
                                          const std::string& targetPinId) {
    Pin* outputPin = GetOutputPin(outputPinId);
    if (!outputPin) return false;
    
    auto& conns = outputPin->connections;
    auto it = std::find(conns.begin(), conns.end(), targetPinId);
    if (it != conns.end()) {
        conns.erase(it);
        return true;
    }
    return false;
}

void VisualScriptNode::Execute(VisualScriptRuntime& runtime) {
    executed = true;
    runtime.LogExecution("Executed node: " + displayName);
}

void VisualScriptNode::Reset() {
    executed = false;
    runtimeValues.clear();
}

void VisualScriptNode::Serialize(std::ostream& out) const {
    // TODO: Implement serialization
}

void VisualScriptNode::Deserialize(std::istream& in) {
    // TODO: Implement deserialization
}

// ============================================
// Built-in Node Implementations
// ============================================

// Event: On Game Start
EventStartNode::EventStartNode() {
    type = "EventStart";
    displayName = "On Start";
    category = NodeCategory::Event;
    
    outputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Output));
}

void EventStartNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    // Trigger the exec output
    Pin* execPin = GetOutputPin("exec");
    if (execPin) {
        runtime.ExecuteFlow(execPin);
    }
}

// Event: On Tick
EventTickNode::EventTickNode() {
    type = "EventTick";
    displayName = "On Tick";
    category = NodeCategory::Event;
    
    outputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Output));
    outputPins.push_back(Pin("deltaTime", "Delta Time", PinType::Float, PinDirection::Output, 0.0f));
}

void EventTickNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    // Set delta time output
    SetOutputValue("deltaTime", runtime.GetDeltaTime());
    
    // Trigger exec
    Pin* execPin = GetOutputPin("exec");
    if (execPin) {
        runtime.ExecuteFlow(execPin);
    }
}

// Event: On Key Pressed
EventKeyNode::EventKeyNode() {
    type = "EventKey";
    displayName = "On Key Pressed";
    category = NodeCategory::Event;
    
    inputPins.push_back(Pin("key", "Key", PinType::String, PinDirection::Input, std::string("Space")));
    outputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Output));
    outputPins.push_back(Pin("pressed", "Pressed", PinType::Bool, PinDirection::Output, false));
}

void EventKeyNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    // TODO: Check actual key state
    std::string key = GetInputValue<std::string>("key", "Space");
    bool isPressed = false; // Would check input system
    
    SetOutputValue("pressed", isPressed);
    
    if (isPressed) {
        Pin* execPin = GetOutputPin("exec");
        if (execPin) {
            runtime.ExecuteFlow(execPin);
        }
    }
}

// Flow: Branch
BranchNode::BranchNode() {
    type = "Branch";
    displayName = "Branch";
    category = NodeCategory::Flow;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    inputPins.push_back(Pin("condition", "Condition", PinType::Bool, PinDirection::Input, false));
    outputPins.push_back(Pin("true", "True", PinType::Flow, PinDirection::Output));
    outputPins.push_back(Pin("false", "False", PinType::Flow, PinDirection::Output));
}

void BranchNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    bool condition = GetInputValue<bool>("condition", false);
    
    Pin* outPin = condition ? GetOutputPin("true") : GetOutputPin("false");
    if (outPin) {
        runtime.ExecuteFlow(outPin);
    }
}

// Flow: Sequence
SequenceNode::SequenceNode() {
    type = "Sequence";
    displayName = "Sequence";
    category = NodeCategory::Flow;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    outputPins.push_back(Pin("then", "Then", PinType::Flow, PinDirection::Output));
    outputPins.push_back(Pin("next", "Next", PinType::Flow, PinDirection::Output));
}

void SequenceNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    // Execute all outputs in order
    Pin* thenPin = GetOutputPin("then");
    Pin* nextPin = GetOutputPin("next");
    
    if (thenPin) runtime.ExecuteFlow(thenPin);
    if (nextPin) runtime.ExecuteFlow(nextPin);
}

// Flow: Delay
DelayNode::DelayNode() {
    type = "Delay";
    displayName = "Delay";
    category = NodeCategory::Flow;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    inputPins.push_back(Pin("duration", "Duration", PinType::Float, PinDirection::Input, 1.0f));
    outputPins.push_back(Pin("completed", "Completed", PinType::Flow, PinDirection::Output));
}

void DelayNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    float duration = GetInputValue<float>("duration", 1.0f);
    
    // TODO: Implement actual delay using timer
    // For now, execute immediately
    Pin* completedPin = GetOutputPin("completed");
    if (completedPin) {
        runtime.ExecuteFlow(completedPin);
    }
}

// Flow: For Loop
ForLoopNode::ForLoopNode() {
    type = "ForLoop";
    displayName = "For Loop";
    category = NodeCategory::Flow;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    inputPins.push_back(Pin("start", "Start", PinType::Int, PinDirection::Input, 0));
    inputPins.push_back(Pin("end", "End", PinType::Int, PinDirection::Input, 10));
    inputPins.push_back(Pin("step", "Step", PinType::Int, PinDirection::Input, 1));
    outputPins.push_back(Pin("loopBody", "Loop Body", PinType::Flow, PinDirection::Output));
    outputPins.push_back(Pin("index", "Index", PinType::Int, PinDirection::Output, 0));
    outputPins.push_back(Pin("completed", "Completed", PinType::Flow, PinDirection::Output));
}

void ForLoopNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    int start = GetInputValue<int>("start", 0);
    int end = GetInputValue<int>("end", 10);
    int step = GetInputValue<int>("step", 1);
    
    Pin* loopBodyPin = GetOutputPin("loopBody");
    
    for (int i = start; i < end; i += step) {
        SetOutputValue("index", i);
        if (loopBodyPin) {
            runtime.ExecuteFlow(loopBodyPin);
        }
    }
    
    Pin* completedPin = GetOutputPin("completed");
    if (completedPin) {
        runtime.ExecuteFlow(completedPin);
    }
}

// Math: Add
AddNode::AddNode() {
    type = "Add";
    displayName = "Add";
    category = NodeCategory::Math;
    
    inputPins.push_back(Pin("a", "A", PinType::Float, PinDirection::Input, 0.0f));
    inputPins.push_back(Pin("b", "B", PinType::Float, PinDirection::Input, 0.0f));
    outputPins.push_back(Pin("result", "Result", PinType::Float, PinDirection::Output, 0.0f));
}

void AddNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    float a = GetInputValue<float>("a", 0.0f);
    float b = GetInputValue<float>("b", 0.0f);
    
    SetOutputValue("result", a + b);
}

// Math: Multiply
MultiplyNode::MultiplyNode() {
    type = "Multiply";
    displayName = "Multiply";
    category = NodeCategory::Math;
    
    inputPins.push_back(Pin("a", "A", PinType::Float, PinDirection::Input, 1.0f));
    inputPins.push_back(Pin("b", "B", PinType::Float, PinDirection::Input, 1.0f));
    outputPins.push_back(Pin("result", "Result", PinType::Float, PinDirection::Output, 1.0f));
}

void MultiplyNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    float a = GetInputValue<float>("a", 1.0f);
    float b = GetInputValue<float>("b", 1.0f);
    
    SetOutputValue("result", a * b);
}

// Math: Vector Operations
VectorNode::VectorNode()
    : operation(Operation::Add)
{
    type = "Vector";
    displayName = "Vector";
    category = NodeCategory::Math;
    
    inputPins.push_back(Pin("a", "A", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)));
    inputPins.push_back(Pin("b", "B", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)));
    outputPins.push_back(Pin("result", "Result", PinType::Vec3, PinDirection::Output, Vec3(0, 0, 0)));
}

void VectorNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    Vec3 a = GetInputValue<Vec3>("a", Vec3(0, 0, 0));
    Vec3 b = GetInputValue<Vec3>("b", Vec3(0, 0, 0));
    Vec3 result;
    
    switch (operation) {
        case Operation::Add: result = a + b; break;
        case Operation::Subtract: result = a - b; break;
        case Operation::Multiply: result = Vec3(a.x * b.x, a.y * b.y, a.z * b.z); break;
        case Operation::Divide: result = Vec3(a.x / b.x, a.y / b.y, a.z / b.z); break;
        case Operation::Dot: result = Vec3(a.Dot(b), 0, 0); break;
        case Operation::Cross: result = a.Cross(b); break;
        case Operation::Normalize: result = a.Normalized(); break;
        case Operation::Length: result = Vec3(a.Length(), 0, 0); break;
    }
    
    SetOutputValue("result", result);
}

// Logic: Compare
CompareNode::CompareNode()
    : operation(Operation::Equal)
{
    type = "Compare";
    displayName = "Compare";
    category = NodeCategory::Logic;
    
    inputPins.push_back(Pin("a", "A", PinType::Float, PinDirection::Input, 0.0f));
    inputPins.push_back(Pin("b", "B", PinType::Float, PinDirection::Input, 0.0f));
    outputPins.push_back(Pin("result", "Result", PinType::Bool, PinDirection::Output, false));
}

void CompareNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    float a = GetInputValue<float>("a", 0.0f);
    float b = GetInputValue<float>("b", 0.0f);
    bool result = false;
    
    switch (operation) {
        case Operation::Equal: result = (a == b); break;
        case Operation::NotEqual: result = (a != b); break;
        case Operation::Less: result = (a < b); break;
        case Operation::LessEqual: result = (a <= b); break;
        case Operation::Greater: result = (a > b); break;
        case Operation::GreaterEqual: result = (a >= b); break;
    }
    
    SetOutputValue("result", result);
}

// Logic: Boolean
BooleanNode::BooleanNode()
    : operation(Operation::And)
{
    type = "Boolean";
    displayName = "Boolean";
    category = NodeCategory::Logic;
    
    inputPins.push_back(Pin("a", "A", PinType::Bool, PinDirection::Input, false));
    inputPins.push_back(Pin("b", "B", PinType::Bool, PinDirection::Input, false));
    outputPins.push_back(Pin("result", "Result", PinType::Bool, PinDirection::Output, false));
}

void BooleanNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    bool a = GetInputValue<bool>("a", false);
    bool b = GetInputValue<bool>("b", false);
    bool result = false;
    
    switch (operation) {
        case Operation::And: result = a && b; break;
        case Operation::Or: result = a || b; break;
        case Operation::Not: result = !a; break;
        case Operation::Xor: result = a != b; break;
    }
    
    SetOutputValue("result", result);
}

// Variable: Get
GetVariableNode::GetVariableNode()
    : variableName("")
{
    type = "GetVariable";
    displayName = "Get Variable";
    category = NodeCategory::Variable;
    
    inputPins.push_back(Pin("name", "Name", PinType::String, PinDirection::Input, std::string("")));
    outputPins.push_back(Pin("value", "Value", PinType::Any, PinDirection::Output));
}

void GetVariableNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    std::string name = GetInputValue<std::string>("name", variableName);
    if (name.empty()) return;
    
    // TODO: Get actual variable value from runtime
    // For now, just log
    runtime.LogExecution("Get variable: " + name);
}

// Variable: Set
SetVariableNode::SetVariableNode()
    : variableName("")
{
    type = "SetVariable";
    displayName = "Set Variable";
    category = NodeCategory::Variable;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    inputPins.push_back(Pin("name", "Name", PinType::String, PinDirection::Input, std::string("")));
    inputPins.push_back(Pin("value", "Value", PinType::Any, PinDirection::Input));
    outputPins.push_back(Pin("then", "Then", PinType::Flow, PinDirection::Output));
}

void SetVariableNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    std::string name = GetInputValue<std::string>("name", variableName);
    if (name.empty()) return;
    
    // TODO: Set actual variable value in runtime
    runtime.LogExecution("Set variable: " + name);
    
    Pin* thenPin = GetOutputPin("then");
    if (thenPin) {
        runtime.ExecuteFlow(thenPin);
    }
}

// Entity: Get Position
GetPositionNode::GetPositionNode() {
    type = "GetPosition";
    displayName = "Get Position";
    category = NodeCategory::Entity;
    
    inputPins.push_back(Pin("entity", "Entity", PinType::Entity, PinDirection::Input));
    outputPins.push_back(Pin("position", "Position", PinType::Vec3, PinDirection::Output, Vec3(0, 0, 0)));
}

void GetPositionNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    // TODO: Get actual entity position
    SetOutputValue("position", Vec3(0, 0, 0));
}

// Entity: Set Position
SetPositionNode::SetPositionNode() {
    type = "SetPosition";
    displayName = "Set Position";
    category = NodeCategory::Entity;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    inputPins.push_back(Pin("entity", "Entity", PinType::Entity, PinDirection::Input));
    inputPins.push_back(Pin("position", "Position", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)));
    outputPins.push_back(Pin("then", "Then", PinType::Flow, PinDirection::Output));
}

void SetPositionNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    Vec3 pos = GetInputValue<Vec3>("position", Vec3(0, 0, 0));
    
    // TODO: Set actual entity position
    runtime.LogExecution("Set position to (" + std::to_string(pos.x) + ", " + 
                        std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
    
    Pin* thenPin = GetOutputPin("then");
    if (thenPin) {
        runtime.ExecuteFlow(thenPin);
    }
}

// World: Raycast
RaycastNode::RaycastNode() {
    type = "Raycast";
    displayName = "Raycast";
    category = NodeCategory::World;
    
    inputPins.push_back(Pin("origin", "Origin", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)));
    inputPins.push_back(Pin("direction", "Direction", PinType::Vec3, PinDirection::Input, Vec3(0, -1, 0)));
    inputPins.push_back(Pin("distance", "Distance", PinType::Float, PinDirection::Input, 100.0f));
    outputPins.push_back(Pin("hit", "Hit", PinType::Bool, PinDirection::Output, false));
    outputPins.push_back(Pin("hitPoint", "Hit Point", PinType::Vec3, PinDirection::Output, Vec3(0, 0, 0)));
    outputPins.push_back(Pin("hitNormal", "Hit Normal", PinType::Vec3, PinDirection::Output, Vec3(0, 1, 0)));
}

void RaycastNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    Vec3 origin = GetInputValue<Vec3>("origin", Vec3(0, 0, 0));
    Vec3 direction = GetInputValue<Vec3>("direction", Vec3(0, -1, 0));
    float distance = GetInputValue<float>("distance", 100.0f);
    
    // TODO: Perform actual raycast
    bool hit = false;
    Vec3 hitPoint = origin + direction * distance;
    Vec3 hitNormal = Vec3(0, 1, 0);
    
    SetOutputValue("hit", hit);
    SetOutputValue("hitPoint", hitPoint);
    SetOutputValue("hitNormal", hitNormal);
}

// Rendering: Spawn Particle
SpawnParticleNode::SpawnParticleNode() {
    type = "SpawnParticle";
    displayName = "Spawn Particle";
    category = NodeCategory::Rendering;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    inputPins.push_back(Pin("position", "Position", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)));
    inputPins.push_back(Pin("color", "Color", PinType::Color, PinDirection::Input));
    inputPins.push_back(Pin("count", "Count", PinType::Int, PinDirection::Input, 10));
    outputPins.push_back(Pin("then", "Then", PinType::Flow, PinDirection::Output));
}

void SpawnParticleNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    Vec3 pos = GetInputValue<Vec3>("position", Vec3(0, 0, 0));
    int count = GetInputValue<int>("count", 10);
    
    // TODO: Spawn actual particles
    runtime.LogExecution("Spawned " + std::to_string(count) + " particles at (" +
                        std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + 
                        std::to_string(pos.z) + ")");
    
    Pin* thenPin = GetOutputPin("then");
    if (thenPin) {
        runtime.ExecuteFlow(thenPin);
    }
}

// Audio: Play Sound
PlaySoundNode::PlaySoundNode() {
    type = "PlaySound";
    displayName = "Play Sound";
    category = NodeCategory::Audio;
    
    inputPins.push_back(Pin("exec", "Exec", PinType::Flow, PinDirection::Input));
    inputPins.push_back(Pin("sound", "Sound", PinType::Asset, PinDirection::Input));
    inputPins.push_back(Pin("volume", "Volume", PinType::Float, PinDirection::Input, 1.0f));
    inputPins.push_back(Pin("position", "Position", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)));
    outputPins.push_back(Pin("then", "Then", PinType::Flow, PinDirection::Output));
}

void PlaySoundNode::Execute(VisualScriptRuntime& runtime) {
    VisualScriptNode::Execute(runtime);
    
    float volume = GetInputValue<float>("volume", 1.0f);
    Vec3 pos = GetInputValue<Vec3>("position", Vec3(0, 0, 0));
    
    // TODO: Play actual sound
    runtime.LogExecution("Played sound at volume " + std::to_string(volume));
    
    Pin* thenPin = GetOutputPin("then");
    if (thenPin) {
        runtime.ExecuteFlow(thenPin);
    }
}

// ============================================
// VisualScriptGraph
// ============================================

VisualScriptGraph::VisualScriptGraph()
    : name("New Graph")
    , id("")
{
}

VisualScriptGraph::VisualScriptGraph(const std::string& name_)
    : name(name_)
    , id("")
{
}

VisualScriptGraph::~VisualScriptGraph() = default;

std::shared_ptr<VisualScriptNode> VisualScriptGraph::CreateNode(const std::string& typeName) {
    auto node = NodeRegistry::GetInstance().CreateNode(typeName);
    if (node) {
        node->id = name + "_" + typeName + "_" + std::to_string(nodes.size());
        nodes.push_back(node);
        nodeMap[node->id] = node;
    }
    return node;
}

void VisualScriptGraph::RemoveNode(const std::string& nodeId) {
    auto it = nodeMap.find(nodeId);
    if (it != nodeMap.end()) {
        // Remove connections to this node
        for (auto& node : nodes) {
            for (auto& pin : node->outputPins) {
                auto connIt = std::remove_if(pin.connections.begin(), pin.connections.end(),
                    [&nodeId](const std::string& conn) {
                        return conn.find(nodeId) == 0;
                    });
                pin.connections.erase(connIt, pin.connections.end());
            }
        }
        
        // Remove from lists
        auto listIt = std::find_if(nodes.begin(), nodes.end(),
            [&nodeId](const std::shared_ptr<VisualScriptNode>& n) {
                return n->id == nodeId;
            });
        if (listIt != nodes.end()) {
            nodes.erase(listIt);
        }
        nodeMap.erase(it);
    }
}

std::shared_ptr<VisualScriptNode> VisualScriptGraph::GetNode(const std::string& nodeId) {
    auto it = nodeMap.find(nodeId);
    if (it != nodeMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool VisualScriptGraph::ConnectNodes(const std::string& fromNodeId, const std::string& fromPinId,
                                      const std::string& toNodeId, const std::string& toPinId) {
    auto fromNode = GetNode(fromNodeId);
    auto toNode = GetNode(toNodeId);
    
    if (!fromNode || !toNode) return false;
    
    return fromNode->ConnectOutput(fromPinId, *toNode, toPinId);
}

bool VisualScriptGraph::DisconnectNodes(const std::string& fromNodeId, const std::string& fromPinId,
                                         const std::string& toNodeId, const std::string& toPinId) {
    auto fromNode = GetNode(fromNodeId);
    if (!fromNode) return false;
    
    return fromNode->DisconnectOutput(fromPinId, toNodeId + ":" + toPinId);
}

std::vector<std::shared_ptr<VisualScriptNode>> VisualScriptGraph::GetEventNodes() const {
    std::vector<std::shared_ptr<VisualScriptNode>> events;
    for (const auto& node : nodes) {
        if (node->category == NodeCategory::Event) {
            events.push_back(node);
        }
    }
    return events;
}

void VisualScriptGraph::Save(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << SerializeToJson();
        file.close();
    }
}

void VisualScriptGraph::Load(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        DeserializeFromJson(buffer.str());
        file.close();
    }
}

std::string VisualScriptGraph::SerializeToJson() const {
    // TODO: Implement JSON serialization
    return "{}";
}

void VisualScriptGraph::DeserializeFromJson(const std::string& json) {
    // TODO: Implement JSON deserialization
}

bool VisualScriptGraph::Validate(std::vector<std::string>& errors) const {
    bool valid = true;
    
    for (const auto& node : nodes) {
        // Check for disconnected required inputs
        for (const auto& pin : node->inputPins) {
            if (pin.type != PinType::Flow && !pin.IsConnected() && !pin.defaultValue.has_value()) {
                errors.push_back("Node '" + node->displayName + "' has disconnected input '" + pin.name + "'");
                valid = false;
            }
        }
    }
    
    return valid;
}

// ============================================
// VisualScriptRuntime
// ============================================

VisualScriptRuntime::VisualScriptRuntime()
    : graph(nullptr)
    , isRunning(false)
    , deltaTime(0)
    , debugMode(false)
{
}

VisualScriptRuntime::VisualScriptRuntime(VisualScriptGraph* graph_)
    : graph(graph_)
    , isRunning(false)
    , deltaTime(0)
    , debugMode(false)
{
}

VisualScriptRuntime::~VisualScriptRuntime() = default;

void VisualScriptRuntime::SetGraph(VisualScriptGraph* graph_) {
    graph = graph_;
}

void VisualScriptRuntime::Start() {
    if (!graph) return;
    
    isRunning = true;
    executionLog.clear();
    
    // Execute start events
    auto eventNodes = graph->GetEventNodes();
    for (const auto& node : eventNodes) {
        if (node->type == "EventStart") {
            ExecuteNode(*node);
        }
    }
    
    Logger::Info("[VisualScript] Started graph: " + graph->GetName());
}

void VisualScriptRuntime::Stop() {
    isRunning = false;
    Logger::Info("[VisualScript] Stopped graph");
}

void VisualScriptRuntime::Update(float dt) {
    if (!isRunning || !graph) return;
    
    deltaTime = dt;
    
    // Execute tick events
    auto eventNodes = graph->GetEventNodes();
    for (const auto& node : eventNodes) {
        if (node->type == "EventTick") {
            ExecuteNode(*node);
        }
    }
}

void VisualScriptRuntime::TriggerEvent(const std::string& eventName) {
    if (!graph) return;
    
    auto eventNodes = graph->GetEventNodes();
    for (const auto& node : eventNodes) {
        if (node->type == eventName) {
            ExecuteNode(*node);
        }
    }
}

void VisualScriptRuntime::ExecuteNode(VisualScriptNode& node) {
    node.Execute(*this);
}

void VisualScriptRuntime::ExecuteFlow(Pin* outputPin) {
    if (!outputPin || outputPin->type != PinType::Flow) return;
    
    // Follow connections
    for (const auto& conn : outputPin->connections) {
        // Parse connection: "nodeId:pinId"
        size_t sep = conn.find(':');
        if (sep != std::string::npos) {
            std::string nodeId = conn.substr(0, sep);
            std::string pinId = conn.substr(sep + 1);
            
            auto node = graph->GetNode(nodeId);
            if (node) {
                Pin* inputPin = node->GetInputPin(pinId);
                if (inputPin && inputPin->type == PinType::Flow) {
                    ExecuteNode(*node);
                }
            }
        }
    }
}

void VisualScriptRuntime::LogExecution(const std::string& message) {
    if (debugMode) {
        executionLog.push_back(message);
        Logger::Debug("[VisualScript] " + message);
    }
}

// ============================================
//// NodeRegistry
// ============================================

NodeRegistry& NodeRegistry::GetInstance() {
    static NodeRegistry instance;
    return instance;
}

void NodeRegistry::RegisterNode(const NodeDefinition& def) {
    definitions[def.typeName] = def;
}

bool NodeRegistry::HasNode(const std::string& typeName) const {
    return definitions.find(typeName) != definitions.end();
}

NodeDefinition NodeRegistry::GetDefinition(const std::string& typeName) const {
    auto it = definitions.find(typeName);
    if (it != definitions.end()) {
        return it->second;
    }
    return NodeDefinition();
}

std::vector<NodeDefinition> NodeRegistry::GetDefinitionsByCategory(NodeCategory category) const {
    std::vector<NodeDefinition> result;
    for (const auto& pair : definitions) {
        if (pair.second.category == category) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<NodeDefinition> NodeRegistry::GetAllDefinitions() const {
    std::vector<NodeDefinition> result;
    for (const auto& pair : definitions) {
        result.push_back(pair.second);
    }
    return result;
}

std::shared_ptr<VisualScriptNode> NodeRegistry::CreateNode(const std::string& typeName) {
    auto it = definitions.find(typeName);
    if (it != definitions.end() && it->second.factory) {
        return it->second.factory();
    }
    
    // Fallback: try to create by type name
    if (typeName == "EventStart") return std::make_shared<EventStartNode>();
    if (typeName == "EventTick") return std::make_shared<EventTickNode>();
    if (typeName == "EventKey") return std::make_shared<EventKeyNode>();
    if (typeName == "Branch") return std::make_shared<BranchNode>();
    if (typeName == "Sequence") return std::make_shared<SequenceNode>();
    if (typeName == "Delay") return std::make_shared<DelayNode>();
    if (typeName == "ForLoop") return std::make_shared<ForLoopNode>();
    if (typeName == "Add") return std::make_shared<AddNode>();
    if (typeName == "Multiply") return std::make_shared<MultiplyNode>();
    if (typeName == "Vector") return std::make_shared<VectorNode>();
    if (typeName == "Compare") return std::make_shared<CompareNode>();
    if (typeName == "Boolean") return std::make_shared<BooleanNode>();
    if (typeName == "GetVariable") return std::make_shared<GetVariableNode>();
    if (typeName == "SetVariable") return std::make_shared<SetVariableNode>();
    if (typeName == "GetPosition") return std::make_shared<GetPositionNode>();
    if (typeName == "SetPosition") return std::make_shared<SetPositionNode>();
    if (typeName == "Raycast") return std::make_shared<RaycastNode>();
    if (typeName == "SpawnParticle") return std::make_shared<SpawnParticleNode>();
    if (typeName == "PlaySound") return std::make_shared<PlaySoundNode>();
    
    return nullptr;
}

void NodeRegistry::RegisterBuiltInNodes() {
    // Events
    RegisterNode({"EventStart", "On Start", "Called when the game starts", NodeCategory::Event, "▶️",
        {}, {{"exec", "Exec", PinType::Flow, PinDirection::Output}}, {},
        []() { return std::make_shared<EventStartNode>(); }});
    
    RegisterNode({"EventTick", "On Tick", "Called every frame", NodeCategory::Event, "🔄",
        {}, {{"exec", "Exec", PinType::Flow, PinDirection::Output}, {"deltaTime", "Delta Time", PinType::Float, PinDirection::Output, 0.0f}}, {},
        []() { return std::make_shared<EventTickNode>(); }});
    
    // Flow
    RegisterNode({"Branch", "Branch", "If/else branch", NodeCategory::Flow, "🔀",
        {{"exec", "Exec", PinType::Flow, PinDirection::Input}, {"condition", "Condition", PinType::Bool, PinDirection::Input, false}},
        {{"true", "True", PinType::Flow, PinDirection::Output}, {"false", "False", PinType::Flow, PinDirection::Output}}, {},
        []() { return std::make_shared<BranchNode>(); }});
    
    RegisterNode({"Sequence", "Sequence", "Execute in sequence", NodeCategory::Flow, "➡️",
        {{"exec", "Exec", PinType::Flow, PinDirection::Input}},
        {{"then", "Then", PinType::Flow, PinDirection::Output}, {"next", "Next", PinType::Flow, PinDirection::Output}}, {},
        []() { return std::make_shared<SequenceNode>(); }});
    
    // Math
    RegisterNode({"Add", "Add", "Add two numbers", NodeCategory::Math, "➕",
        {{"a", "A", PinType::Float, PinDirection::Input, 0.0f}, {"b", "B", PinType::Float, PinDirection::Input, 0.0f}},
        {{"result", "Result", PinType::Float, PinDirection::Output, 0.0f}}, {},
        []() { return std::make_shared<AddNode>(); }});
    
    RegisterNode({"Multiply", "Multiply", "Multiply two numbers", NodeCategory::Math, "✖️",
        {{"a", "A", PinType::Float, PinDirection::Input, 1.0f}, {"b", "B", PinType::Float, PinDirection::Input, 1.0f}},
        {{"result", "Result", PinType::Float, PinDirection::Output, 1.0f}}, {},
        []() { return std::make_shared<MultiplyNode>(); }});
    
    // Logic
    RegisterNode({"Compare", "Compare", "Compare two values", NodeCategory::Logic, "⚖️",
        {{"a", "A", PinType::Float, PinDirection::Input, 0.0f}, {"b", "B", PinType::Float, PinDirection::Input, 0.0f}},
        {{"result", "Result", PinType::Bool, PinDirection::Output, false}}, {},
        []() { return std::make_shared<CompareNode>(); }});
    
    // Entity
    RegisterNode({"GetPosition", "Get Position", "Get entity position", NodeCategory::Entity, "📍",
        {{"entity", "Entity", PinType::Entity, PinDirection::Input}},
        {{"position", "Position", PinType::Vec3, PinDirection::Output, Vec3(0, 0, 0)}}, {},
        []() { return std::make_shared<GetPositionNode>(); }});
    
    RegisterNode({"SetPosition", "Set Position", "Set entity position", NodeCategory::Entity, "📍",
        {{"exec", "Exec", PinType::Flow, PinDirection::Input}, {"entity", "Entity", PinType::Entity, PinDirection::Input}, {"position", "Position", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)}},
        {{"then", "Then", PinType::Flow, PinDirection::Output}}, {},
        []() { return std::make_shared<SetPositionNode>(); }});
    
    // Rendering
    RegisterNode({"SpawnParticle", "Spawn Particle", "Spawn particles at location", NodeCategory::Rendering, "✨",
        {{"exec", "Exec", PinType::Flow, PinDirection::Input}, {"position", "Position", PinType::Vec3, PinDirection::Input, Vec3(0, 0, 0)}, {"count", "Count", PinType::Int, PinDirection::Input, 10}},
        {{"then", "Then", PinType::Flow, PinDirection::Output}}, {},
        []() { return std::make_shared<SpawnParticleNode>(); }});
    
    // Audio
    RegisterNode({"PlaySound", "Play Sound", "Play a sound", NodeCategory::Audio, "🔊",
        {{"exec", "Exec", PinType::Flow, PinDirection::Input}, {"sound", "Sound", PinType::Asset, PinDirection::Input}, {"volume", "Volume", PinType::Float, PinDirection::Input, 1.0f}},
        {{"then", "Then", PinType::Flow, PinDirection::Output}}, {},
        []() { return std::make_shared<PlaySoundNode>(); }});
    
    Logger::Info("[NodeRegistry] Registered " + std::to_string(definitions.size()) + " built-in nodes");
}

// ============================================
// VisualScriptComponent
// ============================================

VisualScriptComponent::VisualScriptComponent()
    : graph(nullptr)
    , runtime(nullptr)
    , autoStart(true)
{
}

VisualScriptComponent::VisualScriptComponent(std::shared_ptr<VisualScriptGraph> graph_)
    : graph(graph_)
    , autoStart(true)
{
    if (graph) {
        runtime = std::make_unique<VisualScriptRuntime>(graph.get());
    }
}

void VisualScriptComponent::Start() {
    if (runtime) {
        runtime->Start();
    }
}

void VisualScriptComponent::Stop() {
    if (runtime) {
        runtime->Stop();
    }
}

void VisualScriptComponent::Update(float dt) {
    if (runtime && runtime->IsRunning()) {
        runtime->Update(dt);
    }
}

} // namespace vge
