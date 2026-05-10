#include "file_system.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace aether {

bool FileSystem::exists(const std::string& path) {
    return vge::File::Exists(path);
}

std::string FileSystem::readText(const std::string& path) {
    return vge::File::ReadText(path);
}

bool FileSystem::writeText(const std::string& path, const std::string& text) {
    return vge::File::WriteText(path, text);
}

bool FileSystem::createDirectory(const std::string& path) {
    return vge::File::CreateDirectory(path);
}

bool FileSystem::deleteFile(const std::string& path) {
    return vge::File::Delete(path);
}

std::vector<std::string> FileSystem::listDirectory(const std::string& path) {
    std::vector<std::string> result;
    
#ifdef PLATFORM_WINDOWS
    std::string searchPath = path + "\\*";
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = findData.cFileName;
            if (name != "." && name != "..") {
                result.push_back(name);
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
                result.push_back(name);
            }
        }
        closedir(dir);
    }
#endif
    
    return result;
}

} // namespace aether
