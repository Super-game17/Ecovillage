#include "entity.hpp"

Entity::Entity(int startX, int startY, EntityType t, sf::Texture *texture):gridX(startX),gridY(startY),type(t) {
        if(texture ==nullptr){
        // Configuration visuelle par défaut
        shape.setSize(sf::Vector2f(20.f, 40.f));
        shape.setOrigin({10.f, 40.f});
        shape.setFillColor(sf::Color::White); // Blanc par défaut
        shadow.setOrigin({10.f, 5.f});
        shadow.setRadius(10.f);
        }
        else {
            shape.setSize({100,100});
            shape.setTexture(texture);
            shape.setOrigin({50.f, 50.f});
            shadow.setOrigin({40.f,-18.5f});
            shadow.setRadius(35.f);
        }

        
        shadow.setScale({1.f, 0.5f});
        shadow.setFillColor(sf::Color(0, 0, 0, 100));

        // Configuration du cercle de sélection
        selectionCircle.setRadius(25.f);
        selectionCircle.setOrigin({25.f, 25.f});
        selectionCircle.setFillColor(sf::Color::Transparent);
        selectionCircle.setOutlineThickness(2.f);
        selectionCircle.setOutlineColor(sf::Color::Yellow);

        currentScreenPos = {0.f, 0.f};
        targetScreenPos = {0.f, 0.f};
    }


 bool Entity::move(int dx, int dy, const Map& carte){
        if (moveCooldown > 0.f || !isAlive) return false;

        int targetX = gridX + dx;
        int targetY = gridY + dy;
        
        if (carte.isObstacle(targetX, targetY)) return false;

        gridX = targetX;
        gridY = targetY;

        updateTextureDirection(dx, dy);
        
        int zLevel = carte.getGroundLevel(static_cast<float>(gridX), static_cast<float>(gridY));
        auto [screenX, screenY] = isoToScreen(gridX, gridY, zLevel, tileWidth, tileHeight, swidth, sheight);
        
        targetScreenPos = {screenX, screenY};
        moveCooldown = MOVE_DELAY;
        return true;
    }

void Entity::updateTextureDirection(int dx, int dy) {
    // Si pas de texture, ignorer
    if (shape.getTexture() == nullptr) return;
    
    // Déterminer la direction dominante
    if (dy < 0) {
        // Haut (N)
        shape.setTextureRect({{200, 0}, {100, 100}});
    } 
    else if (dy > 0) {
        // Bas (S)
        shape.setTextureRect({{0, 0}, {100, 100}});
    } 
    else if (dx < 0) {
        // Gauche (W)
        shape.setTextureRect({{300, 0}, {100, 100}});
    } 
    else if (dx > 0) {
        // Droite (E)
        shape.setTextureRect({{100, 0}, {100, 100}});
    }
}

void Entity::updateVisualPosition(const Map& carte) {
        int zLevel = carte.getGroundLevel(static_cast<float>(gridX), static_cast<float>(gridY));
        auto [screenX, screenY] = isoToScreen(gridX, gridY, zLevel, tileWidth, tileHeight, swidth, sheight);
        targetScreenPos = {screenX, screenY};
        currentScreenPos = targetScreenPos;
        shape.setPosition(currentScreenPos);
        shadow.setPosition(currentScreenPos);
    }

void Entity::update(float deltaTime){
        if (moveCooldown > 0.f) moveCooldown -= deltaTime;
        
        sf::Vector2f diff = targetScreenPos - currentScreenPos;
        float distSq = diff.x*diff.x + diff.y*diff.y;
        
        if (distSq > 0.5f) {
            float t = deltaTime * speed; 
            currentScreenPos += diff * t;
        }
        else
        {
        currentScreenPos = targetScreenPos;
        }
        shape.setPosition(currentScreenPos);
        shadow.setPosition(currentScreenPos);
        if (isSelected) {
        updateSelectionVisual();
        }
    }

void Entity::draw(sf::RenderTarget& target, const Map& Carte) const{
        if (!isAlive) return ;
        sf::RenderStates states;
        states.transform.translate(Carte.chunkOrigin);
        target.draw(shadow, states);
        target.draw(shape, states);

        if(isSelected){
            target.draw(selectionCircle, states);
        }
    }

// Input manuel (pour le joueur uniquement)
void Entity::HandleInput(const sf::RenderWindow& window, const Map& Carte){
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::I)) this->move(0, -1, Carte);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::K)) this->move(0, 1, Carte);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::J)) this->move(-1, 0, Carte);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::L)) this->move(1, 0, Carte);
    }


void Entity::setSelected(bool selected) {
    isSelected = selected;
    updateSelectionVisual();
}

void Entity::updateSelectionVisual() {
    if (isSelected) {
        selectionCircle.setOutlineColor(sf::Color::Yellow);
        selectionCircle.setOutlineThickness(3.f);
        // Positionner l'anneau au pied de l'entité
        selectionCircle.setPosition({currentScreenPos.x, currentScreenPos.y});
        // Ajouter aussi un contour sur le shape pour plus de visibilité
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Yellow);
    }
    else {
        shape.setOutlineThickness(0.f);
    }
}

std::string Entity::getStateString() const {
    switch(currentState) {
        case IDLE: return "Idle";
        case WANDERING: return "Wandering";
        case SEEKING_FOOD: return "Seeking Food";
        case SEEKING_WATER: return "Seeking Water";
        case FLEEING: return "Fleeing";
        case HUNTING: return "Hunting";
        default: return "Unknown";
    }
}

std::string Entity::getStats() const {
    return "Entity Base Stats";
}
// --- UTILITAIRES IA ---

int distManhattan(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}
// Chercher une entité (Nourriture pour Proie, Proie pour Prédateur)
Entity* Entity::scanForTarget(const std::vector<Entity*>& entities, EntityType targetType, int radius) {
    Entity* best = nullptr;
    int minDst = 9999;

    for (auto* e : entities) {
        if (e == this || !e->isAlive || e->type != targetType) continue;
        
        int d = distManhattan(gridX, gridY, e->gridX, e->gridY);
        if (d <= radius && d < minDst) {
            minDst = d;
            best = e;
        }
    }
    return best;
}

sf::Vector2i Entity::scanForWater(const Map& carte, int radius) {
    sf::Vector2i best = {-1, -1};// Indique qu'aucune eau n'a été trouvée
    int minDst = 9999;

    for(int y = gridY - radius; y <= gridY + radius; y++){
        for(int x = gridX - radius; x <= gridX + radius; x++){
            if(carte.isWater(x, y)){ // Utilise la méthode isWater de Map
                int d = distManhattan(gridX, gridY, x, y);
                if(d < minDst){
                    minDst = d;
                    best = {x, y};
                }
            }
        }
    }
    return best;
}

void Entity::moveTowards(int tx, int ty, const Map& carte) {
    if(gridX == tx && gridY == ty) return;
    int dx = tx - gridX;
    int dy = ty - gridY;
    
    // Essayer de bouger sur l'axe le plus éloigné
    if(std::abs(dx) > std::abs(dy)){
        if(move(dx > 0 ? 1 : -1, 0, carte)) return;
        if(move(0, dy > 0 ? 1 : -1, carte)) return;
    } else {
        if(move(0, dy > 0 ? 1 : -1, carte)) return;
        if(move(dx > 0 ? 1 : -1, 0, carte)) return;
    }
    // Si bloqué, aléatoire
    move((rand()%3)-1, (rand()%3)-1, carte);
}

// ========== PREY ==========

Prey::Prey(int x, int y, sf::Texture *texture,
           float genHealth, float genEnergy, float genSpeed, float genHungerRate, float genVisionRadius)
    : Entity(x, y, EntityType::PREY),
      maxHealth(genHealth), maxEnergy(genEnergy), 
      maxSpeed(genSpeed), hungerRate(genHungerRate),
      sprintSpeedMultiplier(1.5f), energyRecoveryRate(15.f) {
    
    health = maxHealth;
    energy = maxEnergy;
    MOVE_DELAY = 0.4f / maxSpeed; // Plus rapide = moins de délai
    
    if(texture == nullptr) {
        //Visuel par défaut différent pour la proie, ici c'est un vert clair
        shape.setFillColor(sf::Color(100, 255, 100));
        shape.setSize(sf::Vector2f(15.f, 30.f)); //Plus petit que le joueur par défaut
        shape.setOrigin({7.5f, 30.f});
    } else {
        shape.setSize({100, 100});
        shape.setTexture(texture);
        shape.setOrigin({50.f, 50.f});
        shadow.setOrigin({40.f,-18.5f});
        shadow.setRadius(35.f);
    }
}

bool Prey::canSprint() const {
    return energy > 20.f && health > 30.f;
}

void Prey::HandleInput(const sf::RenderWindow& window, const Map& Carte, const std::vector<Entity*>& entities) {
    Entity::HandleInput(window, Carte);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)) {
        Entity* food = scanForTarget(entities, EntityType::FOOD, 1);
        if (food) {
            food->isAlive = false; // Manger la nourriture
            hunger -= 30.f; // Réduit la faim
            if (hunger < 0.f) hunger = 0.f;
            std::cout << "Prey at (" << gridX << ", " << gridY << ") ate food.\n";
        }
        else {
            std::cout << "No food to eat nearby.\n";
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::R)) {
        sf::Vector2i water = scanForWater(Carte, 1);
        if (water.x != -1) {
            if(distManhattan(gridX, gridY, water.x, water.y) <= 1){
            thirst -= 30.f; // Réduit la soif
            if (thirst < 0.f) thirst = 0.f;
            std::cout << "Prey at (" << gridX << ", " << gridY << ") drank water.\n";}
        }
        else {
            std::cout << "No water to drink nearby.\n";
        }
    }
}

void Prey::sprintMove(int dx, int dy, const Map& carte) {
    if (!canSprint()) {
        move(dx, dy, carte);
        return;
    }
    
    // Sprint coûte 2x plus cher en energy
    energy -= 8.f; // vs 4.f pour un mouvement normal
    
    // Consommation de faim augmente aussi en sprint
    hunger += hungerRate * 2.f;
    
    // Réduit le MOVE_DELAY temporairement
    float oldDelay = MOVE_DELAY;
    MOVE_DELAY *= 0.6f; // 60% du délai normal
    
    move(dx, dy, carte);
    
    MOVE_DELAY = oldDelay;
}

void Prey::recoverEnergy(float deltaTime) {
    if (energy < maxEnergy) {
        energy += energyRecoveryRate * deltaTime;
        if (energy > maxEnergy) energy = maxEnergy;
    }
}

void Prey::takeDamage(float amount) {
    health -= amount;
    if (health <= 0.f) {
        health = 0.f;
        isAlive = false;
    }
}

Prey* Prey::reproduce(Prey* partner) {
    if (!canReproduce || !partner || !partner->canReproduce) return nullptr;
    
    // Créer un bébé avec traits des parents
    float babyHealth = (maxHealth + partner->maxHealth) / 2.f;
    float babyEnergy = (maxEnergy + partner->maxEnergy) / 2.f;
    float babySpeed = (maxSpeed + partner->maxSpeed) / 2.f;
    float babyHunger = (hungerRate + partner->hungerRate) / 2.f;
    
    Prey* baby = new Prey(gridX, gridY, (sf::Texture*)shape.getTexture(),
                          babyHealth, babyEnergy, babySpeed, babyHunger);
    baby->applyGeneticMutation(0.15f); // 15% de mutation
    
    // Reset reproduction
    this->canReproduce = false;
    this->reproductiveUrge = 0.f;
    this->health *= 0.7f; // Coûte 30% de la santé
    
    partner->canReproduce = false;
    partner->reproductiveUrge = 0.f;
    partner->health *= 0.7f;
    
    return baby;
}

void Prey::applyGeneticMutation(float mutationRate) {
    auto mutate = [mutationRate](float& value) {
        if (rand() % 100 < mutationRate * 100) {
            float change = (rand() % 21 - 10) / 100.f; // -10% à +10%
            value *= (1.f + change);
            value = std::max(10.f, std::min(value, 150.f)); // Clamp
        }
    };
    
    mutate(maxHealth);
    mutate(maxEnergy);
    mutate(maxSpeed);
    mutate(visionRadius);
    mutate(hungerRate);
}

void Prey::determineState() {
    if (health < 30.f) currentState = IDLE; // Trop faible pour faire quoi que ce soit
    else if (hunger > 70.f) currentState = SEEKING_FOOD;
    else if (thirst > 60.f) currentState = SEEKING_WATER;
    else if (reproductiveUrge > 80.f) currentState = IDLE; // En attente d'un partenaire
    else currentState = WANDERING;
}

std::string Prey::getStats() const {
    std::string stats = "=== PREY ===\n";
    stats += "Position: (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")\n";
    stats += "State: " + getStateString() + "\n";
    stats += "Health: " + std::to_string(static_cast<int>(health)) + "/" + std::to_string(static_cast<int>(maxHealth)) + "\n";
    stats += "Energy: " + std::to_string(static_cast<int>(energy)) + "/" + std::to_string(static_cast<int>(maxEnergy)) + "\n";
    stats += "Hunger: " + std::to_string(static_cast<int>(hunger)) + "/100\n";
    stats += "Thirst: " + std::to_string(static_cast<int>(thirst)) + "/100\n";
    stats += "Urge to reproduce: " + std::to_string(static_cast<int>(reproductiveUrge)) + "/100\n";
    stats += "Speed: " + std::to_string(static_cast<int>(maxSpeed * 100.f)) + "%\n";
    return stats;
}

void Prey::update(float deltaTime)  {
     // Appel de l'update physique (interpolation)
        Entity::update(deltaTime);
        if(!isAlive) return;

    // Besoins biologiques mis à jour en continu
    hunger += hungerRate * deltaTime;
    thirst += 0.3f * deltaTime;
    
    // Santé diminue si faim/soif critique
    if (hunger > 90.f || thirst > 90.f) {
        takeDamage(1.f * deltaTime);
    }
    
    // Récupération d'énergie si au repos (pas en mouvement)
    if (moveCooldown <= 0.f) {
        recoverEnergy(deltaTime);
    }
    
    // Reproduction
    if (hunger < 30.f && thirst < 30.f && health > 60.f) {
        reproductiveUrge += 0.5f * deltaTime;
        if (reproductiveUrge > 100.f) reproductiveUrge = 100.f;
    } else {
        reproductiveUrge -= 2.f * deltaTime;
        if (reproductiveUrge < 0.f) reproductiveUrge = 0.f;
    }
    canReproduce = (reproductiveUrge > 70.f);

    aiTickTimer += deltaTime;
}

// Méthode principale de l'IA
void Prey::thinkAndAct(const Map& carte, const std::vector<Entity*>& entities) {
    
    if(moveCooldown > 0.f || !isAlive || aiTickTimer < 0.5f) return;//Il ne réfléchit que toutes les 0.5 secondes
    aiTickTimer = 0;

    // Trop faible pour se déplacer
    if (energy < 5.f) {
        recoverEnergy(0.5f);
        return;
    }

    // 1. FUIR PRÉDATEUR (utilise sprint)
    Entity* predator = scanForTarget(entities, EntityType::PREDATOR, visionRadius+2);
    if (predator && energy > 30.f) {
        int runX = gridX - (predator->gridX - gridX);
        int runY = gridY - (predator->gridY - gridY);
        if (canSprint()) sprintMove(runX > gridX ? 1 : -1, runY > gridY ? 1 : -1, carte);
        else moveTowards(runX, runY, carte);
        return;
    }

    determineState();
    
    // 2. CHERCHER PARTENAIRE POUR REPRODUCTION
    if (canReproduce && energy > 40.f) {
        // Chercher autre proie rassasiée à proximité
        Entity* partner = nullptr;
        int minDist = 9999;
        for (auto* e : entities) {
            if (e == this || e->type != EntityType::PREY || !e->isAlive) continue;
            Prey* p = dynamic_cast<Prey*>(e);
            if (p && p->canReproduce) {
                int d = distManhattan(gridX, gridY, e->gridX, e->gridY);
                if (d <= 10 && d < minDist) {
                    minDist = d;
                    partner = e;
                }
            }
        }
        
        if (partner) {
            if (distManhattan(gridX, gridY, partner->gridX, partner->gridY) <= 1) {
                Prey* baby = reproduce(dynamic_cast<Prey*>(partner));
                if (baby) std::cout << "Prey born at (" << baby->gridX << ", " << baby->gridY << ")\n";
            } else {
                moveTowards(partner->gridX, partner->gridY, carte);
            }
            return;
        }
    }

    // 3. SOIF
    if (thirst > 60.f) {
        sf::Vector2i water = scanForWater(carte, visionRadius+4);
        if (water.x != -1) {
            if (distManhattan(gridX, gridY, water.x, water.y) <= 1) {
                thirst = 0;//Boire
                health+=20; //On recupère un peu de santé
                if(health>=maxHealth) health = maxHealth;
            }
            else moveTowards(water.x, water.y, carte);
            return;
        }
    }

    // 4. FAIM (CHERCHER FOOD ENTITY)
    if (hunger > 50.f) {
        Entity* food = scanForTarget(entities, EntityType::FOOD, visionRadius+4);
        if (food) {
            //Si on est sur la même case que la nourriture
            if (gridX == food->gridX && gridY == food->gridY) {
                food->isAlive = false; //NOURRITURE MANGE
                hunger -= 40.f; //La faim diminue
                health+=50; //On recupère de la santé
                if(health>=maxHealth)health=maxHealth;
                if (hunger < 0.f) hunger = 0.f;
            } else {
                moveTowards(food->gridX, food->gridY, carte);
            }
            return;
        }
    }

    // 5. ERRER
    if (energy > 20.f) {
        move((rand() % 3) - 1, (rand() % 3) - 1, carte);
    }
}

// ========== PREDATOR ==========

Predator::Predator(int x, int y, sf::Texture *texture,
                   float genHealth, float genEnergy, float genSpeed, float genHungerRate, float genVisionRadius)
    : Entity(x, y, EntityType::PREDATOR),
      maxHealth(genHealth), maxEnergy(genEnergy),
      maxSpeed(genSpeed), hungerRate(genHungerRate),
      sprintSpeedMultiplier(1.6f), energyRecoveryRate(12.f) {
    
    health = maxHealth;
    energy = maxEnergy;
    MOVE_DELAY = 0.3f / maxSpeed;
    
    if (texture == nullptr) {
        shape.setFillColor(sf::Color(255, 50, 50)); //Rouge
        shape.setSize({18.f, 30.f});
        shape.setOrigin({9.f, 30.f});
    } else {
        shape.setSize({100, 100});
        shape.setTexture(texture);
        shape.setOrigin({50.f, 50.f});
        shadow.setOrigin({40.f,-18.5f});
        shadow.setRadius(35.f);
    }
}

bool Predator::canSprint() const {
    return energy > 25.f && health > 40.f;
}

void Predator::sprintMove(int dx, int dy, const Map& carte) {
    if (!canSprint()) {
        move(dx, dy, carte);
        return;
    }
    
    energy -= 10.f; // Plus cher que pour Prey
    hunger += hungerRate * 2.5f;
    
    float oldDelay = MOVE_DELAY;
    MOVE_DELAY *= 0.55f;
    
    move(dx, dy, carte);
    
    MOVE_DELAY = oldDelay;
}

void Predator::recoverEnergy(float deltaTime) {
    if (energy < maxEnergy) {
        energy += energyRecoveryRate * deltaTime;
        if (energy > maxEnergy) energy = maxEnergy;
    }
}

void Predator::takeDamage(float amount) {
    health -= amount;
    if (health <= 0.f) {
        health = 0.f;
        isAlive = false;
    }
}

Predator* Predator::reproduce(Predator* partner) {
    if (!canReproduce || !partner || !partner->canReproduce) return nullptr;
    
    float babyHealth = (maxHealth + partner->maxHealth) / 2.f;
    float babyEnergy = (maxEnergy + partner->maxEnergy) / 2.f;
    float babySpeed = (maxSpeed + partner->maxSpeed) / 2.f;
    float babyHunger = (hungerRate + partner->hungerRate) / 2.f;
    
    Predator* baby = new Predator(gridX, gridY, (sf::Texture*)shape.getTexture(),
                                  babyHealth, babyEnergy, babySpeed, babyHunger);
    baby->applyGeneticMutation(0.15f);
    
    this->canReproduce = false;
    this->reproductiveUrge = 0.f;
    this->health *= 0.75f;
    
    partner->canReproduce = false;
    partner->reproductiveUrge = 0.f;
    partner->health *= 0.75f;
    
    return baby;
}

void Predator::applyGeneticMutation(float mutationRate) {
    auto mutate = [mutationRate](float& value) {
        if (rand() % 100 < mutationRate * 100) {
            float change = (rand() % 21 - 10) / 100.f;
            value *= (1.f + change);
            value = std::max(20.f, std::min(value, 180.f));
        }
    };
    
    mutate(maxHealth);
    mutate(maxEnergy);
    mutate(maxSpeed);
    mutate(hungerRate);
}

void Predator::determineState() {
    if (health < 40.f) currentState = IDLE;
    else if (thirst > 80.f) currentState = SEEKING_WATER;
    else if (hunger > 30.f) currentState = HUNTING;
    else if (reproductiveUrge > 80.f) currentState = IDLE;
    else currentState = WANDERING;
}

std::string Predator::getStats() const {
    std::string stats = "=== PREDATOR ===\n";
    stats += "Position: (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")\n";
    stats += "State: " + getStateString() + "\n";
    stats += "Health: " + std::to_string(static_cast<int>(health)) + "/" + std::to_string(static_cast<int>(maxHealth)) + "\n";
    stats += "Energy: " + std::to_string(static_cast<int>(energy)) + "/" + std::to_string(static_cast<int>(maxEnergy)) + "\n";
    stats += "Hunger: " + std::to_string(static_cast<int>(hunger)) + "/100\n";
    stats += "Thirst: " + std::to_string(static_cast<int>(thirst)) + "/100\n";
    stats += "Urge to reproduce: " + std::to_string(static_cast<int>(reproductiveUrge)) + "/100\n";
    stats += "Speed: " + std::to_string(static_cast<int>(maxSpeed * 100.f)) + "%\n";
    return stats;
}

void Predator::update(float dt) {
    Entity::update(dt);
    if (!isAlive) return;

    hunger += hungerRate * dt;
    thirst += 0.4f * dt;
    
    if (hunger > 95.f || thirst > 95.f) {
        takeDamage(1.5f * dt);
    }
    
    if (moveCooldown <= 0.f) {
        recoverEnergy(dt);
    }
    
    if (hunger < 40.f && thirst < 40.f && health > 70.f) {
        reproductiveUrge += 4.f * dt;
        if (reproductiveUrge > 100.f) reproductiveUrge = 100.f;
    } else {
        reproductiveUrge -= 2.f * dt;
        if (reproductiveUrge < 0.f) reproductiveUrge = 0.f;
    }
    canReproduce = (reproductiveUrge > 70.f);

    aiTickTimer += dt;
}

void Predator::thinkAndAct(const Map& carte, const std::vector<Entity*>& entities) {
    if (moveCooldown > 0.f || !isAlive || aiTickTimer < 0.5f) return;
    aiTickTimer = 0;

    if (energy < 10.f) {
        recoverEnergy(0.3f);
        return;
    }

    // 1. REPRODUCTION
    if (canReproduce && energy > 50.f) {
        Entity* partner = nullptr;
        int minDist = 9999;
        for (auto* e : entities) {
            if (e == this || e->type != EntityType::PREDATOR || !e->isAlive) continue;
            Predator* p = dynamic_cast<Predator*>(e);
            if (p && p->canReproduce) {
                int d = distManhattan(gridX, gridY, e->gridX, e->gridY);
                if (d <= 12 && d < minDist) {
                    minDist = d;
                    partner = e;
                }
            }
        }
        
        if (partner) {
            if (distManhattan(gridX, gridY, partner->gridX, partner->gridY) <= 1) {
                Predator* baby = reproduce(dynamic_cast<Predator*>(partner));
                if (baby) std::cout << "Predator born at (" << baby->gridX << ", " << baby->gridY << ")\n";
            } else {
                moveTowards(partner->gridX, partner->gridY, carte);
            }
            return;
        }
    }

    determineState();
    
    // 2. SOIF
    if (thirst > 80.f) {
        sf::Vector2i water = scanForWater(carte, visionRadius+1);
        if (water.x != -1) {
            if (distManhattan(gridX, gridY, water.x, water.y) <= 1) {
                thirst = 0;
                health+=20.f;
                if(health>=maxHealth) health = maxHealth;
            }
            else moveTowards(water.x, water.y, carte);
            return;
        }
    }

    // 3. CHASSE (avec sprint si possible)
    if (hunger > 30.f) {
        Entity* prey = scanForTarget(entities, EntityType::PREY, visionRadius+4);
        if (Prey* p = dynamic_cast<Prey*> (prey)) {
            int d = distManhattan(gridX, gridY, prey->gridX, prey->gridY);
            if (d <= 1) {
                p->takeDamage(50.f); // Proie prend 50 dégâts
                hunger -= 50.f;
                health+=50.f;
                if(health>=maxHealth) health = maxHealth;
                if (hunger < 0.f) hunger = 0.f;
            } else {
                int dx = prey->gridX - gridX;
                int dy = prey->gridY - gridY;
                if (canSprint()) sprintMove(dx > 0 ? 1 : -1, dy > 0 ? 1 : -1, carte);
                else moveTowards(prey->gridX, prey->gridY, carte);
            }
            return;
        }
    }

    // 4. ERRER
    if (energy > 25.f) {
        move((rand() % 3) - 1, (rand() % 3) - 1, carte);
    }
}
