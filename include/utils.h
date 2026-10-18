#pragma once

#include <filesystem>
#include <string>

// Reads a whole file into a string. Throws std::runtime_error, naming the
// file, if it can't be opened or read.
std::string readFile(const std::filesystem::path& path);
