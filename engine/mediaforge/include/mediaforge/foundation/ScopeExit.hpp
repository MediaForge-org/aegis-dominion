#pragma once

#include <concepts>
#include <utility>

namespace mf {

template <std::invocable Function>
class ScopeExit {
public:
    explicit ScopeExit(Function function) noexcept(std::is_nothrow_move_constructible_v<Function>)
        : function_(std::move(function)) {}
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit(ScopeExit&& other) noexcept(std::is_nothrow_move_constructible_v<Function>)
        : function_(std::move(other.function_)), active_(std::exchange(other.active_, false)) {}
    ~ScopeExit() { if (active_) { function_(); } }

    void release() noexcept { active_ = false; }

private:
    Function function_;
    bool active_{true};
};

template <typename Function>
ScopeExit(Function) -> ScopeExit<Function>;

} // namespace mf
