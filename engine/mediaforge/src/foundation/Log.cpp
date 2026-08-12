#include <mediaforge/foundation/Log.hpp>

#include <iostream>
#include <mutex>
#include <utility>

namespace mf {
namespace {

std::mutex sinkMutex;
LogSink currentSink;

const char* levelName(LogLevel level) {
    switch (level) {
    case LogLevel::debug: return "DEBUG";
    case LogLevel::info: return "INFO";
    case LogLevel::warning: return "WARNING";
    case LogLevel::error: return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace

void setLogSink(LogSink sink) {
    const std::scoped_lock lock(sinkMutex);
    currentSink = std::move(sink);
}

void log(LogLevel level, std::string_view message) {
    const std::scoped_lock lock(sinkMutex);
    if (currentSink) {
        currentSink(level, message);
        return;
    }
    std::clog << "[MediaForge][" << levelName(level) << "] " << message << '\n';
}

} // namespace mf
