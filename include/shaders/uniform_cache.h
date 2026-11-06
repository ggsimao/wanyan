#pragma once

#include <GL/glew.h>

#include <string>
#include <unordered_map>

struct UniformNameHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }
};

class UniformCache {
public:
    UniformCache() = default;
    UniformCache(const UniformCache&) = delete;
    UniformCache& operator=(const UniformCache&) = delete;
    UniformCache(UniformCache&& other) noexcept;
    UniformCache& operator=(UniformCache&& other) noexcept;

    GLint uniformLocation(std::string_view name, std::string label, GLuint program) const;
private:
    mutable std::unordered_map<std::string, GLint, UniformNameHash, std::equal_to<>> _cache;
};
