#pragma once

#include <format>
#include <iosfwd>
#include <mutex>
#include <source_location>
#include <string>
#include <string_view>

namespace aegis::logging {

enum class Level { Debug, Info, Warning, Error };

struct Message {
    std::string_view format;
    std::source_location location;
    consteval Message(const char* value, const std::source_location& source = std::source_location::current())
        : format(value), location(source) {}
};

class Logger {
public:
    explicit Logger(std::ostream& output);

    void setLevel(Level level) { minimum_ = level; }
    Level level() const { return minimum_; }
    bool enabled(Level level) const { return level >= minimum_; }
    void write(Level level, std::string_view message,
               const std::source_location& location = std::source_location::current());

    template <typename... Args>
    void debug(Message message, Args&&... args) {
        write(Level::Debug, std::vformat(message.format, std::make_format_args(args...)), message.location);
    }
    template <typename... Args>
    void info(Message message, Args&&... args) {
        write(Level::Info, std::vformat(message.format, std::make_format_args(args...)), message.location);
    }
    template <typename... Args>
    void warning(Message message, Args&&... args) {
        write(Level::Warning, std::vformat(message.format, std::make_format_args(args...)), message.location);
    }
    template <typename... Args>
    void error(Message message, Args&&... args) {
        write(Level::Error, std::vformat(message.format, std::make_format_args(args...)), message.location);
    }

private:
    std::ostream* output_;
    Level minimum_ = Level::Info;
    mutable std::mutex mutex_;
};

Logger& log();
std::string_view toString(Level level);

} // namespace aegis::logging
