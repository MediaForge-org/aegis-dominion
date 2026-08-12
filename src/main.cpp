#include "app/Application.hpp"
#include "logging/Logger.hpp"
#include <exception>
int main(){ try{ aegis::app::Application application; return application.run(); } catch(const std::exception& e){ aegis::logging::log().error("Fatal error: {}", e.what()); return 1; } }
