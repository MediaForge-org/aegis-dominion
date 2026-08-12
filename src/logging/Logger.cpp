#include "Logger.hpp"

#include <iostream>

namespace aegis::logging {

Logger::Logger(std::ostream& output) : output_(&output) {}

std::string_view toString(Level level) {
    switch (level) {
        case Level::Debug: return "DEBUG";
        case Level::Info: return "INFO";
        case Level::Warning: return "WARNING";
        case Level::Error: return "ERROR";
    }
    return "UNKNOWN";
}

void Logger::write(Level level, std::string_view message, const std::source_location& location) {
    if (!enabled(level)) return;
    std::scoped_lock lock(mutex_);
    *output_ << '[' << toString(level) << "] " << message;
    if (level >= Level::Warning)
        *output_ << " (" << location.file_name() << ':' << location.line() << ", " << location.function_name() << ')';
    *output_ << '\n';
    output_->flush();
}

Logger& log() {
    static Logger instance(std::clog);
    return instance;
}

} // namespace aegis::logging
