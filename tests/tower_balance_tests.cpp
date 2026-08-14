#include "Tower.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

struct TowerExpectation {
    TowerKind kind;
    TowerStats base;
    std::string_view branchAName;
    std::string_view branchBName;
    std::string_view branchADescription;
    std::string_view branchBDescription;
};

constexpr std::array towerExpectations{
    TowerExpectation{TowerKind::Pulse,
                     {.damage = 24, .range = 175, .cooldown = .42f, .projectileSpeed = 760,
                      .splash = 0, .slowFactor = 1, .slowDuration = 0, .chains = 1},
                     "Overclock", "Panzerbrecher", "Sehr viel höhere Feuerrate.",
                     "Mehr Schaden und Reichweite."},
    TowerExpectation{TowerKind::Cannon,
                     {.damage = 70, .range = 185, .cooldown = 1.35f, .projectileSpeed = 410,
                      .splash = 72, .slowFactor = 1, .slowDuration = 0, .chains = 1},
                     "Splitterladung", "Schweres Kaliber", "Deutlich größerer Explosionsradius.",
                     "Massiver Schaden pro Treffer."},
    TowerExpectation{TowerKind::Frost,
                     {.damage = 16, .range = 165, .cooldown = .62f, .projectileSpeed = 620,
                      .splash = 0, .slowFactor = .68f, .slowDuration = 1.7f, .chains = 1},
                     "Tiefkühlung", "Kristallsplitter", "Stärkerer und längerer Slow.",
                     "Mehr Direktschaden."},
    TowerExpectation{TowerKind::Sniper,
                     {.damage = 148, .range = 330, .cooldown = 1.85f, .projectileSpeed = 0,
                      .splash = 0, .slowFactor = 1, .slowDuration = 0, .chains = 1},
                     "Beschleuniger", "Exekutor", "Schnellere Schussfolge.",
                     "Extremer Einzelschaden."},
    TowerExpectation{TowerKind::Tesla,
                     {.damage = 41, .range = 185, .cooldown = .92f, .projectileSpeed = 0,
                      .splash = 0, .slowFactor = 1, .slowDuration = 0, .chains = 3},
                     "Relaisnetz", "Überladung", "Blitz springt auf mehr Ziele.",
                     "Höherer Schaden je Sprung."},
    TowerExpectation{TowerKind::Missile,
                     {.damage = 112, .range = 230, .cooldown = 1.75f, .projectileSpeed = 330,
                      .splash = 94, .slowFactor = 1, .slowDuration = 0, .chains = 1},
                     "Schwarm", "Sprengkopf", "Schnellere Raketenfolge.",
                     "Größere und stärkere Explosion."},
};

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void requireNear(float actual, float expected, const std::string& label) {
    const float tolerance = 0.00001f * std::max(1.f, std::abs(expected));
    if (std::abs(actual - expected) > tolerance)
        throw std::runtime_error(label + " changed: expected " + std::to_string(expected) +
                                 ", got " + std::to_string(actual));
}

void requireStats(const TowerStats& actual, const TowerStats& expected, const std::string& label) {
    requireNear(actual.damage, expected.damage, label + " damage");
    requireNear(actual.range, expected.range, label + " range");
    requireNear(actual.cooldown, expected.cooldown, label + " cooldown");
    requireNear(actual.projectileSpeed, expected.projectileSpeed, label + " projectileSpeed");
    requireNear(actual.splash, expected.splash, label + " splash");
    requireNear(actual.slowFactor, expected.slowFactor, label + " slowFactor");
    requireNear(actual.slowDuration, expected.slowDuration, label + " slowDuration");
    require(actual.chains == expected.chains,
            label + " chains changed: expected " + std::to_string(expected.chains) +
                ", got " + std::to_string(actual.chains));
}

TowerStats expectedAtLevel(TowerStats stats, int level) {
    stats.damage *= std::pow(1.42f, float(level - 1));
    stats.cooldown *= std::pow(.91f, float(level - 1));
    stats.range += 8.f * float(level - 1);
    stats.splash += 5.f * float(level - 1);
    return stats;
}

TowerStats expectedForBranch(TowerKind kind, TowerStats stats, UpgradeBranch branch) {
    switch (kind) {
        case TowerKind::Pulse:
            if (branch == UpgradeBranch::A) {
                stats.cooldown *= .70f;
                stats.damage *= 1.06f;
            } else {
                stats.damage *= 1.42f;
                stats.range *= 1.08f;
            }
            break;
        case TowerKind::Cannon:
            if (branch == UpgradeBranch::A) {
                stats.splash *= 1.5f;
                stats.damage *= .94f;
            } else {
                stats.damage *= 1.52f;
                stats.cooldown *= 1.10f;
            }
            break;
        case TowerKind::Frost:
            if (branch == UpgradeBranch::A) {
                stats.slowFactor = .48f;
                stats.slowDuration = 2.7f;
            } else {
                stats.damage *= 1.55f;
                stats.slowFactor = .62f;
            }
            break;
        case TowerKind::Sniper:
            if (branch == UpgradeBranch::A) {
                stats.cooldown *= .68f;
                stats.damage *= .96f;
            } else {
                stats.damage *= 1.72f;
                stats.cooldown *= 1.08f;
            }
            break;
        case TowerKind::Tesla:
            if (branch == UpgradeBranch::A) {
                stats.chains += 2;
                stats.damage *= .92f;
            } else {
                stats.damage *= 1.38f;
                stats.chains -= 1;
            }
            break;
        case TowerKind::Missile:
            if (branch == UpgradeBranch::A) {
                stats.cooldown *= .68f;
                stats.damage *= .9f;
                stats.splash *= .88f;
            } else {
                stats.damage *= 1.62f;
                stats.splash *= 1.3f;
                stats.cooldown *= 1.13f;
            }
            break;
    }
    return stats;
}

int expectedUpgradeCost(TowerKind kind, int level) {
    if (level >= 4) return 0;
    return static_cast<int>(towerCost(kind) * (0.72f + 0.38f * level));
}

void checkTower(const TowerExpectation& expectation) {
    const std::string label = towerName(expectation.kind);
    auto base = makeTower(expectation.kind, {137.f, 281.f});
    require(base->kind() == expectation.kind, label + " factory returned the wrong kind");
    require(base->position() == sf::Vector2f(137.f, 281.f), label + " constructor changed position");
    require(base->level() == 1 && base->branch() == UpgradeBranch::None, label + " initial upgrade state changed");
    require(base->targetMode() == TargetMode::First, label + " initial target mode changed");
    require(base->spent() == towerCost(expectation.kind), label + " initial cost/spend changed");
    require(base->sellValue() == static_cast<int>(towerCost(expectation.kind) * .72f),
            label + " initial sell value changed");
    require(base->upgradeCost() == expectedUpgradeCost(expectation.kind, 1), label + " level-1 upgrade cost changed");
    requireStats(base->stats(), expectation.base, label + " level 1");
    requireNear(base->range(), expectation.base.range, label + " range accessor");

    require(base->branchAName() == expectation.branchAName && base->branchBName() == expectation.branchBName &&
                base->branchADesc() == expectation.branchADescription &&
                base->branchBDesc() == expectation.branchBDescription,
            label + " branch copy changed");

    constexpr std::array targetModes{TargetMode::Strongest, TargetMode::Nearest, TargetMode::Last, TargetMode::First};
    for (const auto mode : targetModes) {
        base->cycleTargetMode();
        require(base->targetMode() == mode, label + " target-mode cycle changed");
    }

    const int firstUpgrade = base->upgradeCost();
    require(base->upgrade(), label + " level-2 upgrade failed");
    require(base->level() == 2 && base->needsBranch() && base->spent() == towerCost(expectation.kind) + firstUpgrade,
            label + " level-2 upgrade/spend/branch gate changed");
    require(base->sellValue() == static_cast<int>(base->spent() * .72f), label + " level-2 sell value changed");
    require(base->upgradeCost() == expectedUpgradeCost(expectation.kind, 2), label + " level-2 upgrade cost changed");
    requireStats(base->stats(), expectedAtLevel(expectation.base, 2), label + " level 2");
    const int gatedSpend = base->spent();
    require(!base->upgrade() && base->level() == 2 && base->spent() == gatedSpend,
            label + " branch gate accepted a branchless level-3 upgrade");

    for (const auto branch : {UpgradeBranch::A, UpgradeBranch::B}) {
        auto specialized = makeTower(expectation.kind, {});
        require(specialized->upgrade(), label + " specialization setup failed");
        const int secondUpgrade = specialized->upgradeCost();
        require(specialized->upgrade(branch), label + " specialization failed");
        require(specialized->level() == 3 && specialized->branch() == branch && !specialized->needsBranch(),
                label + " specialization state changed");
        require(specialized->spent() == towerCost(expectation.kind) + expectedUpgradeCost(expectation.kind, 1) +
                                            secondUpgrade,
                label + " specialization spend changed");
        requireStats(specialized->stats(), expectedForBranch(expectation.kind,
                                                             expectedAtLevel(expectation.base, 3), branch),
                     label + (branch == UpgradeBranch::A ? " level 3 branch A" : " level 3 branch B"));

        const int thirdUpgrade = specialized->upgradeCost();
        require(thirdUpgrade == expectedUpgradeCost(expectation.kind, 3) && specialized->upgrade(),
                label + " level-4 upgrade failed");
        require(specialized->maxLevel() && specialized->level() == 4 && specialized->upgradeCost() == 0,
                label + " max-level state changed");
        requireStats(specialized->stats(), expectedForBranch(expectation.kind,
                                                             expectedAtLevel(expectation.base, 4), branch),
                     label + (branch == UpgradeBranch::A ? " level 4 branch A" : " level 4 branch B"));
        const int maxSpend = specialized->spent();
        require(!specialized->upgrade() && specialized->spent() == maxSpend,
                label + " accepted an upgrade past level 4");
        require(specialized->sellValue() == static_cast<int>(maxSpend * .72f),
                label + " max-level sell value changed");
    }
}

} // namespace

int main() {
    try {
        for (const auto& expectation : towerExpectations) checkTower(expectation);
        std::cout << "Tower balance, upgrades, costs, targeting and branch copy checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
