#include "dir.h"

#include <utility>
#include <cstdlib>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

std::string GetRealPath(const std::string &path) {
    char *buffer = nullptr;
    std::string real_path;
#ifdef _WIN32
    buffer = _fullpath(nullptr, path.c_str(), 0);
#else
    buffer = realpath(path.c_str(), nullptr);
#endif
    if (buffer) {
        real_path = buffer;
        delete buffer;
    }
    return std::move(real_path);
}

std::string GetCurrentDir() {
    char *buffer = nullptr;
    std::string path;
    buffer = getcwd(nullptr, 0);
    if (buffer) {
        path = buffer;
        delete buffer;
    }
    return std::move(path);
}
