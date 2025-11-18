#include<iostream>
#include<SFML/Graphics.hpp>
#include"Grille_iso.hpp"
#include"camera.hpp"
#include "FastNoiseLite.h"
#include "map.hpp"

int main(){
    unsigned int width = 1280;
    unsigned int height = 720;
    Camera camera(static_cast<float>(width), static_cast<float>(height), sf::Vector2f(width/2.f, height/2.f));
    sf::RenderWindow window(sf::VideoMode({width, height}), "Chunk System");
    Map Carte;
    sf::Texture texture;
    if(!texture.loadFromFile("assets/sprites/block.png")){
        std::cerr << "Erreur lors du chargement de la texture" << std::endl;
        return -1;
    }
    texture.setSmooth(true);
    
    while (window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()){
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape){
                    window.close();
                }
                // Augmenter/diminuer la distance de rendu
                else if(keyPressed->scancode == sf::Keyboard::Scancode::Up){
                    Carte.renderDistance++;
                    std::cout << "Render distance: " << Carte.renderDistance << std::endl;
                }
                else if(keyPressed->scancode == sf::Keyboard::Scancode::Down && Carte.renderDistance > 1){
                    Carte.renderDistance--;
                    std::cout << "Render distance: " << Carte.renderDistance << std::endl;
                }
            }
            camera.handleEvent(*event);
        }

        camera.handleInput(window);
        camera.update(window);
        
        Carte.update(camera.getCenter(), texture);
        window.clear(sf::Color(153, 204, 255));
        
        // Dessiner tous les chunks visibles
        Carte.render(window, texture);

        window.display();
    }
    
    return 0;
}
