#include <iostream>
#include "game.hpp"
#include "SFML/Audio.hpp"

// Les variables globales swidth/sheight ont été déplacées dans game.cpp
// et sont devenues des constantes locales.

int main(){

    sf::SoundBuffer musicbuffer;
    if (!musicbuffer.loadFromFile("assets/sounds/music.mp3")) {
        std::cerr << "Erreur: Impossible de charger le son music.mp3" << std::endl;
    }
    sf::Sound sound(musicbuffer);
    // Initialisation du générateur de nombres aléatoires
    // (Important de le faire ici, avant le Game constructor)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    sound.play();
    sound.setLooping(true);
    Game game;
    game.run();
    
    return 0;
}