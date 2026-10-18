#include "utils.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    if (file.bad()) {
        throw std::runtime_error("Unable to read file " + path.string());
    }
    return contents.str();
}
