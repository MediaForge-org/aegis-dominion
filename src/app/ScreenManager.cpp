#include "ScreenManager.hpp"

#include <utility>

namespace aegis::app {

ScreenManager::ScreenManager(Factory factory) : factory_(std::move(factory)) {}

Screen* ScreenManager::current() { return stack_.empty() ? nullptr : stack_.back().get(); }
const Screen* ScreenManager::current() const { return stack_.empty() ? nullptr : stack_.back().get(); }

void ScreenManager::replace(ScreenRequest request) {
    pendingOperation_ = Operation::Replace;
    pendingRequest_ = std::move(request);
}

void ScreenManager::push(ScreenRequest request) {
    pendingOperation_ = Operation::Push;
    pendingRequest_ = std::move(request);
}

void ScreenManager::pop() { pendingOperation_ = Operation::Pop; }
void ScreenManager::quit() { pendingOperation_ = Operation::Quit; }

void ScreenManager::applyPending() {
    const auto operation = pendingOperation_;
    pendingOperation_ = Operation::None;
    if (operation == Operation::None) return;
    if (operation == Operation::Quit) {
        quitRequested_ = true;
        return;
    }
    if (operation == Operation::Pop) {
        if (!stack_.empty()) stack_.pop_back();
        if (stack_.empty()) quitRequested_ = true;
        else stack_.back()->onResume();
        return;
    }

    auto screen = factory_(pendingRequest_);
    if (!screen) {
        quitRequested_ = true;
        return;
    }
    if (operation == Operation::Replace) stack_.clear();
    stack_.push_back(std::move(screen));
}

} // namespace aegis::app
