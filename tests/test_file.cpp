#include<iostream>
#include<SFML/Graphics.hpp>
#include"camera.hpp"
#include "map.hpp"
#include"utilitaires.hpp"
int main(){

    Camera camera(static_cast<float>(swidth), static_cast<float>(sheight), sf::Vector2f(swidth/2.f, sheight/2.f));
    sf::RenderWindow window(sf::VideoMode({swidth, sheight}), "Chunk System");
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
        afficher_grille_iso(window,swidth,sheight);
        window.display();
    }
    
    return 0;
}
