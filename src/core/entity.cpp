#include "entity.hpp"



Entity::Entity(int startX, int startY, EntityType t):gridX(startX),gridY(startY),type(t) {
        // Configuration visuelle par défaut
        shape.setSize(sf::Vector2f(20.f, 40.f));
        shape.setOrigin({10.f, 40.f});
        shape.setFillColor(sf::Color::White); // Blanc par défaut

        shadow.setRadius(10.f);
        shadow.setOrigin({10.f, 5.f});
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
        
        int zLevel = carte.getGroundLevel(static_cast<float>(gridX), static_cast<float>(gridY));
        auto [screenX, screenY] = isoToScreen(gridX, gridY, zLevel, tileWidth, tileHeight, swidth, sheight);
        
        targetScreenPos = sf::Vector2f(screenX, screenY + 25.f);      
        moveCooldown = MOVE_DELAY;
        return true;
    }


void Entity::updateVisualPosition(const Map& carte) {
        int zLevel = carte.getGroundLevel(static_cast<float>(gridX), static_cast<float>(gridY));
        auto [screenX, screenY] = isoToScreen(gridX, gridY, zLevel, tileWidth, tileHeight, swidth, sheight);
        targetScreenPos = {screenX, screenY + 25.f};
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

bool Entity::checkClick(int mouseX, int mouseY, const Map& carte) const {
    if (!isAlive) return false;
    sf::Vector2f entityPos = shape.getPosition() + carte.chunkOrigin;
    float dx = mouseX - entityPos.x;
    float dy = mouseY - entityPos.y;
    float distanceSq = dx * dx + dy * dy;
    float radius = 20.f; // Rayon de détection
    return distanceSq <= radius * radius;
}

std::string Entity::getStateString() const {
    switch(currentState) {
        case IDLE: return "Idle";
        case WANDERING: return "Wandering";
        case SEEKING_FOOD: return "Seeking Food";
        case SEEKING_WATER: return "Seeking Water";
        case FLEEING: return "Fleeing";
        case HUNTING: return "Hunting";
        case DRINKING: return "Drinking";
        case EATING: return "Eating";
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

Prey::Prey(int x, int y): Entity(x ,y, EntityType::PREY) {
       

    // Visuel différent pour la proie (Vert clair par exemple)
    shape.setFillColor(sf::Color(100, 255, 100));
    shape.setSize(sf::Vector2f(15.f, 30.f)); // Plus petit que le joueur
    shape.setOrigin({7.5f, 30.f});
    
    // Vitesse de déplacement basée sur la génétique
    MOVE_DELAY = 0.4f; // Un peu plus lent que le joueur par défaut
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
void Prey::determineState(){
    if (hunger > 60.f) currentState = SEEKING_FOOD;
    else if (thirst > 40.f) currentState = SEEKING_WATER;
    else currentState = WANDERING;
}

std::string Prey::getStats() const {
    std::string stats = "=== PREY ===\n";
    stats += "Position: (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")\n";
    stats += "State: " + getStateString() + "\n";
    stats += "Hunger: " + std::to_string(static_cast<int>(hunger)) + "/100\n";
    stats += "Thirst: " + std::to_string(static_cast<int>(thirst)) + "/100\n";
    stats += "Energy: " + std::to_string(static_cast<int>(energy)) + "/100\n";
    return stats;
}

void Prey::update(float deltaTime)  {
     // Appel de l'update physique (interpolation)
        Entity::update(deltaTime);
        if(!isAlive) return;

        // Mise à jour des besoins biologiques (en continu)
        hunger += 0.2f * deltaTime; // Augmente doucement
        thirst += 0.3f * deltaTime;
        energy -= 0.5f * deltaTime;

        // Mise à jour du cerveau (IA) par intervalle
        aiTickTimer += deltaTime;
    }

// Méthode principale de l'IA
void Prey::thinkAndAct(const Map& carte, const std::vector<Entity*>& entities) {
    
    if(moveCooldown > 0.f || !isAlive || aiTickTimer < 0.5f) return;//Il ne réfléchit que toutes les 0.5 secondes
    aiTickTimer = 0;

    // 1. FUIR PRÉDATEUR
    Entity* predator = scanForTarget(entities, EntityType::PREDATOR, 6);
    if(predator) {
        int runX = gridX - (predator->gridX - gridX);
        int runY = gridY - (predator->gridY - gridY);
        moveTowards(runX, runY, carte);
        return;
    }

    determineState();
    // 2. SOIF
    if(thirst > 60.f) {
        sf::Vector2i water = scanForWater(carte, 10);
        if(water.x != -1) {
            if(distManhattan(gridX, gridY, water.x, water.y) <= 1) thirst = 0; // Boire
            else moveTowards(water.x, water.y, carte);
            return;
        }
    }

    // 3. FAIM (CHERCHER FOOD ENTITY)
    if(hunger > 40.f) {
        Entity* food = scanForTarget(entities, EntityType::FOOD, 10);
        if(food) {
            // Si on est sur la même case que la nourriture
            if(gridX == food->gridX && gridY == food->gridY) {
                food->isAlive = false; // MANGÉ !
                hunger = 0;
            } else {
                moveTowards(food->gridX, food->gridY, carte);
            }
            return;
        }
    }

    // 4. ERRER
    move((rand()%3)-1, (rand()%3)-1, carte);
}


// --- PREDATOR ---


Predator::Predator(int x, int y) : Entity(x, y, EntityType::PREDATOR) {
    shape.setFillColor(sf::Color(255, 50, 50)); // Rouge
    shape.setSize({18.f, 30.f});
    shape.setOrigin({9.f, 30.f});
    MOVE_DELAY = 0.3f;
    speed = 12.f;
}

void Predator::determineState() {
    if (thirst > 70.f) currentState = SEEKING_WATER;
    else if (hunger > 20.f) currentState = HUNTING;
    else currentState = WANDERING;
}

std::string Predator::getStats() const {
    std::string stats = "=== PREDATOR ===\n";
    stats += "Position: (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")\n";
    stats += "State: " + getStateString() + "\n";
    stats += "Hunger: " + std::to_string(static_cast<int>(hunger)) + "/100\n";
    stats += "Thirst: " + std::to_string(static_cast<int>(thirst)) + "/100\n";
    return stats;
}

void Predator::update(float dt) {
    Entity::update(dt);
    if(!isAlive) return;
    hunger += 1.5f * dt;
    thirst += 2.0f * dt;
    aiTickTimer += dt;
}

void Predator::thinkAndAct( const Map& carte, const std::vector<Entity*>& entities) {
    if(moveCooldown > 0.f || !isAlive || aiTickTimer < 0.3f) return;
    aiTickTimer = 0;

    determineState();
    // 1. SOIF
    if(thirst > 70.f) {
        sf::Vector2i water = scanForWater(carte, 15);
        if(water.x != -1) {
            if(distManhattan(gridX, gridY, water.x, water.y) <= 1) thirst = 0;
            else moveTowards(water.x, water.y, carte);
            return;
        }
    }

    // 2. CHASSE (Prey)
    if(hunger > 20.f) {
        Entity* prey = scanForTarget(entities, EntityType::PREY, 12);
        if(prey) {
            if(distManhattan(gridX, gridY, prey->gridX, prey->gridY) <= 1) {
                prey->isAlive = false; // TUÉ !
                hunger = 0;
            } else {
                moveTowards(prey->gridX, prey->gridY, carte);
            }
            return;
        }
    }

    // 3. ERRER
    move((rand()%3)-1, (rand()%3)-1, carte);
}
