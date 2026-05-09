#include "utils.h"
#include <iostream>
namespace aether {
void Logger::info(const std::string& msg) { std::cout << "[INFO] " << msg << std::endl; }
void Logger::error(const std::string& msg) { std::cerr << "[ERROR] " << msg << std::endl; }
} // namespace aether
