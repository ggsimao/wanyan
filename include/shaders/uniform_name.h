#pragma once

#include <cstddef>
#include <format>

static constexpr std::size_t BUFFER_SIZE = 64;

class UniformName {
  public:
    // Forwarding the same argument twice is normally a red flag, since the
    // first use could move from it. It’s safe because the format functions only
    // read their arguments. It’s also needed: it keeps the argument types
    // matching fmt’s type, which plain args... wouldn’t.
    template <typename... Args>
    UniformName(std::format_string<Args...> fmt, Args&&... args) {
        _size = std::formatted_size(fmt, std::forward<Args>(args)...);
        if (_size > BUFFER_SIZE) {
            _fallback = std::format(fmt, std::forward<Args>(args)...);
        } else {
            std::format_to_n(_buffer, _size, fmt, std::forward<Args>(args)...);
        }
    }
    UniformName(const UniformName&) = delete;
    UniformName& operator=(const UniformName&) = delete;
    UniformName(UniformName&& other) = delete;
    UniformName& operator=(UniformName&& other) = delete;

    std::string_view view() const& {
        if (_size > BUFFER_SIZE) {
            return std::string_view(_fallback);
        } else {
            return std::string_view(_buffer, _size);
        }
    }
    std::string_view view() const&& = delete;

  private:
    char _buffer[BUFFER_SIZE];
    std::string _fallback;
    std::size_t _size;
};
