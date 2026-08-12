#include "Theme.hpp"

namespace aegis::ui {

const Theme& Theme::command() {
    static const Theme theme{
        {{7, 13, 21}, {12, 21, 31, 242}, {18, 30, 43, 245}, {55, 84, 102},
         {91, 210, 255}, {203, 139, 255}, {255, 140, 78}, {92, 220, 143},
         {255, 197, 76}, {255, 92, 107}, {224, 235, 244}, {137, 158, 176}, {76, 91, 104}},
        {4.f, 8.f, 16.f, 24.f, 40.f},
        {34.f, 46.f, 58.f, 20.f, 6.f, 1.5f, 14.f, 20.f, 28.f},
        {18, 13, 22, 32, 46}};
    return theme;
}

bool Theme::complete() const {
    return spacing.xs > 0.f && spacing.s > spacing.xs && spacing.m > spacing.s && spacing.l > spacing.m &&
           spacing.xl > spacing.l && sizes.button > 0.f && sizes.panelPadding > 0.f && typography.body > 0;
}

} // namespace aegis::ui
