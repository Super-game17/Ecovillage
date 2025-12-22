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

    //Attributs de l'entité
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
        HUNTING
    };
    State currentState = IDLE;

    float moveCooldown = 0.f;
    float MOVE_DELAY = 0.45f; // Délai entre les mouvements (ralenti)

    sf::Vector2f currentScreenPos;
    sf::Vector2f targetScreenPos;
    float speed = 10.0f;

    // === ANIMATION DE DÉGÂTS ===
    bool isDamaged = false;           // Est-ce que l'entité vient de prendre des dégâts ?
    float damageFlashTimer = 0.f;     // Timer pour le clignotement
    float damageFlashDuration = 0.4f; // Durée totale du clignotement (0.4 secondes)
    sf::Color originalColor;          // Couleur originale pour revenir après
    
    // === ANIMATION DE MORT ===
    bool isDying = false;             // En train de mourir (animation en cours)
    float deathAnimTimer = 0.f;       // Timer pour l'animation de mort
    float deathAnimDuration = 0.8f;   // Durée de l'animation (0.8 secondes)
    
    // Méthodes d'animation
    void updateDamageAnimation(float deltaTime);
    void updateDeathAnimation(float deltaTime);
    void triggerDamageFlash(); // À appeler quand l'entité prend des dégâts

    Entity(int startX, int startY , EntityType t, sf::Texture *texture = nullptr) ;

    sf::Vector2i getPosition(){return sf::Vector2i(gridX, gridY);}
    int getDepth() const { return gridX + gridY; }

    // Déplacement générique (utilisé par Joueur et IA)
    bool move(int dx, int dy, const Map& map);
    void updateTextureDirection(int dx, int dy);  // Adapte la texture en fonction de la direction dominante
       

    void updateVisualPosition(const Map& map);
    virtual void draw(sf::RenderTarget& target, const Map& Carte) const;
        

    virtual void update(float deltaTime); 
    // Input manuel (pour le joueur uniquement)
    void HandleInput(const sf::RenderWindow& window, const Map& Carte);
    Entity* scanForTarget(const std::vector<Entity*>& entities, EntityType targetType, int radius);
    sf::Vector2i scanForWater(const Map& map, int radius);
    void Entity::moveTowards(int tx, int ty, const Map& map);
    void setSelected(bool selected);
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

    float aiTickTimer = 0.f;
    // Attributs biologiques
    float health = 100.f;     // 0 = mort
    float hunger = 0;         // 0 = rassasié, 100 = mort de faim
    float thirst = 0;         // 0 = hydraté, 100 = mort de soif
    float energy = 100.f;     // 100 = pleine forme, 0 = épuisé

    // Attributs génétiques (héréditaires)
    float maxHealth;
    float maxEnergy;
    float maxSpeed;           // Vitesse de base
    float sprintSpeedMultiplier; // x1.5 à x2.0 en sprint
    float hungerRate;         // Vitesse de consommation
    float energyRecoveryRate; // Vitesse de récupération au repos
    float visionRadius; //Distance de vue en cases
    // États de reproduction
    bool canReproduce = false;
    float reproductiveUrge = 0.f; // Augmente quand bien nourri/hydraté

    Prey(int x, int y, sf::Texture *texture = nullptr,
         float genHealth = 100.f, float genEnergy = 100.f, 
         float genSpeed = 1.0f, float genHungerRate = 1.0f, float genVisionRadius = 9.0f);
    
    // Méthode principale de l'IA
    void update(float deltaTime) override;
    void thinkAndAct(const Map& carte, const std::vector<Entity*>& entities);
    std::string getStats() const override;
    void determineState();
    
    // Systèmes d'énergie et de sprint
    bool canSprint() const;
    void sprintMove(int dx, int dy, const Map& carte);
    void recoverEnergy(float deltaTime);
    void takeDamage(float amount); // Réduit health
    
    // Reproduction
    Prey* reproduce(Prey* partner); // Retourne le bébé
    void applyGeneticMutation(float mutationRate = 0.1f);
    
    void HandleInput(const sf::RenderWindow& window, const Map& Carte, const std::vector<Entity*>& entities);

};

class Predator : public Entity {
public:
    float aiTickTimer = 0.f;

    float health = 100.f;
    float hunger = 0;
    float thirst = 0;
    float energy = 100.f;

    // Attributs génétiques
    float maxHealth;
    float maxEnergy;
    float maxSpeed;
    float sprintSpeedMultiplier;
    float hungerRate;
    float energyRecoveryRate;
    float visionRadius;

    bool canReproduce = false;
    float reproductiveUrge = 0.f;

    Predator(int x, int y, sf::Texture *texture = nullptr,
             float genHealth = 120.f, float genEnergy = 120.f,
             float genSpeed = 1.2f, float genHungerRate = 1.5f, float genVisionRadius = 9.0f);
    
    void update(float dt) override;
    void thinkAndAct(const Map& carte, const std::vector<Entity*>& entities);
    std::string getStats() const override;
    void determineState();
    
    bool canSprint() const;
    void sprintMove(int dx, int dy, const Map& carte);
    void recoverEnergy(float deltaTime);
    void takeDamage(float amount);
    
    Predator* reproduce(Predator* partner);
    void applyGeneticMutation(float mutationRate = 0.1f);
};

#endif //ENTITY_HPP