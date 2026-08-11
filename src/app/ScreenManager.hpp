#pragma once

#include "Screen.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace aegis::app {

class ScreenManager {
public:
    using Factory = std::function<std::unique_ptr<Screen>(const ScreenRequest&)>;

    explicit ScreenManager(Factory factory);

    Screen* current();
    const Screen* current() const;

    void replace(ScreenRequest request);
    void push(ScreenRequest request);
    void pop();
    void quit();
    void applyPending();
    bool quitRequested() const { return quitRequested_; }

private:
    enum class Operation { None, Replace, Push, Pop, Quit };
    Factory factory_;
    std::vector<std::unique_ptr<Screen>> stack_;
    Operation pendingOperation_ = Operation::None;
    ScreenRequest pendingRequest_;
    bool quitRequested_ = false;
};

} // namespace aegis::app
