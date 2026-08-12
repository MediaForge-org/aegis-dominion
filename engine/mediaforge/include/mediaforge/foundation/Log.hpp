#pragma once

#include <functional>
#include <string_view>

namespace mf {

enum class LogLevel { debug, info, warning, error };
using LogSink = std::function<void(LogLevel, std::string_view)>;

void setLogSink(LogSink sink);
void log(LogLevel level, std::string_view message);

} // namespace mf
