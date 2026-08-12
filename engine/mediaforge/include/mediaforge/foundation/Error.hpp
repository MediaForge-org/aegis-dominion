#pragma once

#include <expected>
#include <string>
#include <utility>

namespace mf {

enum class ErrorCode {
    invalidArgument,
    initializationFailed,
    platformError,
    gpuError,
    ioError,
    unsupported,
};

struct Error {
    ErrorCode code{ErrorCode::initializationFailed};
    std::string message;
};

template <typename T>
using Result = std::expected<T, Error>;

using Status = Result<void>;

inline std::unexpected<Error> fail(ErrorCode code, std::string message) {
    return std::unexpected(Error{code, std::move(message)});
}

} // namespace mf
