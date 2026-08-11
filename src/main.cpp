#include "Game.hpp"
#include <exception>
#include <iostream>
int main(){ try{ Game game; return game.run(); } catch(const std::exception& e){ std::cerr<<"Fataler Fehler: "<<e.what()<<"\n"; return 1; } }
