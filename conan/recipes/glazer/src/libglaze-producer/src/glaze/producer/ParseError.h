#pragma once

#include <stdexcept>
#include <string>

namespace glaze::producer {

/**
 * @brief Exception thrown when gl.xml cannot be parsed into a Registry.
 * Extended description: wraps both I/O failures (file not found, unreadable)
 * and structural failures (missing <registry> root, malformed command, etc.).
 * Inherits std::runtime_error so CLI-level handlers can catch any code path
 * with a single catch(std::exception) fallback while still differentiating
 * parser problems from other errors via a dynamic_cast where needed.
 */
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string &message);
};

} // namespace glaze::producer
