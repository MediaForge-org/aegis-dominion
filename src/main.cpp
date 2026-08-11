#include "app/Application.hpp"
#include <exception>
#include <iostream>
int main(){ try{ aegis::app::Application application; return application.run(); } catch(const std::exception& e){ std::cerr<<"Fataler Fehler: "<<e.what()<<"\n"; return 1; } }
