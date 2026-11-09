#include "shaders/uniform_cache.h"

#include <iostream>
#include <string_view>
#include <utility>

UniformCache::UniformCache(UniformCache&& other) noexcept
    : _cache(std::move(other._cache)) {}

UniformCache& UniformCache::operator=(UniformCache&& other) noexcept {
    if (this != &other) {
        _cache = std::move(other._cache);
    }
    return *this;
}

GLint UniformCache::uniformLocation(std::string_view name, std::string label,
                                    GLuint program) const {
    auto val = _cache.find(name);
    if (val == _cache.end()) {
        auto name_string = std::string(name);
        const GLint location =
            glGetUniformLocation(program, name_string.c_str());
        if (location == -1) {
            // Either the name isn't in the shader (a typo or a mismatch), or
            // the compiler removed it as unused. GL can't tell the two apart.
            std::cerr << "Warning: uniform \"" << name_string
                      << "\" not found in program \"" << label << "\"\n";
        }
        _cache.emplace(std::move(name_string), location);
        return location;
    }
    return val->second;
}
