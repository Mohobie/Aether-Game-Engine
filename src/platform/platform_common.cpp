#include "platform_common.h"
#include <fstream>
#include <iostream>

#ifdef PLATFORM_WINDOWS
    #include <direct.h>
    #include <io.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <fcntl.h>
#endif

namespace vge {

// ============================================
// Path Utilities
// ============================================
std::string Path::Join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    
    char last = a.back();
    char first = b.front();
    
    if (last == '/' || last == '\\') {
        if (first == '/' || first == '\\') {
            return a + b.substr(1);
        }
        return a + b;
    }
    
    if (first == '/' || first == '\\') {
        return a + b;
    }
    
    return a + PATH_SEP_STR + b;
}

std::string Path::GetDirectory(const std::string& path) {
    size_t lastSep = path.find_last_of("/\\");
    if (lastSep == std::string::npos) return "";
    return path.substr(0, lastSep);
}

std::string Path::GetFilename(const std::string& path) {
    size_t lastSep = path.find_last_of("/\\");
    if (lastSep == std::string::npos) return path;
    return path.substr(lastSep + 1);
}

std::string Path::GetExtension(const std::string& path) {
    size_t lastDot = path.find_last_of('.');
    if (lastDot == std::string::npos) return "";
    size_t lastSep = path.find_last_of("/\\");
    if (lastSep != std::string::npos && lastDot < lastSep) return "";
    return path.substr(lastDot);
}

std::string Path::Normalize(const std::string& path) {
    std::string result = path;
    for (size_t i = 0; i < result.length(); ++i) {
        if (result[i] == '/') {
            result[i] = PATH_SEP;
        }
    }
    return result;
}

bool Path::IsAbsolute(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    if (path.length() >= 2 && path[1] == ':') return true;
    if (path.length() >= 2 && path[0] == '\\' && path[1] == '\\') return true;
#else
    if (!path.empty() && path[0] == '/') return true;
#endif
    return false;
}

// ============================================
// File Operations
// ============================================
bool File::Exists(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    return _access(path.c_str(), 0) == 0;
#else
    return access(path.c_str(), F_OK) == 0;
#endif
}

bool File::IsDirectory(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0) return false;
    return (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return false;
    return S_ISDIR(info.st_mode);
#endif
}

bool File::CreateDirectory(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

bool File::Delete(const std::string& path) {
    if (IsDirectory(path)) {
#ifdef PLATFORM_WINDOWS
        return _rmdir(path.c_str()) == 0;
#else
        return rmdir(path.c_str()) == 0;
#endif
    }
    return std::remove(path.c_str()) == 0;
}

uint64_t File::GetLastModifiedTime(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0) return 0;
    return static_cast<uint64_t>(info.st_mtime);
#else
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return 0;
    return static_cast<uint64_t>(info.st_mtime);
#endif
}

std::vector<std::string> File::ListDirectory(const std::string& path) {
    std::vector<std::string> result;
#ifdef PLATFORM_WINDOWS
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((path + "\\*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = findData.cFileName;
            if (name != "." && name != "..") {
                result.push_back(Path::Join(path, name));
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(path.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != "..") {
                result.push_back(Path::Join(path, name));
            }
        }
        closedir(dir);
    }
#endif
    return result;
}

std::vector<std::string> File::ListDirectoryRecursive(const std::string& path) {
    std::vector<std::string> result;
    auto files = ListDirectory(path);
    for (const auto& file : files) {
        result.push_back(file);
        if (IsDirectory(file)) {
            auto subFiles = ListDirectoryRecursive(file);
            result.insert(result.end(), subFiles.begin(), subFiles.end());
        }
    }
    return result;
}

size_t File::GetSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    return static_cast<size_t>(file.tellg());
}

std::string File::ReadText(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

bool File::WriteText(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

// ============================================
// Sleep
// ============================================
void SleepMs(uint32_t milliseconds) {
#ifdef PLATFORM_WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// ============================================
// Thread ID
// ============================================
uint64_t GetCurrentThreadId() {
#ifdef PLATFORM_WINDOWS
    return static_cast<uint64_t>(::GetCurrentThreadId());
#else
    return static_cast<uint64_t>(pthread_self());
#endif
}

} // namespace vge
