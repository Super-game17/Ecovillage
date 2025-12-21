#include "entity.hpp"

Entity::Entity(int startX, int startY, EntityType t, sf::Texture *texture):gridX(startX),gridY(startY),type(t) {
        if(texture ==nullptr){
        // Configuration visuelle par défaut
        shape.setSize(sf::Vector2f(20.f, 40.f));
        shape.setOrigin({10.f, 40.f});
        shape.setFillColor(sf::Color::White); // Blanc par défaut
        originalColor = shape.getFillColor();
        shadow.setOrigin({10.f, 5.f});
        shadow.setRadius(10.f);
        }
        else {
            shape.setSize({100,100});
            shape.setTexture(texture);
            shape.setOrigin({50.f, 50.f});
            originalColor = shape.getFillColor();
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

// Méthode pour déclencher l'animation de dégâts
void Entity::triggerDamageFlash() {
    isDamaged = true;
    damageFlashTimer = 0.f;
}

// Animation de clignotement rouge (dégâts)
void Entity::updateDamageAnimation(float deltaTime) {
    if (!isDamaged) return;
    
    damageFlashTimer += deltaTime;
    
    // Fréquence de clignotement : 4 fois par seconde
    float flashFrequency = 8.f; // Plus élevé = clignotement plus rapide
    float phase = sin(damageFlashTimer * flashFrequency * 3.14159f);
    
    if (phase > 0.f) {
        // Rouge vif pendant la phase "allumée"
        shape.setFillColor(sf::Color(255, 50, 50, 255));
    } else {
        // Couleur originale pendant la phase "éteinte"
        shape.setFillColor(originalColor);
    }
    
    // Fin de l'animation
    if (damageFlashTimer >= damageFlashDuration) {
        isDamaged = false;
        shape.setFillColor(originalColor); // Revenir à la normale
    }
}

// Animation de mort
void Entity::updateDeathAnimation(float deltaTime) {
    if (!isDying) return;
    
    deathAnimTimer += deltaTime;
    
    // Calculer le progrès de l'animation (0.0 à 1.0)
    float progress = deathAnimTimer / deathAnimDuration;
    
    if (progress >= 1.f) {
        // Animation terminée, l'entité peut vraiment mourir
        isAlive = false;
        isDying = false;
        deathAnimTimer = 0.f;
        return;
    }
    
    // EFFET 1 : Fade out (disparition progressive)
    float alpha = 255.f * (1.f - progress);
    sf::Color currentColor = shape.getFillColor();
    currentColor.a = alpha;
    shape.setFillColor(currentColor);
    
    // EFFET 2 : L'entité "tombe" (descend progressivement)
    float fallOffset = progress * 30.f; // Tombe de 30 pixels
    shape.setPosition({currentScreenPos.x, currentScreenPos.y + fallOffset});
    
    // EFFET 3 : Rotation (tombe sur le côté)
    sf::Angle angle = sf::degrees(progress * 90.f); // Tourne de 90 degrés
    shape.setRotation(angle); // Rotation de 90 degrés
    
    // EFFET 4 : L'ombre rétrécit
    float shadowScale = 1.f - progress;
    shadow.setScale({shadowScale, shadowScale * 0.5f});
}


void Entity::update(float deltaTime) {
    if (moveCooldown > 0.f) moveCooldown -= deltaTime;
    
    // Animations de dégâts et de mort
    updateDamageAnimation(deltaTime);
    updateDeathAnimation(deltaTime);
    
    // Ne pas bouger si en train de mourir
    if (isDying) {
        if (isSelected) updateSelectionVisual();
        return;
    }

    // Interpolation de la position pour un mouvement fluide
    sf::Vector2f diff = targetScreenPos - currentScreenPos;
    // Calcul de la distance au carré
    float distSq = diff.x * diff.x + diff.y * diff.y;
    
    // Si la distance est significative, avancer vers la cible
    if (distSq > 0.5f) {
        float t = deltaTime * speed; 
        currentScreenPos += diff * t;
    } else {
        currentScreenPos = targetScreenPos;
    }
    
    shape.setPosition(currentScreenPos);
    shadow.setPosition(currentScreenPos);
    
    if (isSelected) {
        updateSelectionVisual();
    }
}


void Entity::draw(sf::RenderTarget& target, const Map& Carte) const{
        if (!isAlive && !isDying) return ;
        sf::RenderStates states;
        states.transform.translate(Carte.chunkOrigin);
        target.draw(shadow, states);
        target.draw(shape, states);

        if(isSelected && !isDying){
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
      maxSpeed(genSpeed), hungerRate(genHungerRate), visionRadius(genVisionRadius),
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

void Prey::HandleInput(const sf::RenderWindow& window, const Map& Carte, const std::vector<Entity*>& entities) {
    Entity::HandleInput(window, Carte);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)) {
        Entity* food = scanForTarget(entities, EntityType::FOOD, 1);
        if (food) {
            food->isAlive = false; // Manger la nourriture
            hunger -= 30.f; // Réduit la faim
            health += 40.f; // Restaure un peu de santé
            if (health > maxHealth) health = maxHealth;
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
            health += 20.f; // Restaure un peu de santé
            if (health > maxHealth) health = maxHealth;
            if (thirst < 0.f) thirst = 0.f;
            std::cout << "Prey at (" << gridX << ", " << gridY << ") drank water.\n";}
        }
        else {
            std::cout << "No water to drink nearby.\n";
        }
    }
}

// Sprint plus accessible pour les proies aussi
bool Prey::canSprint() const {
    return energy > 12.f && health > 20.f; // Seuils plus bas
}

void Prey::sprintMove(int dx, int dy, const Map& carte) {
    if (!canSprint()) {
        move(dx, dy, carte);
        return;
    }
    
    energy -= 5.f; // Moins coûteux (était 8)
    hunger += hungerRate * 1.5f; // Moins punitif (était 2)
    
    float oldDelay = MOVE_DELAY;
    MOVE_DELAY *= 0.6f;
    
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

    triggerDamageFlash();
    if (health <= 0.f) {
        health = 0.f;
        isDying = true;
        std::cout << "Prey at (" << gridX << ", " << gridY << ") is dying.\n";
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
        reproductiveUrge += 3.f * deltaTime;
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
    
    if(moveCooldown > 0.f || !isAlive || aiTickTimer < 0.5f) return;
    aiTickTimer = 0;

    if (energy < 3.f) { // Seuil plus bas
        recoverEnergy(0.5f);
        return;
    }

    // 1. FUIR PRÉDATEUR (vision augmentée pour détecter plus tôt)
    Entity* predator = scanForTarget(entities, EntityType::PREDATOR, visionRadius + 4); // Au lieu de +2
    if (predator && energy > 20.f) {
        int runX = gridX - (predator->gridX - gridX);
        int runY = gridY - (predator->gridY - gridY);
        
        // Utiliser sprint si le prédateur est proche
        int d = distManhattan(gridX, gridY, predator->gridX, predator->gridY);
        if (d <= 3 && canSprint()) {
            sprintMove(runX > gridX ? 1 : -1, runY > gridY ? 1 : -1, carte);
        } else {
            moveTowards(runX, runY, carte);
        }
        return;
    }

    determineState();
    
    // 2. REPRODUCTION (seulement si sécurisé)
    if (canReproduce && energy > 30.f) {
        // Vérifier qu'il n'y a pas de prédateur proche
        Entity* nearbyPredator = scanForTarget(entities, EntityType::PREDATOR, 8);
        if (!nearbyPredator) {
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
    }

    // 3. SOIF
    if (thirst > 55.f) { // Seuil légèrement abaissé
        sf::Vector2i water = scanForWater(carte, visionRadius + 5);
        if (water.x != -1) {
            if (distManhattan(gridX, gridY, water.x, water.y) <= 1) {
                thirst = 0;
                health += 20;
                if(health >= maxHealth) health = maxHealth;
            }
            else moveTowards(water.x, water.y, carte);
            return;
        }
    }

    // 4. FAIM
    if (hunger > 45.f) { // Seuil légèrement abaissé
        Entity* food = scanForTarget(entities, EntityType::FOOD, visionRadius + 5);
        if (food) {
            if (gridX == food->gridX && gridY == food->gridY) {
                food->isAlive = false;
                hunger -= 45.f;
                health += 40;
                if(health >= maxHealth) health = maxHealth;
                if (hunger < 0.f) hunger = 0.f;
            } else {
                moveTowards(food->gridX, food->gridY, carte);
            }
            return;
        }
    }

    // 5. ERRER
    if (energy > 12.f) {
        move((rand() % 3) - 1, (rand() % 3) - 1, carte);
    }
}

// ========== PREDATOR ==========

Predator::Predator(int x, int y, sf::Texture *texture,
                   float genHealth, float genEnergy, float genSpeed, float genHungerRate, float genVisionRadius)
    : Entity(x, y, EntityType::PREDATOR),
      maxHealth(genHealth), maxEnergy(genEnergy),
      maxSpeed(genSpeed), hungerRate(genHungerRate), visionRadius(genVisionRadius),
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

// Ajustement du sprint pour être plus accessible
bool Predator::canSprint() const {
    return energy > 15.f && health > 25.f; // Seuils plus bas
}

void Predator::sprintMove(int dx, int dy, const Map& carte) {
    if (!canSprint()) {
        move(dx, dy, carte);
        return;
    }
    
    energy -= 6.f; // Moins coûteux (était 10)
    hunger += hungerRate * 1.5f; // Moins punitif (était 2.5)
    
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
    triggerDamageFlash();
    if (health <= 0.f) {
        health = 0.f;
        isDying = true;
        std::cout << "Predator at (" << gridX << ", " << gridY << ") is dying.\n";
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
    else if (hunger > 50.f) currentState = HUNTING;
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

    // Réduction de la consommation de faim (était trop élevée)
    hunger += hungerRate * dt * 0.6f; // 40% moins rapide
    thirst += 0.35f * dt; // Légèrement réduit aussi
    
    if (hunger > 95.f || thirst > 95.f) {
        takeDamage(1.2f * dt); // Moins punitif
    }
    
    if (moveCooldown <= 0.f) {
        recoverEnergy(dt);
    }
    
    // Reproduction uniquement si en bonne santé ET rassasié
    if (hunger < 30.f && thirst < 30.f && health > 70.f) {
        reproductiveUrge += 3.f * dt;
        if (reproductiveUrge > 100.f) reproductiveUrge = 100.f;
    } else {
        reproductiveUrge -= 2.f * dt;
        if (reproductiveUrge < 0.f) reproductiveUrge = 0.f;
    }
    canReproduce = (reproductiveUrge > 70.f);

    aiTickTimer += dt;
}

void Predator::thinkAndAct(const Map& carte, const std::vector<Entity*>& entities) {
    if (moveCooldown > 0.f || !isAlive || aiTickTimer < 0.4f) return; // Tick plus rapide
    aiTickTimer = 0;

    // Seuil d'énergie beaucoup plus bas pour agir
    if (energy < 3.f) {
        recoverEnergy(0.3f);
        return;
    }

    determineState();
    
    // 1. SOIF CRITIQUE (priorité absolue si > 85)
    if (thirst > 85.f) {
        sf::Vector2i water = scanForWater(carte, visionRadius + 6);
        if (water.x != -1) {
            if (distManhattan(gridX, gridY, water.x, water.y) <= 1) {
                thirst = 0;
                health += 15.f;
                if (health >= maxHealth) health = maxHealth;
            }
            else moveTowards(water.x, water.y, carte);
            return;
        }
    }

    // 2. CHASSE (seuil abaissé pour commencer plus tôt)
    if (hunger > 50.f) { // Au lieu de 30
        Entity* prey = scanForTarget(entities, EntityType::PREY, visionRadius + 8); // Vision augmentée
        if (Prey* p = dynamic_cast<Prey*>(prey)) {
            int d = distManhattan(gridX, gridY, prey->gridX, prey->gridY);
            if (d <= 1) {
                // ATTAQUE !
                p->takeDamage(80.f);
                hunger -= 60.f; // Plus nourrissant
                health += 40.f;
                if (health >= maxHealth) health = maxHealth;
                if (hunger < 0.f) hunger = 0.f;
                // Message uniquement si la proie meurt vraiment
                if (p->health <= 0.f) {
                    std::cout << "Predator killed prey at (" << prey->gridX << ", " << prey->gridY << ")\n";
                }
            } else {
                // Utiliser sprint si proche et assez d'énergie
                int dx = prey->gridX - gridX;
                int dy = prey->gridY - gridY;
                if (d <= 4 && canSprint()) {
                    sprintMove(dx > 0 ? 1 : -1, dy > 0 ? 1 : -1, carte);
                } else {
                    moveTowards(prey->gridX, prey->gridY, carte);
                }
            }
            return;
        }
    }
    
    // 3. SOIF NORMALE
    if (thirst > 50.f) {
        sf::Vector2i water = scanForWater(carte, visionRadius + 6);
        if (water.x != -1) {
            if (distManhattan(gridX, gridY, water.x, water.y) <= 1) {
                thirst = 0;
                health += 15.f;
                if (health >= maxHealth) health = maxHealth;
            }
            else moveTowards(water.x, water.y, carte);
            return;
        }
    }

    // 4. REPRODUCTION (seulement si bien nourri)
    if (canReproduce && energy > 40.f && hunger < 30.f) {
        Entity* partner = nullptr;
        int minDist = 9999;
        for (auto* e : entities) {
            if (e == this || e->type != EntityType::PREDATOR || !e->isAlive) continue;
            Predator* p = dynamic_cast<Predator*>(e);
            if (p && p->canReproduce && p->hunger < 30.f) { // Partenaire doit aussi être rassasié
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

    // 5. ERRER (consomme peu d'énergie)
    if (energy > 15.f) {
        move((rand() % 3) - 1, (rand() % 3) - 1, carte);
    }
}