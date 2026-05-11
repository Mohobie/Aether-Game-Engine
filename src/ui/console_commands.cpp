#include "ui/console_commands.h"
#include "core/engine.h"
#include "core/player_controller.h"
#include "core/time_system.h"
#include "core/item_system.h"
#include "voxel/world.h"
#include "editor/entity_spawner.h"
#include "debug/debug_system.h"
#include "core/logger.h"
#include <algorithm>
#include <cstdlib>
#include <iomanip>

namespace vge {

// ============================================
// ParsedCommand Helpers
// ============================================
int ParsedCommand::GetIntArg(size_t index, int defaultVal) const {
    if (index >= args.size()) return defaultVal;
    try {
        return std::stoi(args[index]);
    } catch (...) {
        return defaultVal;
    }
}

float ParsedCommand::GetFloatArg(size_t index, float defaultVal) const {
    if (index >= args.size()) return defaultVal;
    try {
        return std::stof(args[index]);
    } catch (...) {
        return defaultVal;
    }
}

bool ParsedCommand::GetBoolArg(size_t index, bool defaultVal) const {
    if (index >= args.size()) return defaultVal;
    std::string val = args[index];
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (val == "true" || val == "1" || val == "yes" || val == "on") return true;
    if (val == "false" || val == "0" || val == "no" || val == "off") return false;
    return defaultVal;
}

Vec3 ParsedCommand::GetVec3Arg(size_t startIndex, const Vec3& defaultVal) const {
    if (startIndex + 2 >= args.size()) return defaultVal;
    try {
        return Vec3(
            std::stof(args[startIndex]),
            std::stof(args[startIndex + 1]),
            std::stof(args[startIndex + 2])
        );
    } catch (...) {
        return defaultVal;
    }
}

// ============================================
// Command Parser
// ============================================
ParsedCommand CommandParser::Parse(const std::string& input) {
    ParsedCommand result;
    result.raw = input;
    
    auto tokens = Tokenize(input);
    if (!tokens.empty()) {
        result.name = tokens[0];
        for (size_t i = 1; i < tokens.size(); ++i) {
            result.args.push_back(tokens[i]);
        }
    }
    
    return result;
}

std::vector<std::string> CommandParser::Tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        
        if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    return tokens;
}

bool CommandParser::IsValidCommandName(const std::string& name) {
    if (name.empty() || name.length() > 64) return false;
    for (char c : name) {
        if (!std::isalnum(c) && c != '_' && c != '-') return false;
    }
    return true;
}

// ============================================
// Command Registry
// ============================================
CommandRegistry::CommandRegistry() {
    RegisterBuiltInCommands();
}

void CommandRegistry::Register(const CommandDef& def) {
    commands[def.name] = def;
    for (const auto& alias : def.aliases) {
        aliasMap[alias] = def.name;
    }
}

void CommandRegistry::Register(const std::string& name, const std::string& description,
                               const std::string& usage, const std::vector<std::string>& aliases,
                               int minArgs, int maxArgs,
                               std::function<CommandResult(const ParsedCommand&, const CommandContext&, std::string&)> handler,
                               bool requiresCheats, bool isDebug) {
    CommandDef def(name, description, usage, aliases, minArgs, maxArgs, handler, requiresCheats, isDebug);
    Register(def);
}

void CommandRegistry::RegisterAlias(const std::string& alias, const std::string& target) {
    aliasMap[alias] = target;
}

const CommandDef* CommandRegistry::GetCommand(const std::string& name) const {
    auto it = commands.find(name);
    if (it != commands.end()) return &(it->second);
    
    auto aliasIt = aliasMap.find(name);
    if (aliasIt != aliasMap.end()) {
        auto cmdIt = commands.find(aliasIt->second);
        if (cmdIt != commands.end()) return &(cmdIt->second);
    }
    
    return nullptr;
}

bool CommandRegistry::HasCommand(const std::string& name) const {
    return GetCommand(name) != nullptr;
}

std::vector<CommandDef> CommandRegistry::GetAllCommands() const {
    std::vector<CommandDef> result;
    for (const auto& pair : commands) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<CommandDef> CommandRegistry::GetDebugCommands() const {
    std::vector<CommandDef> result;
    for (const auto& pair : commands) {
        if (pair.second.isDebug) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<CommandDef> CommandRegistry::GetCheatsCommands() const {
    std::vector<CommandDef> result;
    for (const auto& pair : commands) {
        if (pair.second.requiresCheats) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::string CommandRegistry::GetHelpText(const std::string& commandName) const {
    const CommandDef* cmd = GetCommand(commandName);
    if (!cmd) return "Unknown command: " + commandName;
    
    std::stringstream ss;
    ss << "Command: " << cmd->name << "\n";
    ss << "Description: " << cmd->description << "\n";
    ss << "Usage: " << cmd->usage << "\n";
    if (!cmd->aliases.empty()) {
        ss << "Aliases: ";
        for (size_t i = 0; i < cmd->aliases.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << cmd->aliases[i];
        }
        ss << "\n";
    }
    if (cmd->requiresCheats) ss << "Requires cheats enabled\n";
    if (cmd->isDebug) ss << "Debug command\n";
    return ss.str();
}

std::string CommandRegistry::GetAllHelpText(bool includeDebug) const {
    std::stringstream ss;
    ss << "=== Available Commands ===\n\n";
    
    // Group by category
    ss << "[Player Commands]\n";
    ss << "  teleport/tp <x> <y> <z>   - Teleport to coordinates\n";
    ss << "  give <item> [count]       - Give item to player\n";
    ss << "  settime <time>            - Set time of day (0-24000)\n";
    ss << "  spawn <type> [x] [y] [z]  - Spawn entity\n";
    ss << "  speed <value>             - Set player speed\n";
    ss << "  kill                      - Kill player\n";
    ss << "  heal [amount]             - Heal player\n";
    ss << "\n";
    
    ss << "[World Commands]\n";
    ss << "  weather <type>            - Set weather (clear/rain/storm/snow)\n";
    ss << "\n";
    
    ss << "[System Commands]\n";
    ss << "  help [command]            - Show help\n";
    ss << "  clear                     - Clear console\n";
    ss << "  quit/exit                 - Exit game\n";
    ss << "  echo <text>               - Print text\n";
    ss << "  list                      - List all commands\n";
    ss << "  history                   - Show command history\n";
    ss << "\n";
    
    if (includeDebug) {
        ss << "[Debug Commands]\n";
        ss << "  debug info                - Show debug info\n";
        ss << "  debug profiler            - Show profiler report\n";
        ss << "  debug metrics             - Show performance metrics\n";
        ss << "  debug visual <mode>       - Toggle visual debug mode\n";
        ss << "  debug wireframe           - Toggle wireframe mode\n";
        ss << "\n";
    }
    
    ss << "Use 'help <command>' for detailed info on a specific command.\n";
    return ss.str();
}

void CommandRegistry::RegisterBuiltInCommands() {
    // Player commands
    Register({"teleport", "Teleport player to coordinates", 
              "teleport/tp <x> <y> [z] or teleport/tp <player> <x> <y> <z>",
              {"tp"}, 1, 4, CommandHandlers::Teleport, true, false});
    
    Register({"give", "Give item(s) to player",
              "give <item_id> [count]",
              {}, 1, 2, CommandHandlers::GiveItem, true, false});
    
    Register({"settime", "Set time of day",
              "settime <0-24000> or settime day/night/noon/midnight",
              {"time"}, 1, 1, CommandHandlers::SetTime, true, false});
    
    Register({"spawn", "Spawn entity at position",
              "spawn <entity_type> [x] [y] [z]",
              {}, 1, 4, CommandHandlers::SpawnEntity, true, false});
    
    Register({"speed", "Set player movement speed",
              "speed <value>",
              {}, 1, 1, CommandHandlers::SetSpeed, true, false});
    
    Register({"kill", "Kill the player",
              "kill",
              {}, 0, 0, CommandHandlers::Kill, true, false});
    
    Register({"heal", "Heal the player",
              "heal [amount]",
              {}, 0, 1, CommandHandlers::Heal, true, false});
    
    // World commands
    Register({"weather", "Set weather",
              "weather <clear/rain/storm/snow>",
              {}, 1, 1, CommandHandlers::SetWeather, true, false});
    
    // Debug commands
    Register({"debug", "Debug commands",
              "debug info/profiler/metrics/visual/wireframe",
              {}, 1, 2, CommandHandlers::DebugInfo, false, true});
    
    // System commands
    Register({"help", "Show help information",
              "help [command]",
              {"?", "h"}, 0, 1, CommandHandlers::Help, false, false});
    
    Register({"clear", "Clear console output",
              "clear",
              {"cls"}, 0, 0, CommandHandlers::Clear, false, false});
    
    Register({"quit", "Quit the game",
              "quit/exit",
              {"exit"}, 0, 0, CommandHandlers::Quit, false, false});
    
    Register({"echo", "Print text to console",
              "echo <text>",
              {}, 1, -1, CommandHandlers::Echo, false, false});
    
    Register({"list", "List all available commands",
              "list",
              {"commands", "cmds"}, 0, 0, CommandHandlers::ListCommands, false, false});
    
    Register({"history", "Show command history",
              "history",
              {}, 0, 0, CommandHandlers::History, false, false});
}

// ============================================
// Command Executor
// ============================================
CommandExecutor::CommandExecutor()
    : cheatsEnabled(false), debugEnabled(false), maxHistory(100) {}

void CommandExecutor::Initialize(const CommandContext& ctx) {
    context = ctx;
    registry.RegisterBuiltInCommands();
}

void CommandExecutor::Shutdown() {
    history.clear();
}

CommandResult CommandExecutor::Execute(const std::string& commandLine, std::string& output) {
    if (commandLine.empty()) {
        output = "";
        return CommandResult::Success;
    }
    
    AddToHistory(commandLine);
    
    ParsedCommand cmd = CommandParser::Parse(commandLine);
    if (cmd.name.empty()) {
        output = "Empty command";
        return CommandResult::InvalidCommand;
    }
    
    return ExecuteParsed(cmd, output);
}

CommandResult CommandExecutor::ExecuteParsed(const ParsedCommand& cmd, std::string& output) {
    const CommandDef* def = registry.GetCommand(cmd.name);
    if (!def) {
        output = "Unknown command: " + cmd.name + ". Type 'help' for available commands.";
        return CommandResult::InvalidCommand;
    }
    
    // Check cheats requirement
    if (def->requiresCheats && !cheatsEnabled) {
        output = "Command '" + cmd.name + "' requires cheats to be enabled.";
        return CommandResult::ExecutionError;
    }
    
    // Check debug requirement
    if (def->isDebug && !debugEnabled) {
        output = "Command '" + cmd.name + "' requires debug mode to be enabled.";
        return CommandResult::ExecutionError;
    }
    
    // Validate arguments
    std::string error;
    if (!ValidateArgs(cmd, *def, error)) {
        output = error;
        return CommandResult::InvalidArguments;
    }
    
    // Execute
    return def->handler(cmd, context, output);
}

bool CommandExecutor::ValidateArgs(const ParsedCommand& cmd, const CommandDef& def, std::string& error) const {
    int argCount = static_cast<int>(cmd.args.size());
    
    if (argCount < def.minArgs) {
        error = "Too few arguments. Usage: " + def.usage;
        return false;
    }
    
    if (def.maxArgs >= 0 && argCount > def.maxArgs) {
        error = "Too many arguments. Usage: " + def.usage;
        return false;
    }
    
    return true;
}

void CommandExecutor::AddToHistory(const std::string& cmd) {
    if (cmd.empty()) return;
    history.push_back(cmd);
    if (history.size() > maxHistory) {
        history.erase(history.begin());
    }
}

// ============================================
// Command History
// ============================================
CommandHistory::CommandHistory(size_t max) : currentIndex(-1), maxSize(max) {}

void CommandHistory::Add(const std::string& cmd) {
    if (cmd.empty()) return;
    commands.push_back(cmd);
    if (commands.size() > maxSize) {
        commands.erase(commands.begin());
    }
    currentIndex = static_cast<int>(commands.size());
}

std::string CommandHistory::GetPrevious() {
    if (commands.empty()) return "";
    currentIndex--;
    if (currentIndex < 0) currentIndex = 0;
    return commands[currentIndex];
}

std::string CommandHistory::GetNext() {
    if (commands.empty()) return "";
    currentIndex++;
    if (currentIndex >= static_cast<int>(commands.size())) {
        currentIndex = static_cast<int>(commands.size());
        return "";
    }
    return commands[currentIndex];
}

void CommandHistory::ResetIndex() {
    currentIndex = static_cast<int>(commands.size());
}

// ============================================
// Built-in Command Handlers
// ============================================
namespace CommandHandlers {

// Player Commands
CommandResult Teleport(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.player) {
        output = "Player not available";
        return CommandResult::ExecutionError;
    }
    
    Vec3 targetPos;
    size_t argOffset = 0;
    
    // Check if first arg is a player name (not a number)
    if (cmd.args.size() == 4) {
        // teleport <player> <x> <y> <z>
        argOffset = 1;
        output = "Teleporting player '" + cmd.args[0] + "' to ";
    } else {
        output = "Teleporting to ";
    }
    
    if (cmd.args.size() >= 3 + argOffset) {
        targetPos = cmd.GetVec3Arg(argOffset);
    } else if (cmd.args.size() == 2 + argOffset) {
        // teleport <x> <z> (keep current Y)
        targetPos.x = cmd.GetFloatArg(argOffset);
        targetPos.y = ctx.player->GetPosition().y;
        targetPos.z = cmd.GetFloatArg(argOffset + 1);
    } else if (cmd.args.size() == 1 + argOffset) {
        // Could be a named location or relative
        output = "Usage: teleport <x> <y> <z> or teleport <x> <z>";
        return CommandResult::InvalidArguments;
    } else {
        output = "Usage: teleport <x> <y> <z>";
        return CommandResult::InvalidArguments;
    }
    
    ctx.player->SetPosition(targetPos);
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "(" << targetPos.x << ", " << targetPos.y << ", " << targetPos.z << ")";
    output += ss.str();
    
    return CommandResult::Success;
}

CommandResult GiveItem(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.player || !ctx.inventoryManager) {
        output = "Inventory system not available";
        return CommandResult::ExecutionError;
    }
    
    std::string itemID = cmd.GetArg(0);
    int count = cmd.GetIntArg(1, 1);
    if (count < 1) count = 1;
    if (count > 9999) count = 9999;
    
    // Check if item exists in registry
    ItemRegistry& itemReg = ItemRegistry::GetInstance();
    if (!itemReg.HasItem(itemID)) {
        output = "Unknown item: " + itemID;
        return CommandResult::InvalidArguments;
    }
    
    // Get or create player inventory
    Inventory* playerInv = ctx.inventoryManager->GetInventory("player");
    if (!playerInv) {
        playerInv = ctx.inventoryManager->CreateInventory("player", 9, 4);
    }
    
    if (playerInv->AddItem(itemID, count)) {
        const ItemDef& def = itemReg.GetItem(itemID);
        std::stringstream ss;
        ss << "Gave " << count << "x " << def.displayName << " (" << itemID << ")";
        output = ss.str();
        return CommandResult::Success;
    } else {
        output = "Failed to give item - inventory full";
        return CommandResult::ExecutionError;
    }
}

CommandResult SetTime(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.timeSystem) {
        output = "Time system not available";
        return CommandResult::ExecutionError;
    }
    
    std::string timeArg = cmd.GetArg(0);
    std::transform(timeArg.begin(), timeArg.end(), timeArg.begin(), ::tolower);
    
    int timeValue = 0;
    if (timeArg == "day" || timeArg == "morning") {
        timeValue = 1000;
    } else if (timeArg == "noon") {
        timeValue = 6000;
    } else if (timeArg == "night") {
        timeValue = 13000;
    } else if (timeArg == "midnight") {
        timeValue = 18000;
    } else {
        try {
            timeValue = std::stoi(timeArg);
            if (timeValue < 0 || timeValue > 24000) {
                output = "Time must be between 0 and 24000";
                return CommandResult::InvalidArguments;
            }
        } catch (...) {
            output = "Invalid time value. Use 0-24000 or day/noon/night/midnight";
            return CommandResult::InvalidArguments;
        }
    }
    
    ctx.timeSystem->SetTime(timeValue);
    output = "Time set to " + std::to_string(timeValue) + " (" + ctx.timeSystem->GetTimeString() + ")";
    return CommandResult::Success;
}

CommandResult SpawnEntity(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.entitySpawner || !ctx.player) {
        output = "Entity spawner not available";
        return CommandResult::ExecutionError;
    }
    
    std::string entityType = cmd.GetArg(0);
    Vec3 spawnPos;
    
    if (cmd.args.size() >= 4) {
        spawnPos = cmd.GetVec3Arg(1);
    } else {
        // Spawn in front of player
        spawnPos = ctx.player->GetPosition();
        // Offset slightly forward based on player look direction
        Vec3 lookDir;
        lookDir.x = cosf(ctx.player->GetYaw() * 3.14159f / 180.0f);
        lookDir.z = sinf(ctx.player->GetYaw() * 3.14159f / 180.0f);
        spawnPos = spawnPos + lookDir * 3.0f;
    }
    
    uint32_t entityId = ctx.entitySpawner->SpawnEntity(entityType, spawnPos);
    if (entityId > 0) {
        std::stringstream ss;
        ss << "Spawned " << entityType << " at (" 
           << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z 
           << ") with ID " << entityId;
        output = ss.str();
        return CommandResult::Success;
    } else {
        output = "Failed to spawn entity: " + entityType;
        return CommandResult::ExecutionError;
    }
}

CommandResult SetSpeed(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    output = "Speed command requires player controller integration";
    return CommandResult::ExecutionError;
}

CommandResult Kill(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.player) {
        output = "Player not available";
        return CommandResult::ExecutionError;
    }
    
    // Reset player to spawn position with fall damage
    ctx.player->SetPosition(Vec3(0, 40, 0));
    output = "Player killed and respawned";
    return CommandResult::Success;
}

CommandResult Heal(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    // Health system not yet implemented - just acknowledge
    int amount = cmd.GetIntArg(0, 100);
    std::stringstream ss;
    ss << "Healed player for " << amount << " HP (health system pending)";
    output = ss.str();
    return CommandResult::Success;
}

// World Commands
CommandResult SetWeather(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.timeSystem) {
        output = "Weather system not available";
        return CommandResult::ExecutionError;
    }
    
    std::string weatherArg = cmd.GetArg(0);
    std::transform(weatherArg.begin(), weatherArg.end(), weatherArg.begin(), ::tolower);
    
    WeatherType weather;
    if (weatherArg == "clear" || weatherArg == "sun") {
        weather = WeatherType::Clear;
    } else if (weatherArg == "rain") {
        weather = WeatherType::Rain;
    } else if (weatherArg == "storm" || weatherArg == "thunder") {
        weather = WeatherType::Storm;
    } else if (weatherArg == "snow") {
        weather = WeatherType::Snow;
    } else {
        output = "Unknown weather type. Use: clear, rain, storm, snow";
        return CommandResult::InvalidArguments;
    }
    
    ctx.timeSystem->SetWeather(weather, 600.0f); // 10 minutes default
    output = "Weather set to " + weatherArg;
    return CommandResult::Success;
}

// Debug Commands
CommandResult DebugInfo(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.debugSystem) {
        output = "Debug system not available";
        return CommandResult::ExecutionError;
    }
    
    if (cmd.args.size() < 1) {
        output = "Usage: debug info/profiler/metrics/visual/wireframe";
        return CommandResult::InvalidArguments;
    }
    
    std::string subcmd = cmd.GetArg(0);
    std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);
    
    if (subcmd == "info") {
        output = "Debug System Info:\n";
        output += "  Enabled: " + std::string(ctx.debugSystem->IsEnabled() ? "Yes" : "No") + "\n";
        output += "  Profiler samples: " + std::to_string(ctx.debugSystem->GetProfiler().GetSamples().size()) + "\n";
    } else if (subcmd == "profiler") {
        return CommandHandlers::DebugProfiler(cmd, ctx, output);
    } else if (subcmd == "metrics") {
        return CommandHandlers::DebugMetrics(cmd, ctx, output);
    } else if (subcmd == "visual") {
        return CommandHandlers::DebugVisual(cmd, ctx, output);
    } else if (subcmd == "wireframe") {
        return CommandHandlers::DebugWireframe(cmd, ctx, output);
    } else {
        output = "Unknown debug subcommand: " + subcmd;
        return CommandResult::InvalidArguments;
    }
    
    return CommandResult::Success;
}

CommandResult DebugProfiler(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.debugSystem) {
        output = "Debug system not available";
        return CommandResult::ExecutionError;
    }
    
    auto report = ctx.debugSystem->GetProfiler().GetReport();
    std::stringstream ss;
    ss << "=== Profiler Report ===\n";
    for (const auto& line : report) {
        ss << line << "\n";
    }
    output = ss.str();
    return CommandResult::Success;
}

CommandResult DebugMetrics(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.debugSystem) {
        output = "Debug system not available";
        return CommandResult::ExecutionError;
    }
    
    auto& metrics = ctx.debugSystem->GetMetrics();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "=== Performance Metrics ===\n";
    ss << "FPS: " << metrics.fps << "\n";
    ss << "Frame Time: " << metrics.frameTimeMs << " ms\n";
    ss << "Draw Calls: " << metrics.drawCalls << "\n";
    ss << "Triangles: " << metrics.trianglesRendered << "\n";
    ss << "Vertices: " << metrics.verticesRendered << "\n";
    ss << "Loaded Chunks: " << metrics.loadedChunks << "\n";
    ss << "Visible Chunks: " << metrics.visibleChunks << "\n";
    ss << "Memory Used: " << metrics.memoryUsedMB << " MB\n";
    ss << "Player Position: (" << metrics.playerPosition.x << ", " 
       << metrics.playerPosition.y << ", " << metrics.playerPosition.z << ")\n";
    output = ss.str();
    return CommandResult::Success;
}

CommandResult DebugVisual(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.debugSystem) {
        output = "Debug system not available";
        return CommandResult::ExecutionError;
    }
    
    auto& visualizer = ctx.debugSystem->GetVisualizer();
    
    if (cmd.args.size() < 2) {
        output = "Usage: debug visual <wireframe/normals/bounds/chunks/lights/collision/none/all>";
        return CommandResult::InvalidArguments;
    }
    
    std::string mode = cmd.GetArg(1);
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    
    if (mode == "wireframe") {
        visualizer.ToggleMode(DebugDrawMode::Wireframe);
        output = "Toggled wireframe mode";
    } else if (mode == "normals") {
        visualizer.ToggleMode(DebugDrawMode::Normals);
        output = "Toggled normals visualization";
    } else if (mode == "bounds" || mode == "boundingboxes") {
        visualizer.ToggleMode(DebugDrawMode::BoundingBoxes);
        output = "Toggled bounding boxes";
    } else if (mode == "chunks" || mode == "chunkborders") {
        visualizer.ToggleMode(DebugDrawMode::ChunkBorders);
        output = "Toggled chunk borders";
    } else if (mode == "lights" || mode == "lightvolumes") {
        visualizer.ToggleMode(DebugDrawMode::LightVolumes);
        output = "Toggled light volumes";
    } else if (mode == "collision") {
        visualizer.ToggleMode(DebugDrawMode::CollisionShapes);
        output = "Toggled collision shapes";
    } else if (mode == "none" || mode == "off") {
        visualizer.SetDrawMode(0);
        output = "Disabled all debug visuals";
    } else if (mode == "all") {
        visualizer.SetDrawMode(static_cast<uint32_t>(DebugDrawMode::All));
        output = "Enabled all debug visuals";
    } else {
        output = "Unknown visual mode: " + mode;
        return CommandResult::InvalidArguments;
    }
    
    return CommandResult::Success;
}

CommandResult DebugWireframe(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    if (!ctx.debugSystem) {
        output = "Debug system not available";
        return CommandResult::ExecutionError;
    }
    
    ctx.debugSystem->GetVisualizer().ToggleMode(DebugDrawMode::Wireframe);
    output = "Toggled wireframe mode";
    return CommandResult::Success;
}

// System Commands
CommandResult Help(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    (void)ctx; // Unused
    CommandRegistry registry;
    
    if (cmd.args.empty()) {
        output = registry.GetAllHelpText(true);
    } else {
        output = registry.GetHelpText(cmd.GetArg(0));
    }
    return CommandResult::Success;
}

CommandResult Clear(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    (void)cmd;
    (void)ctx;
    output = "__CLEAR__"; // Special signal for console UI
    return CommandResult::Success;
}

CommandResult Quit(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    (void)cmd;
    if (ctx.engine) {
        ctx.engine->Stop();
        output = "Shutting down...";
        return CommandResult::Success;
    }
    output = "Engine not available";
    return CommandResult::ExecutionError;
}

CommandResult Echo(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    (void)ctx;
    std::stringstream ss;
    for (size_t i = 0; i < cmd.args.size(); ++i) {
        if (i > 0) ss << " ";
        ss << cmd.args[i];
    }
    output = ss.str();
    return CommandResult::Success;
}

CommandResult ListCommands(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    (void)cmd;
    (void)ctx;
    CommandRegistry registry;
    auto commands = registry.GetAllCommands();
    
    std::stringstream ss;
    ss << "=== All Commands (" << commands.size() << ") ===\n";
    for (const auto& def : commands) {
        ss << "  " << def.name;
        if (!def.aliases.empty()) {
            ss << " (";
            for (size_t i = 0; i < def.aliases.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << def.aliases[i];
            }
            ss << ")";
        }
        if (def.requiresCheats) ss << " [cheats]";
        if (def.isDebug) ss << " [debug]";
        ss << "\n";
    }
    output = ss.str();
    return CommandResult::Success;
}

CommandResult History(const ParsedCommand& cmd, const CommandContext& ctx, std::string& output) {
    (void)cmd;
    (void)ctx;
    output = "Command history not available in this context";
    return CommandResult::Success;
}

} // namespace CommandHandlers

} // namespace vge
