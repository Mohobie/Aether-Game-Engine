#pragma once

// Cross-platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_MACOS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#elif defined(__unix__)
    #define PLATFORM_UNIX
#endif

// Windows-specific includes and fixes
#ifdef PLATFORM_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX  // Prevent Windows.h from defining min/max macros
    #endif
    #include <windows.h>
    #include <winsock2.h>
    #include <direct.h>
    #include <io.h>
    
    // POSIX compatibility
    #define access _access
    #define F_OK 0
    #define W_OK 2
    #define R_OK 4
    
    // Threading
    #include <thread>
    #include <mutex>
    
    // Sleep function
    #define sleep_ms(ms) Sleep(ms)
    
    // Path separator
    #define PATH_SEP '\\'
    #define PATH_SEP_STR "\\"
#else
    // Linux/macOS
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <fcntl.h>
    
    // Sleep function
    #define sleep_ms(ms) usleep((ms) * 1000)
    
    // Path separator
    #define PATH_SEP '/'
    #define PATH_SEP_STR "/"
#endif

// Cross-platform types
#include <cstdint>
#include <string>

namespace vge {

// Cross-platform path utilities
class Path {
public:
    static std::string Join(const std::string& a, const std::string& b);
    static std::string GetDirectory(const std::string& path);
    static std::string GetFilename(const std::string& path);
    static std::string GetExtension(const std::string& path);
    static std::string Normalize(const std::string& path);
    static bool IsAbsolute(const std::string& path);
};

// Cross-platform file operations
class File {
public:
    static bool Exists(const std::string& path);
    static bool IsDirectory(const std::string& path);
    static bool CreateDirectory(const std::string& path);
    static bool Delete(const std::string& path);
    static size_t GetSize(const std::string& path);
    static std::string ReadText(const std::string& path);
    static bool WriteText(const std::string& path, const std::string& content);
};

// Cross-platform sleep
void SleepMs(uint32_t milliseconds);

// Cross-platform thread ID
uint64_t GetCurrentThreadId();

} // namespace vge
