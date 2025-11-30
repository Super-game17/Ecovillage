
#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "SFML/Graphics.hpp"
#include "map.hpp"

class Entity {
public:
    // Coordonnées sur la grille (Entiers)
    int gridX, gridY; 
    
    sf::RectangleShape shape;
    sf::CircleShape shadow;

    // Cooldown pour éviter les déplacements trop rapides
    float moveCooldown = 0.f;
    const float MOVE_DELAY = 0.2f; // délai entre deux mouvements (en secondes)

    // Constructeur
    Entity(int startX, int startY): gridX(startX), gridY(startY) 
    {
        // Configuration visuelle
        shape.setSize(sf::Vector2f(20.f, 40.f));
        shape.setOrigin({10.f, 40.f});
        shape.setFillColor(sf::Color::Red);

        shadow.setRadius(10.f);
        shadow.setOrigin({10.f, 5.f});
        shadow.setScale({1.f, 0.5f});
        shadow.setFillColor(sf::Color(0, 0, 0, 100));
    }

    sf::Vector2i getPosition(){return sf::Vector2i(gridX, gridY);}

    // Déplacer le joueur d'une case
    void move(int dx, int dy, const Map& map){
        int targetX = gridX + dx;
        int targetY = gridY + dy;
        if (map.isObstacle(targetX, targetY)){
            // Ne pas bouger si obstacle
            return;
        }
        gridX = targetX;
        gridY = targetY;
        
        updateVisualPosition(map);
        moveCooldown = MOVE_DELAY; // réinitialiser le cooldown
    }

    // Mettre à jour la position visuelle basée sur gridX, gridY
    void updateVisualPosition(const Map& map) {
        // 1. Récupérer la hauteur du sol à cette position
        int zLevel = map.getGroundLevel(static_cast<float>(gridX), static_cast<float>(gridY));

        auto [screenX, screenY] = isoToScreen(gridX, gridY, zLevel, tileWidth, tileHeight, swidth, sheight);

        // Appliquer la position
        shape.setPosition({screenX, screenY + 25.f});
        shadow.setPosition({screenX, screenY + 25.f});
        //Logs pour debug
        std::cout<<"===========================================================\n";
        std::cout << "Entity moved to (" << gridX << ", " << gridY << ")\n";
        std::cout << "Screen position: (" << screenX << ", " << screenY + 25.f << ")\n";
        std::cout << "Ground level (z): " << zLevel << "\n";
    }

    void draw(sf::RenderTarget& target, const Map& Carte) const {
        // Dessiner l'entité avec l'offset global
        sf::RenderStates states;
        states.transform.translate(Carte.chunkOrigin);
        target.draw(shadow, states);
        target.draw(shape, states);
    }

    // Mettre à jour le cooldown (appeler une fois par frame avec deltaTime)
    void update(float deltaTime) {
        if (moveCooldown > 0.f) {
            moveCooldown -= deltaTime;
        }
    }

    void HandleInput(const sf::RenderWindow& window, const Map& Carte){
        // Ne bouger que si le cooldown a expiré
        if (moveCooldown > 0.f) return;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::I)) {
            this->move(0, -1, Carte);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::K)) {
            this->move(0, 1, Carte);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::J)) {
            this->move(-1, 0, Carte);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::L)) {
            this->move(1, 0, Carte);
        }
    }

    int getDepth() const {

        return gridX + gridY;
    }
};

#endif //ENTITY_HPP