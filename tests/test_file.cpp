#include "entity.hpp"
#include "map.hpp"
#include "camera.hpp"
#include "utilitaires.hpp"

int main(){
    sf::RenderWindow window(sf::VideoMode({swidth, sheight}), "Entity Test");
    Map carte;
    Camera camera(swidth, sheight, {0.f,0.f});
    sf::Texture entityTexture;
    if (!entityTexture.loadFromFile("assets/sprites/DeerAtlas.png")) {
        std::cerr << "Erreur de chargement de la texture de l'entité !" << std::endl;
        return -1;
    }
    sf::Texture blocktex;
    if (!blocktex.loadFromFile("assets/sprites/block.png")) {
        std::cerr << "Erreur de chargement de la texture des blocks !" << std::endl;
        return -1;
    }
    Entity player(15, 25, EntityType::PLAYER, &entityTexture);
    player.updateVisualPosition(carte);
    
    // Valeurs d'ajustement
    float shapeOriginX = 50.f;
    float shapeOriginY = 50.f;
    float shadowOriginX = 40;
    float shadowOriginY = -18.5f;
   // float shadowOffsetY = 40.f;
    
    sf::Clock clock;

    while(window.isOpen()){
        float deltaTime = clock.restart().asSeconds();
        while(const auto event = window.pollEvent()){
            if(event->is<sf::Event::Closed>()){
                window.close();
            }
            camera.handleEvent(*event);
        }
        
        // === CONTRÔLES D'AJUSTEMENT EN TEMPS RÉEL ===
        
        // SHAPE ORIGIN
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Z)) {
            shapeOriginY -= 0.5f;
            player.shape.setOrigin({shapeOriginX, shapeOriginY});
            std::cout << "Shape Origin: (" << shapeOriginX << ", " << shapeOriginY << ")\n";
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::X)) {
            shapeOriginY += 0.5f;
            player.shape.setOrigin({shapeOriginX, shapeOriginY});
            std::cout << "Shape Origin: (" << shapeOriginX << ", " << shapeOriginY << ")\n";
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::C)) {
            shapeOriginX -= 0.5f;
            player.shape.setOrigin({shapeOriginX, shapeOriginY});
            std::cout << "Shape Origin: (" << shapeOriginX << ", " << shapeOriginY << ")\n";
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::V)) {
            shapeOriginX += 0.5f;
            player.shape.setOrigin({shapeOriginX, shapeOriginY});
            std::cout << "Shape Origin: (" << shapeOriginX << ", " << shapeOriginY << ")\n";
        }
        
        // SHADOW ORIGIN
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
            shadowOriginY -= 0.5f;
            player.shadow.setOrigin({shadowOriginX, shadowOriginY});
            std::cout << "Shadow Origin: (" << shadowOriginX << ", " << shadowOriginY << ")\n";
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
            shadowOriginY += 0.5f;
            player.shadow.setOrigin({shadowOriginX, shadowOriginY});
            std::cout << "Shadow Origin: (" << shadowOriginX << ", " << shadowOriginY << ")\n";
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
            shadowOriginX -= 0.5f;
            player.shadow.setOrigin({shadowOriginX, shadowOriginY});
            std::cout << "Shadow Origin: (" << shadowOriginX << ", " << shadowOriginY << ")\n";
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
            shadowOriginX += 0.5f;
            player.shadow.setOrigin({shadowOriginX, shadowOriginY});
            std::cout << "Shadow Origin: (" << shadowOriginX << ", " << shadowOriginY << ")\n";
        }
         
        // === FIN CONTRÔLES ===
        
        carte.update(camera.getCenter(), blocktex);
        window.clear(sf::Color::Cyan);
        player.update(deltaTime);
        camera.handleInput(window);
        player.HandleInput(window, carte);
        
        // Mettre à jour la position de l'ombre
        sf::Vector2f shadowPos = player.currentScreenPos;
        player.shadow.setPosition(shadowPos);
        
        camera.update(window);
        carte.render(window, blocktex, std::vector<Entity*>{}, &player);
        window.display();
    }
    return 0;
}