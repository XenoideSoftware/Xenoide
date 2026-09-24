#include "glaze/producer/ParseError.h"

namespace glaze::producer {

ParseError::ParseError(const std::string &message) : std::runtime_error(message) {}

} // namespace glaze::producer
