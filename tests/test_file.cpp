#include <iostream>
#include "game.hpp"

// Les variables globales swidth/sheight ont été déplacées dans game.cpp
// et sont devenues des constantes locales.

int main(){
    // Initialisation du générateur de nombres aléatoires
    // (Important de le faire ici, avant le Game constructor)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    Game game;
    game.run();
    
    return 0;
}