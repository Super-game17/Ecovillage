#include<iostream>
#include<SFML/Graphics.hpp>
#include"Grille_iso.hpp"


int main(){
    unsigned int width = 1280;
    unsigned int height = 720;
    const int cols = 10;
    const int rows = 10;
    const int tileWidth = 100;
    const int tileHeight = 50;

    sf::RenderWindow window (sf::VideoMode({width,height}), "Teeetet");
    sf::Texture texture;
    if(!texture.loadFromFile("assets/sprites/block.png")){
        std::cerr<<"Erreur lors du chargement de la texture"<<std::endl;
        return -1;
    }
    texture.setSmooth(true);

    sf::Sprite block(texture);
    while (window.isOpen()){

        while (const std::optional event = window.pollEvent()){

            if (event->is<sf::Event::Closed>()){

                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){

                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape){

                    window.close();
                }
            }

        
        window.clear(sf::Color::Black);

        
        for (int x = 0; x < cols; ++x) {
            for (int y = 0; y < rows; ++y) {
                float screenX = (width / 2.f) + (x - y) * (tileWidth / 2.f);
                float screenY = (height / 4.f) + (x + y) * (tileHeight / 2.f);

                block.setPosition(
                    {static_cast<float> (screenX), 
                    static_cast<float> (screenY)});
                window.draw(block);
            }
        }
        afficher_grille_iso(window, width, height);
        window.display();

        }
    }
    return 0;
}