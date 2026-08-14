#include "E2Performance.hpp"
#include "MediaForgeMenu.hpp"
#include "VerdantShowcase.hpp"

#include <iostream>

int main(int argc, char** argv) {
    try {
        const auto run = aegis::mediaforge::parseE2RunConfiguration(argc, argv);
        return run.showcase ? aegis::mediaforge::runVerdantShowcase(run)
                            : aegis::mediaforge::runInteractiveMenu(run);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 64;
    }
}
