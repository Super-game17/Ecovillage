#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "map.hpp"
#include <random>
#include <cmath>
#include<vector>

enum  class EntityType{PLAYER,PREY,PREDATOR,FOOD};

// Classe de base pour toutes les créatures
class Entity {
public:
    EntityType type;
    int gridX, gridY; 
    sf::RectangleShape shape;
    sf::CircleShape shadow;
    sf::CircleShape selectionCircle;
    bool isAlive=true;
    bool isSelected = false;

    // États de l'entité
    enum State {
        IDLE,
        WANDERING,
        SEEKING_FOOD,
        SEEKING_WATER,
        FLEEING,
        HUNTING,
        DRINKING,
        EATING
    };
    State currentState = IDLE;

    float moveCooldown = 0.f;
    float MOVE_DELAY = 0.45f; // Délai entre les mouvements (ralenti)

    sf::Vector2f currentScreenPos;
    sf::Vector2f targetScreenPos;
    float speed = 10.0f;

    Entity(int startX, int startY, EntityType t) ;
   
    virtual ~Entity() = default;

    sf::Vector2i getPosition(){return sf::Vector2i(gridX, gridY);}
    int getDepth() const { return gridX + gridY; }

    // Déplacement générique (utilisé par Joueur et IA)
    bool move(int dx, int dy, const Map& map);
       

    void updateVisualPosition(const Map& map);
    virtual void draw(sf::RenderTarget& target, const Map& Carte) const;
        

    virtual void update(float deltaTime); 
    // Input manuel (pour le joueur uniquement)
    void HandleInput(const sf::RenderWindow& window, const Map& Carte);
    Entity* scanForTarget(const std::vector<Entity*>& entities, EntityType targetType, int radius);
    sf::Vector2i scanForWater(const Map& map, int radius);
    void Entity::moveTowards(int tx, int ty, const Map& map);
    void setSelected(bool selected);
    bool checkClick(int mouseX, int mouseY, const Map& carte) const;
    std::string getStateString() const;
    virtual std::string getStats() const;
    void updateSelectionVisual();

};


// --- Classe Nourriture -----
class Food : public Entity {
public:
    Food(int x, int y) : Entity(x, y, EntityType::FOOD) {
        // Apparence d'une baie ou d'un buisson
        shape.setSize({10.f, 10.f});
        shape.setOrigin({5.f, 5.f});
        shape.setFillColor(sf::Color(255, 50, 50)); // Rouge
        
        shadow.setRadius(5.f);
        shadow.setOrigin({5.f, 2.5f});

}
};

// --- CLASSE PROIE ---
class Prey : public Entity {
public:
    // Attributs biologiques
    float hunger =0;       // 0 = rassasié, 100 = mort de faim
    float thirst =0;       // 0 = hydraté, 100 = mort de soif
    float energy=100;       // 100 = pleine forme, 0 = épuisé (doit dormir)
    float urgeToReproduce=0; // Augmente avec le temps

    // Attributs génétiques (fixes pour une entité)
    float maxSpeed;     // Influence le MOVE_DELAY
    float visionRadius; // Distance de vue en cases

    // IA
    float aiTickTimer = 0.f;


    Prey(int x, int y) ; 
    void HandleInput(const sf::RenderWindow& window, const Map& Carte, const std::vector<Entity*>& entities);

    void update(float deltaTime) override ;
       
    // Méthode principale de l'IA
    void thinkAndAct(const Map& carte , const std::vector<Entity*>& entities) ;
    std::string getStats() const override;
    void determineState();
        
};

class Predator : public Entity {
    public:
    float hunger =0;       // 0 = rassasié, 100 = mort de faim
    float thirst =0;       // 0 = hydraté, 100 = mort de soif
    float energy=100;       // 100 = pleine forme, 0 = épuisé (doit dormir)
    float urgeToReproduce=0; // Augmente avec le temps

    // Attributs génétiques (fixes pour une entité)
    float maxSpeed;     // Influence le MOVE_DELAY
    float visionRadius; // Distance de vue en cases

    // IA
    float aiTickTimer = 0.f;
    
    Predator(int x,int y);
    void update(float deltaTime) override ;
       
    // Méthode principale de l'IA
    void thinkAndAct(const Map& carte , const std::vector<Entity*>& entities) ;

    std::string getStats() const override;
    void determineState();


};

#endif //ENTITY_HPP