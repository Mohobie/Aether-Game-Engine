#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <sstream>
#include "math/vec3.h"

namespace vge {

// Forward declarations
class Engine;
class PlayerController;
class World;
class TimeSystem;
class EntitySpawner;
class DebugSystem;
class InventoryManager;

// ============================================
// Command Result
// ============================================
enum class CommandResult {
    Success,
    InvalidCommand,
    InvalidArguments,
    ExecutionError,
    UnknownError
};

// ============================================
// Command Context - holds references to engine systems
// ============================================
struct CommandContext {
    Engine* engine;
    PlayerController* player;
    World* world;
    TimeSystem* timeSystem;
    EntitySpawner* entitySpawner;
    DebugSystem* debugSystem;
    InventoryManager* inventoryManager;
    
    CommandContext() 
        : engine(nullptr), player(nullptr), world(nullptr),
          timeSystem(nullptr), entitySpawner(nullptr),
          debugSystem(nullptr), inventoryManager(nullptr) {}
    
    bool IsValid() const {
        return engine != nullptr && player != nullptr && world != nullptr;
    }
};

// ============================================
// Parsed Command
// ============================================
struct ParsedCommand {
    std::string name;
    std::vector<std::string> args;
    std::string raw;
    
    bool HasArg(size_t index) const { return index < args.size(); }
    std::string GetArg(size_t index, const std::string& defaultVal = "") const {
        return index < args.size() ? args[index] : defaultVal;
    }
    int GetIntArg(size_t index, int defaultVal = 0) const;
    float GetFloatArg(size_t index, float defaultVal = 0.0f) const;
    bool GetBoolArg(size_t index, bool defaultVal = false) const;
    Vec3 GetVec3Arg(size_t startIndex, const Vec3& defaultVal = Vec3(0,0,0)) const;
};

// ============================================
// Command Definition
// ============================================
struct CommandDef {
    std::string name;
    std::string description;
    std::string usage;
    std::vector<std::string> aliases;
    int minArgs;
    int maxArgs;
    std::function<CommandResult(const ParsedCommand&, const CommandContext&, std::string&)> handler;
    bool requiresCheats;
    bool isDebug;
    
    CommandDef(const std::string& n = "", const std::string& desc = "", 
               const std::string& u = "", const std::vector<std::string>& a = {},
               int minA = 0, int maxA = -1,
               std::function<CommandResult(const ParsedCommand&, const CommandContext&, std::string&)> h = nullptr,
               bool cheats = false, bool debug = false)
        : name(n), description(desc), usage(u), aliases(a),
          minArgs(minA), maxArgs(maxA), handler(h),
          requiresCheats(cheats), isDebug(debug) {}
};

// ============================================
// Command Registry
// ============================================
class CommandRegistry {
private:
    std::unordered_map<std::string, CommandDef> commands;
    std::unordered_map<std::string, std::string> aliasMap;
    
public:
    CommandRegistry();
    ~CommandRegistry() = default;
    
    // Register a command
    void Register(const CommandDef& def);
    void Register(const std::string& name, const std::string& description,
                  const std::string& usage, const std::vector<std::string>& aliases,
                  int minArgs, int maxArgs,
                  std::function<CommandResult(const ParsedCommand&, const CommandContext&, std::string&)> handler,
                  bool requiresCheats = false, bool isDebug = false);
    void RegisterAlias(const std::string& alias, const std::string& target);
    
    // Get command
    const CommandDef* GetCommand(const std::string& name) const;
    bool HasCommand(const std::string& name) const;
    
    // Get all commands
    std::vector<CommandDef> GetAllCommands() const;
    std::vector<CommandDef> GetDebugCommands() const;
    std::vector<CommandDef> GetCheatsCommands() const;
    
    // Help text generation
    std::string GetHelpText(const std::string& commandName) const;
    std::string GetAllHelpText(bool includeDebug = false) const;
    
    // Built-in command registration
    void RegisterBuiltInCommands();
};

// ============================================
// Command Parser
// ============================================
class CommandParser {
public:
    static ParsedCommand Parse(const std::string& input);
    static std::vector<std::string> Tokenize(const std::string& input);
    static bool IsValidCommandName(const std::string& name);
};

// ============================================
// Command Executor
// ============================================
class CommandExecutor {
private:
    CommandRegistry registry;
    CommandContext context;
    bool cheatsEnabled;
    bool debugEnabled;
    std::vector<std::string> history;
    size_t maxHistory;
    
public:
    CommandExecutor();
    ~CommandExecutor() = default;
    
    // Setup
    void Initialize(const CommandContext& ctx);
    void Shutdown();
    
    // Execute a command string
    CommandResult Execute(const std::string& commandLine, std::string& output);
    CommandResult ExecuteParsed(const ParsedCommand& cmd, std::string& output);
    
    // Context
    void SetContext(const CommandContext& ctx) { context = ctx; }
    const CommandContext& GetContext() const { return context; }
    
    // Settings
    void SetCheatsEnabled(bool enabled) { cheatsEnabled = enabled; }
    bool AreCheatsEnabled() const { return cheatsEnabled; }
    void SetDebugEnabled(bool enabled) { debugEnabled = enabled; }
    bool IsDebugEnabled() const { return debugEnabled; }
    
    // History
    void AddToHistory(const std::string& cmd);
    const std::vector<std::string>& GetHistory() const { return history; }
    void ClearHistory() { history.clear(); }
    
    // Access registry
    CommandRegistry& GetRegistry() { return registry; }
    const CommandRegistry& GetRegistry() const { return registry; }
    
    // Validation
    bool ValidateArgs(const ParsedCommand& cmd, const CommandDef& def, std::string& error) const;
};

// ============================================
// Command History UI Helper
// ============================================
class CommandHistory {
private:
    std::vector<std::string> commands;
    int currentIndex;
    size_t maxSize;
    
public:
    CommandHistory(size_t max = 100);
    
    void Add(const std::string& cmd);
    std::string GetPrevious();
    std::string GetNext();
    void ResetIndex();
    
    const std::vector<std::string>& GetAll() const { return commands; }
    void Clear() { commands.clear(); currentIndex = -1; }
};

// ============================================
// Built-in Command Handlers
// ============================================
namespace CommandHandlers {
    // Player commands
    CommandResult Teleport(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult GiveItem(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult SetTime(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult SpawnEntity(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    
    // Debug commands
    CommandResult DebugInfo(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult DebugProfiler(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult DebugMetrics(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult DebugVisual(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult DebugWireframe(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    
    // System commands
    CommandResult Help(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult Clear(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult Quit(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult Echo(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult ListCommands(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult History(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    
    // World commands
    CommandResult SetWeather(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult SetSpeed(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult Kill(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
    CommandResult Heal(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output);
} // namespace CommandHandlers

} // namespace vge
