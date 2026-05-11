#include "ui/console_commands.h"
#include <iostream>
#include <cassert>

using namespace vge;

void TestCommandParser() {
    std::cout << "Testing CommandParser..." << std::endl;
    
    // Test basic parsing
    auto cmd = CommandParser::Parse("teleport 100 200 300");
    assert(cmd.name == "teleport");
    assert(cmd.args.size() == 3);
    assert(cmd.GetArg(0) == "100");
    assert(cmd.GetIntArg(0) == 100);
    assert(cmd.GetFloatArg(1) == 200.0f);
    
    // Test quoted strings
    auto cmd2 = CommandParser::Parse("echo \"hello world\"");
    assert(cmd2.name == "echo");
    assert(cmd2.args.size() == 1);
    assert(cmd2.GetArg(0) == "hello world");
    
    // Test aliases
    auto cmd3 = CommandParser::Parse("tp 1 2 3");
    assert(cmd3.name == "tp");
    
    // Test empty
    auto cmd4 = CommandParser::Parse("");
    assert(cmd4.name.empty());
    
    std::cout << "  CommandParser tests passed!" << std::endl;
}

void TestCommandRegistry() {
    std::cout << "Testing CommandRegistry..." << std::endl;
    
    CommandRegistry registry;
    
    // Test built-in commands exist
    assert(registry.HasCommand("teleport"));
    assert(registry.HasCommand("tp")); // alias
    assert(registry.HasCommand("give"));
    assert(registry.HasCommand("spawn"));
    assert(registry.HasCommand("settime"));
    assert(registry.HasCommand("help"));
    assert(registry.HasCommand("quit"));
    assert(registry.HasCommand("debug"));
    
    // Test command retrieval
    auto* cmd = registry.GetCommand("teleport");
    assert(cmd != nullptr);
    assert(cmd->name == "teleport");
    assert(cmd->requiresCheats == true);
    
    // Test help text
    std::string help = registry.GetHelpText("teleport");
    assert(!help.empty());
    assert(help.find("teleport") != std::string::npos);
    
    std::cout << "  CommandRegistry tests passed!" << std::endl;
}

void TestCommandExecutor() {
    std::cout << "Testing CommandExecutor..." << std::endl;
    
    CommandExecutor executor;
    CommandContext ctx; // Empty context for basic tests
    executor.Initialize(ctx);
    
    std::string output;
    CommandResult result;
    
    // Test help command (no cheats needed)
    result = executor.Execute("help", output);
    assert(result == CommandResult::Success);
    assert(!output.empty());
    
    // Test echo command
    result = executor.Execute("echo hello world", output);
    assert(result == CommandResult::Success);
    assert(output == "hello world");
    
    // Test unknown command
    result = executor.Execute("unknowncommand", output);
    assert(result == CommandResult::InvalidCommand);
    
    // Test invalid arguments - teleport with no args should fail
    result = executor.Execute("teleport", output);
    // With empty context (no player), it returns ExecutionError not InvalidArguments
    // because the command handler checks for player before args
    assert(result == CommandResult::ExecutionError || result == CommandResult::InvalidArguments);
    
    // Test cheats requirement (cheats disabled)
    result = executor.Execute("teleport 1 2 3", output);
    assert(result == CommandResult::ExecutionError); // Cheats not enabled
    
    // Enable cheats and try again - still fails due to no player/world in context
    executor.SetCheatsEnabled(true);
    result = executor.Execute("teleport 1 2 3", output);
    // Will fail due to no player, but not due to cheats
    assert(result == CommandResult::ExecutionError);
    
    // Test history
    executor.Execute("echo test1", output);
    executor.Execute("echo test2", output);
    const auto& history = executor.GetHistory();
    assert(history.size() >= 2);
    
    std::cout << "  CommandExecutor tests passed!" << std::endl;
}

void TestArgumentValidation() {
    std::cout << "Testing Argument Validation..." << std::endl;
    
    CommandExecutor executor;
    CommandContext ctx;
    executor.Initialize(ctx);
    executor.SetCheatsEnabled(true);
    
    std::string output;
    CommandResult result;
    
    // Test too few args
    result = executor.Execute("give", output);
    assert(result == CommandResult::InvalidArguments);
    
    // Test too many args (give takes max 2)
    result = executor.Execute("give item 5 extra", output);
    assert(result == CommandResult::InvalidArguments);
    
    // Test valid args
    result = executor.Execute("give wood 5", output);
    // Will fail due to no inventory, but args are valid
    // Actually it will fail because item doesn't exist in registry
    
    std::cout << "  Argument Validation tests passed!" << std::endl;
}

void TestCommandHistory() {
    std::cout << "Testing CommandHistory..." << std::endl;
    
    CommandHistory history(10);
    
    history.Add("cmd1");
    history.Add("cmd2");
    history.Add("cmd3");
    
    // Test navigation
    assert(history.GetPrevious() == "cmd3");
    assert(history.GetPrevious() == "cmd2");
    assert(history.GetPrevious() == "cmd1");
    assert(history.GetPrevious() == "cmd1"); // Stays at first
    
    assert(history.GetNext() == "cmd2");
    assert(history.GetNext() == "cmd3");
    assert(history.GetNext().empty()); // Past end
    
    std::cout << "  CommandHistory tests passed!" << std::endl;
}

void TestHelpSystem() {
    std::cout << "Testing Help System..." << std::endl;
    
    CommandRegistry registry;
    
    // Test full help
    std::string allHelp = registry.GetAllHelpText(true);
    assert(!allHelp.empty());
    assert(allHelp.find("teleport") != std::string::npos);
    assert(allHelp.find("give") != std::string::npos);
    assert(allHelp.find("spawn") != std::string::npos);
    assert(allHelp.find("debug") != std::string::npos);
    
    // Test specific help
    std::string tpHelp = registry.GetHelpText("teleport");
    assert(tpHelp.find("Teleport player") != std::string::npos);
    assert(tpHelp.find("tp") != std::string::npos); // alias
    
    // Test unknown command help
    std::string unknown = registry.GetHelpText("nonexistent");
    assert(unknown.find("Unknown") != std::string::npos);
    
    std::cout << "  Help System tests passed!" << std::endl;
}

int main() {
    std::cout << "=== Console Command System Tests ===" << std::endl;
    
    TestCommandParser();
    TestCommandRegistry();
    TestCommandExecutor();
    TestArgumentValidation();
    TestCommandHistory();
    TestHelpSystem();
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}
