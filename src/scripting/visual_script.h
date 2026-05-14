#pragma once

#include "math/vec3.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <any>

namespace vge {

// ============================================
// Visual Scripting System
// ============================================
// Node-based visual scripting for non-programmers
// Similar to Unreal Blueprints or Unity Visual Scripting

// Forward declarations
class VisualScriptNode;
class VisualScriptGraph;
class VisualScriptRuntime;

// ============================================
// Pin Types
// ============================================

enum class PinType {
    Flow,       // Execution flow (white lines)
    Bool,       // Boolean value
    Int,        // Integer
    Float,      // Floating point
    String,     // Text
    Vec3,       // 3D vector
    Color,      // RGBA color
    Entity,     // Entity reference
    Asset,      // Asset reference
    Any         // Dynamic type
};

enum class PinDirection {
    Input,
    Output
};

// ============================================
// Pin
// ============================================

struct Pin {
    std::string id;
    std::string name;
    PinType type;
    PinDirection direction;
    std::any defaultValue;
    bool allowMultipleConnections;
    
    // Connected pins (pin IDs)
    std::vector<std::string> connections;
    
    Pin() = default;
    Pin(const std::string& id_, const std::string& name_, 
        PinType type_, PinDirection dir_, 
        const std::any& default_ = std::any())
        : id(id_)
        , name(name_)
        , type(type_)
        , direction(dir_)
        , defaultValue(default_)
        , allowMultipleConnections(false)
    {}
    
    bool IsConnected() const { return !connections.empty(); }
    bool CanConnectTo(const Pin& other) const;
};

// ============================================
// Node Types
// ============================================

enum class NodeCategory {
    Flow,       // Branch, sequence, delay
    Math,       // Arithmetic, vector math
    Logic,      // Comparisons, boolean ops
    Variable,   // Get/set variables
    Event,      // Game events (on start, on tick)
    Entity,     // Entity operations
    World,      // World queries, raycast
    Input,      // Key press, mouse
    Rendering,  // Spawn particle, play animation
    Audio,      // Play sound
    Custom      // User-defined nodes
};

// ============================================
// Node Definition
// ============================================

struct NodeDefinition {
    std::string typeName;
    std::string displayName;
    std::string description;
    NodeCategory category;
    std::string icon;  // Emoji or icon name
    
    // Pin definitions
    std::vector<Pin> inputPins;
    std::vector<Pin> outputPins;
    
    // Custom properties
    std::unordered_map<std::string, std::string> properties;
    
    // Factory function
    std::function<std::shared_ptr<VisualScriptNode>()> factory;
};

// ============================================
// Visual Script Node
// ============================================

class VisualScriptNode {
public:
    std::string id;
    std::string type;
    std::string displayName;
    NodeCategory category;
    
    // Position in graph (for editor)
    float x, y;
    
    // Pins
    std::vector<Pin> inputPins;
    std::vector<Pin> outputPins;
    
    // Custom properties
    std::unordered_map<std::string, std::any> properties;
    
    // Runtime state
    bool executed;
    std::unordered_map<std::string, std::any> runtimeValues;
    
public:
    VisualScriptNode();
    virtual ~VisualScriptNode() = default;
    
    // Pin access
    Pin* GetInputPin(const std::string& pinId);
    Pin* GetOutputPin(const std::string& pinId);
    const Pin* GetInputPin(const std::string& pinId) const;
    const Pin* GetOutputPin(const std::string& pinId) const;
    
    // Connection
    bool ConnectOutput(const std::string& outputPinId, 
                       VisualScriptNode& targetNode, 
                       const std::string& targetPinId);
    bool DisconnectOutput(const std::string& outputPinId,
                          const std::string& targetPinId);
    
    // Value access
    template<typename T>
    T GetInputValue(const std::string& pinId, const T& defaultValue = T{}) const;
    
    template<typename T>
    void SetOutputValue(const std::string& pinId, const T& value);
    
    // Execution
    virtual void Execute(VisualScriptRuntime& runtime);
    virtual void Reset();
    
    // Serialization
    virtual void Serialize(std::ostream& out) const;
    virtual void Deserialize(std::istream& in);
};

// ============================================
// Built-in Node Types
// ============================================

// Event: On Game Start
class EventStartNode : public VisualScriptNode {
public:
    EventStartNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Event: On Tick (Update)
class EventTickNode : public VisualScriptNode {
public:
    EventTickNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Event: On Key Pressed
class EventKeyNode : public VisualScriptNode {
public:
    EventKeyNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Flow: Branch (if/else)
class BranchNode : public VisualScriptNode {
public:
    BranchNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Flow: Sequence
class SequenceNode : public VisualScriptNode {
public:
    SequenceNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Flow: Delay
class DelayNode : public VisualScriptNode {
public:
    DelayNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Flow: For Loop
class ForLoopNode : public VisualScriptNode {
public:
    ForLoopNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Math: Add
class AddNode : public VisualScriptNode {
public:
    AddNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Math: Multiply
class MultiplyNode : public VisualScriptNode {
public:
    MultiplyNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Math: Vector Operations
class VectorNode : public VisualScriptNode {
public:
    enum class Operation { Add, Subtract, Multiply, Divide, Dot, Cross, Normalize, Length };
    
    Operation operation;
    
    VectorNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Logic: Compare
class CompareNode : public VisualScriptNode {
public:
    enum class Operation { Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual };
    
    Operation operation;
    
    CompareNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Logic: And/Or/Not
class BooleanNode : public VisualScriptNode {
public:
    enum class Operation { And, Or, Not, Xor };
    
    Operation operation;
    
    BooleanNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Variable: Get
class GetVariableNode : public VisualScriptNode {
public:
    std::string variableName;
    
    GetVariableNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Variable: Set
class SetVariableNode : public VisualScriptNode {
public:
    std::string variableName;
    
    SetVariableNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Entity: Get Position
class GetPositionNode : public VisualScriptNode {
public:
    GetPositionNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Entity: Set Position
class SetPositionNode : public VisualScriptNode {
public:
    SetPositionNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// World: Raycast
class RaycastNode : public VisualScriptNode {
public:
    RaycastNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Rendering: Spawn Particle
class SpawnParticleNode : public VisualScriptNode {
public:
    SpawnParticleNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// Audio: Play Sound
class PlaySoundNode : public VisualScriptNode {
public:
    PlaySoundNode();
    void Execute(VisualScriptRuntime& runtime) override;
};

// ============================================
// Visual Script Graph
// ============================================

class VisualScriptGraph {
private:
    std::string name;
    std::string id;
    std::vector<std::shared_ptr<VisualScriptNode>> nodes;
    std::unordered_map<std::string, std::shared_ptr<VisualScriptNode>> nodeMap;
    
    // Variables
    std::unordered_map<std::string, std::any> variables;
    
public:
    VisualScriptGraph();
    explicit VisualScriptGraph(const std::string& name_);
    ~VisualScriptGraph();
    
    // Node management
    std::shared_ptr<VisualScriptNode> CreateNode(const std::string& typeName);
    void RemoveNode(const std::string& nodeId);
    std::shared_ptr<VisualScriptNode> GetNode(const std::string& nodeId);
    
    // Connection
    bool ConnectNodes(const std::string& fromNodeId, const std::string& fromPinId,
                      const std::string& toNodeId, const std::string& toPinId);
    bool DisconnectNodes(const std::string& fromNodeId, const std::string& fromPinId,
                         const std::string& toNodeId, const std::string& toPinId);
    
    // Variable management
    template<typename T>
    void SetVariable(const std::string& name, const T& value);
    
    template<typename T>
    T GetVariable(const std::string& name, const T& defaultValue = T{}) const;
    
    bool HasVariable(const std::string& name) const;
    void RemoveVariable(const std::string& name);
    
    // Get all nodes
    const std::vector<std::shared_ptr<VisualScriptNode>>& GetNodes() const { return nodes; }
    
    // Find event nodes
    std::vector<std::shared_ptr<VisualScriptNode>> GetEventNodes() const;
    
    // Serialization
    void Save(const std::string& filename) const;
    void Load(const std::string& filename);
    std::string SerializeToJson() const;
    void DeserializeFromJson(const std::string& json);
    
    // Validation
    bool Validate(std::vector<std::string>& errors) const;
    
    // Info
    std::string GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }
    size_t GetNodeCount() const { return nodes.size(); }
};

// ============================================
// Visual Script Runtime
// ============================================

class VisualScriptRuntime {
private:
    VisualScriptGraph* graph;
    
    // Execution state
    bool isRunning;
    float deltaTime;
    
    // Event queues
    std::vector<std::shared_ptr<VisualScriptNode>> pendingEvents;
    
    // Debug
    bool debugMode;
    std::vector<std::string> executionLog;
    
public:
    VisualScriptRuntime();
    explicit VisualScriptRuntime(VisualScriptGraph* graph_);
    ~VisualScriptRuntime();
    
    void SetGraph(VisualScriptGraph* graph_);
    
    // Execution
    void Start();
    void Stop();
    void Update(float dt);
    void TriggerEvent(const std::string& eventName);
    
    // Execute specific node
    void ExecuteNode(VisualScriptNode& node);
    void ExecuteFlow(Pin* outputPin);
    
    // State
    bool IsRunning() const { return isRunning; }
    float GetDeltaTime() const { return deltaTime; }
    
    // Debug
    void SetDebugMode(bool enable) { debugMode = enable; }
    void LogExecution(const std::string& message);
    const std::vector<std::string>& GetExecutionLog() const { return executionLog; }
    void ClearExecutionLog() { executionLog.clear(); }
    
    // Variable access (proxies to graph)
    template<typename T>
    void SetVariable(const std::string& name, const T& value);
    
    template<typename T>
    T GetVariable(const std::string& name, const T& defaultValue = T{}) const;
};

// ============================================
// Node Registry
// ============================================

class NodeRegistry {
private:
    std::unordered_map<std::string, NodeDefinition> definitions;
    
    NodeRegistry() = default;
    
public:
    static NodeRegistry& GetInstance();
    
    // Registration
    void RegisterNode(const NodeDefinition& def);
    void RegisterBuiltInNodes();
    
    // Query
    bool HasNode(const std::string& typeName) const;
    NodeDefinition GetDefinition(const std::string& typeName) const;
    std::vector<NodeDefinition> GetDefinitionsByCategory(NodeCategory category) const;
    std::vector<NodeDefinition> GetAllDefinitions() const;
    
    // Factory
    std::shared_ptr<VisualScriptNode> CreateNode(const std::string& typeName);
};

// ============================================
// Visual Script Component (for ECS)
// ============================================

class VisualScriptComponent {
public:
    std::shared_ptr<VisualScriptGraph> graph;
    std::unique_ptr<VisualScriptRuntime> runtime;
    bool autoStart;
    
    VisualScriptComponent();
    explicit VisualScriptComponent(std::shared_ptr<VisualScriptGraph> graph_);
    
    void Start();
    void Stop();
    void Update(float dt);
};

} // namespace vge
