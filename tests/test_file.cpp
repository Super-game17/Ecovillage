#include<iostream>
#include<SFML/Graphics.hpp>
#include"camera.hpp"
#include "map.hpp"
#include"utilitaires.hpp"
#include "entity.hpp"

int main(){
    Camera camera(static_cast<float>(swidth), static_cast<float>(sheight), sf::Vector2f(swidth/2.f, sheight/2.f));
    sf::RenderWindow window(sf::VideoMode({swidth, sheight}), "Chunk System");
    Map Carte;
    Carte.initNoise();
    
    // Charger la texture des blocs
    sf::Texture textureBlocks;
    if(!textureBlocks.loadFromFile("assets/sprites/block.png")){
        std::cerr << "Erreur lors du chargement de la texture des blocs" << std::endl;
        return -1;
    }
    textureBlocks.setSmooth(true);
    
    // Créer l'entité joueur
    Entity player(0, 0);
    player.updateVisualPosition(Carte);
    sf::Clock gameClock;

    while (window.isOpen()){
        float deltaTime = gameClock.restart().asSeconds();

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
        
        // Mettre à jour l'entité avec deltaTime
        player.update(deltaTime);
        
        Carte.update(camera.getCenter(), textureBlocks);
        window.clear(sf::Color(153, 204, 255));
        
        // Dessiner tous les chunks visibles
        Carte.render(window, textureBlocks, player);
        
        //player.draw(window, Carte);
        player.HandleInput(window, Carte);
        window.display();
    }
    
    return 0;
}
