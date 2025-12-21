#include "game.hpp"
#include <iostream>
#include <cmath>

Game::Game() 
    : window(sf::VideoMode({swidth, sheight}), "Isometric Chunk System"),
      gui(window),
      camera(static_cast<float>(swidth), static_cast<float>(sheight), sf::Vector2f(swidth/2.f, sheight/2.f))
{
    // 1. Charger la Configuration (et créer le fichier par défaut si besoin)
    config.load();
    
    // 2. Charger les ressources
    if(!textureBlocks.loadFromFile("assets/sprites/block.png")){
        std::cerr << "Erreur: assets/sprites/block.png non trouvé." << std::endl;
        window.close();
    }
    textureBlocks.setSmooth(true);
    if(!deerText.loadFromFile("assets/sprites/DeerAtlas.png")){
        std::cerr << "Erreur: assets/sprites/Deer1.png non trouvé." << std::endl;
        window.close();
    }
    deerText.setSmooth(true);
    if(!bearText.loadFromFile("assets/sprites/Bear1.png")){
        std::cerr << "Erreur: assets/sprites/Bear2.png non trouvé." << std::endl;
        window.close();
    }
    bearText.setSmooth(true);
    
    // 3. Démarrer sur le Menu
    setupMenu();
}

void Game::run() {
    while (window.isOpen()) {
        float deltaTime = gameClock.restart().asSeconds();
        
        handleEvents();
        
        if (currentState != GameState::MENU) {
            update(deltaTime);
        }
        
        render();
    }
    
    // Nettoyage des entités lors de la fermeture
    for (Entity* e : entities) {
        delete e;
    }
    entities.clear();
}

void Game::handleEvents() {
    while (const std::optional event = window.pollEvent()){
        gui.handleEvent(*event);
        
        if (event->is<sf::Event::Closed>()){
            window.close();
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
            if(keyPressed->scancode == sf::Keyboard::Scancode::Escape){
                if (currentState != GameState::MENU) {
                    setupMenu();
                    currentState = GameState::MENU;
                }
            }
            
            if(keyPressed->scancode == sf::Keyboard::Scancode::Up){
                map.renderDistance++;
                std::cout << "Render Distance: " << map.renderDistance << std::endl;
            }
            if(keyPressed->scancode == sf::Keyboard::Scancode::Down && map.renderDistance > 1){
                map.renderDistance--;
                std::cout << "Render Distance: " << map.renderDistance << std::endl;
            }
            
            // F2 : Toggle Perlin Config Panel (UNIQUEMENT en mode PERLIN_PLAY)
            if(keyPressed->scancode == sf::Keyboard::Scancode::F2 && currentState == GameState::PERLIN_PLAY){
                togglePerlinPanel();
            }

            // F2 : Toggle Perlin Config Panel (UNIQUEMENT en mode PERLIN_PLAY)
            if(keyPressed->scancode == sf::Keyboard::Scancode::F2 && currentState != GameState::PERLIN_PLAY){
                toggleSpawnerPanel();
            }
            
            // --- SÉLECTION D'ENTITÉS (sauf en mode MENU et PERLIN_PLAY) ---
            if (currentState != GameState::MENU && currentState != GameState::PERLIN_PLAY) {
                // TAB : Sélectionner l'entité suivante
                if (keyPressed->scancode == sf::Keyboard::Scancode::Tab) {
                    selectNextEntity();
                }
                // SHIFT+TAB : Sélectionner l'entité précédente
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Tab && 
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
                    selectPreviousEntity();
                }
                // F1 : Afficher stats dans console
                if (keyPressed->scancode == sf::Keyboard::Scancode::F1) {
                // Afficher les stats de l'entité sélectionnée
                if (selectedEntity != nullptr) {
                    std::cout << "Stats de l'entité sélectionnée : " << selectedEntity->getStats() << std::endl;
                } else {
                    std::cout << "Aucune entité sélectionnée." << std::endl;
                }
                }
            
                // SPACE : Désélectionner
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                    deselectEntity();
                }
            }
        }
        
        if (currentState != GameState::MENU) {
            camera.handleEvent(*event);
        }
    }
}

void Game::update(float deltaTime) {
    camera.handleInput(window);
    camera.update(window);
    
    if (currentState == GameState::SIMULATION_MODE || 
        currentState == GameState::SURVIVAL_MODE || 
        currentState == GameState::FREE_MODE) 
    {
        // 1. Mise à jour de toutes les entités
        for (size_t i = 0; i < entities.size(); ) {
            Entity* e = entities[i];
            
            if (!e->isAlive) {
                // Si l'entité morte était sélectionnée, désélectionner
                if (e == selectedEntity) {
                    deselectEntity();
                }
                delete e;
                entities.erase(entities.begin() + i);
                continue; 
            }
            e->update(deltaTime);
            if (Prey* p = dynamic_cast<Prey*>(e)) {
                if(currentState == GameState::SURVIVAL_MODE && p->isSelected){
                    // Le joueur contrôle cette proie
                } else {
                    p->thinkAndAct(map, entities);
                }
            }
            else if (Predator* pr = dynamic_cast<Predator*>(e)) {
                pr->thinkAndAct(map, entities);
            }
            ++i;
        }
        
        // 2. Mettre à jour le pointeur de sélection après modifications
        updateSelectedEntity();
        
        // 3. Contrôle de l'entité sélectionnée
        if (selectedEntity != nullptr) {
            if (Prey* p = dynamic_cast<Prey*>(selectedEntity)) {
                p->HandleInput(window, map, entities);
            } else {
                selectedEntity->HandleInput(window, map);
            }
            // Centrer la caméra sur l'entité sélectionnée (optionnel)
            // camera.setCenter(selectedEntity->currentScreenPos);
        }
        
        // 4. Utiliser le spawner UNIQUEMENT en mode FREE et SURVIVAL
        if (currentState == GameState::FREE_MODE || currentState == GameState::SURVIVAL_MODE) {
            spawner.update(deltaTime, entities, map, selectedEntity, &deerText, &bearText);
        }
        else if (currentState == GameState::SIMULATION_MODE) {
            spawner.updateFoodOnly(deltaTime, entities, map);
        }
        
        // Mettre à jour l'affichage des stats UNIQUEMENT dans les modes avec entités
        updateStatsDisplay();
    }

    map.update(camera.getCenter(), textureBlocks);
}

void Game::render() {
    window.clear(sf::Color(153, 204, 255));
    
    if (currentState != GameState::MENU) {
        if (currentState == GameState::PERLIN_PLAY) {
            map.renderFast(window, textureBlocks);
        }
        else {
            // Utiliser l'entité sélectionnée pour le rendu, sinon une entité par défaut
            Entity defaultEntity(0, 0, EntityType::PLAYER);
            Entity* renderEntity = selectedEntity ? selectedEntity : &defaultEntity;
            
            map.render(window, textureBlocks, entities, renderEntity);
            
            // Les entités se dessinent elles-mêmes avec leur cercle de sélection
            // Pas besoin de dessiner manuellement le highlight ici
        }
    }

    gui.draw();
    window.display();
}

// --- FONCTIONS DE SÉLECTION ---

void Game::selectNextEntity() {
    if (entities.empty()) {
        selectedEntity = nullptr;
        return;
    }
    
    // Désélectionner l'entité actuelle
    if (selectedEntity != nullptr) {
        selectedEntity->setSelected(false);
    }
    
    // Trouver la prochaine entité sélectionnable (pas de FOOD)
    size_t startIndex = selectedIndex;
    do {
        selectedIndex = (selectedIndex + 1) % entities.size();
        
        if (entities[selectedIndex]->type != EntityType::FOOD) {
            selectedEntity = entities[selectedIndex];
            selectedEntity->setSelected(true);
            
            std::cout << "Entité sélectionnée : " << selectedIndex 
                      << " (Type: " << static_cast<int>(selectedEntity->type) << ")" << std::endl;
            return;
        }
        
        if (selectedIndex == startIndex) {
            selectedEntity = nullptr;
            std::cout << "Aucune entité sélectionnable disponible" << std::endl;
            return;
        }
    } while (true);
}

void Game::selectPreviousEntity() {
    if (entities.empty()) {
        selectedEntity = nullptr;
        return;
    }
    
    // Désélectionner l'entité actuelle
    if (selectedEntity != nullptr) {
        selectedEntity->setSelected(false);
    }
    
    size_t startIndex = selectedIndex;
    do {
        if (selectedIndex == 0) {
            selectedIndex = entities.size() - 1;
        } else {
            selectedIndex--;
        }
        
        if (entities[selectedIndex]->type != EntityType::FOOD) {
            selectedEntity = entities[selectedIndex];
            selectedEntity->setSelected(true);
            
            std::cout << "Entité sélectionnée : " << selectedIndex 
                      << " (Type: " << static_cast<int>(selectedEntity->type) << ")" << std::endl;
            return;
        }
        
        if (selectedIndex == startIndex) {
            selectedEntity = nullptr;
            std::cout << "Aucune entité sélectionnable disponible" << std::endl;
            return;
        }
    } while (true);
}

void Game::deselectEntity() {
    if (selectedEntity != nullptr) {
        selectedEntity->setSelected(false);
    }
    selectedEntity = nullptr;
    std::cout << "Aucune entité sélectionnée - Mode observation" << std::endl;
}

void Game::updateSelectedEntity() {
    if (selectedEntity == nullptr) return;
    
    // Vérifier si l'entité existe toujours dans le vecteur
    auto it = std::find(entities.begin(), entities.end(), selectedEntity);
    if (it != entities.end()) {
        selectedIndex = std::distance(entities.begin(), it);
    } else {
        // L'entité a été supprimée
        deselectEntity();
    }
}

// =======================================================================
// --- LOGIQUE DE DÉMARRAGE DES MODES ---
// =======================================================================

void Game::startGame(GameState newState, MapMode mapMode) {
    currentState = newState;
    gui.removeAllWidgets(); // Nettoyer l'interface = masquer le menu
    
    // 1. Nettoyer les anciennes entités
    for (Entity* e : entities) {
        delete e;
    }
    entities.clear();
    
    // 2. Réinitialiser la carte et le bruit
    map.reset(mapMode, config.world);
    map.update(camera.getCenter(), textureBlocks);
    // 3. Positionner la caméra au centre de la zone générée
    camera.setCenter({swidth/2.f, sheight/2.f}); 
    
    // Créer l'UI appropriée selon le mode
    if (newState == GameState::PERLIN_PLAY) {
        // Mode Perlin : Seulement le panneau de config Perlin
        setupPerlinConfigPanel();
    } else {
        // Modes avec entités : Seulement le label de stats
        setupStatsLabel();
    }
    
    if (newState == GameState::PERLIN_PLAY) {
        setupPerlinPlay();
    } else if (newState == GameState::FREE_MODE) {
        setupFreeMode();
    } else if (newState == GameState::SIMULATION_MODE) {
        setupSimulationMode();
    } else if (newState == GameState::SURVIVAL_MODE) {
        setupSurvivalMode();
    }
}

void Game::setupPerlinPlay() {
    // Mode Pur Visuel : Pas d'entités, caméra libre, rendu rapide.
    std::cout << "Mode Perlin-Play Démarré." << std::endl;
    // On pourrait ajouter des sliders TGUI ici pour ajuster la config.world en temps réel.
}

void Game::setupFreeMode() {
    std::cout << "Mode Free Démarré." << std::endl;
    
    selectedEntity = nullptr;
    
    // Utiliser le spawner pour une création initiale robuste
    spawnInitialEntities(10, 3, 20);
    
    std::cout << "Appuyez sur TAB pour sélectionner une entité, ESPACE pour désélectionner" << std::endl;
}

void Game::setupSimulationMode() {
    std::cout << "Mode Simulation Démarré." << std::endl;
    
    selectedEntity = nullptr;
    spawnInitialEntities(40, 15, 30);
    
    std::cout << "Appuyez sur TAB pour sélectionner une entité, ESPACE pour désélectionner" << std::endl;
}

void Game::setupSurvivalMode() {
    std::cout << "Mode Survival Démarré." << std::endl;
    
    // Créer le joueur au centre
    Prey* player = new Prey(0, 0, &deerText);
    player->shape.setFillColor(sf::Color(100, 180, 255));
    player->updateVisualPosition(map);
    entities.push_back(player);
    
    selectedEntity = player;
    selectedIndex = 0;
    selectedEntity->setSelected(true);
    
    camera.setCenter(player->currentScreenPos);
    
    // Spawner autour du joueur
    spawnInitialEntities(2, 1, 15);
    
    std::cout << "Joueur sélectionné automatiquement en mode Survival" << std::endl;
}

// Nouvelle fonction centralisée de spawn initial
void Game::spawnInitialEntities(int preyCount, int predCount, int spawnRadius) {
    if (map.chunksToRender.empty()) {
        std::cerr << "Erreur: Aucun chunk à rendre pour spawn initial" << std::endl;
        return;
    }
    
    int preySpawned = 0, predSpawned = 0;
    int maxAttempts = 50; // Maximum d'essais par entité
    
    // SPAWN PROIES
    for (int i = 0; i < preyCount && preySpawned < preyCount; ) {
        const Chunk* randomChunk = map.chunksToRender[rand() % map.chunksToRender.size()];
        int baseX = randomChunk->chunkX * Chunk::SIZE;
        int baseY = randomChunk->chunkY * Chunk::SIZE;
        
        int x = baseX + (rand() % Chunk::SIZE);
        int y = baseY + (rand() % Chunk::SIZE);
        
        if (!map.isObstacle(x, y) && !map.isWater(x, y)) {
            Prey* prey = new Prey(x, y, &deerText);
            prey->updateVisualPosition(map);
            entities.push_back(prey);
            preySpawned++;
            std::cout << "init Prey spawned at (" << x << ", " << y << ")\n";
        }
        
        i++;
        if (i > maxAttempts) break; // Éviter boucle infinie
    }
    
    // SPAWN PRÉDATEURS
    for (int i = 0; i < predCount && predSpawned < predCount; ) {
        const Chunk* randomChunk = map.chunksToRender[rand() % map.chunksToRender.size()];
        int baseX = randomChunk->chunkX * Chunk::SIZE;
        int baseY = randomChunk->chunkY * Chunk::SIZE;
        
        int x = baseX + (rand() % Chunk::SIZE);
        int y = baseY + (rand() % Chunk::SIZE);
        
        if (!map.isObstacle(x, y) && !map.isWater(x, y)) {
            Predator* pred = new Predator(x, y, &bearText);
            pred->updateVisualPosition(map);
            entities.push_back(pred);
            predSpawned++;
            std::cout << "init Predator spawned at (" << x << ", " << y << ")\n";
        }
        
        i++;
        if (i > maxAttempts) break;
    }
    
    std::cout << "Joueur selectionné automatiquement en mode Survival" << std::endl;
    std::cout << "Initial spawn: " << preySpawned << " proies, " << predSpawned 
              << " prédateurs" << std::endl;
}

// =======================================================================
// --- LOGIQUE DE MENU TGUI ---
// =======================================================================

void Game::setupMenu() {
    gui.removeAllWidgets();
    
    auto panel = tgui::Panel::create();
    panel->setSize({"50%", "80%"});
    panel->setPosition({"25%", "10%"});
    gui.add(panel, "MainPanel");

    // Titre
    auto label = tgui::Label::create("EcoVillage");
    label->setTextSize(36);
    label->setPosition({"center", 20});
    panel->add(label);

    // Boutons de mode
    float startY = 100.f;
    float buttonHeight = 50.f;
    float spacing = 20.f;

    // 1. Perlin Play (Visualisation rapide)
    auto btnPerlin = tgui::Button::create("1. Perlin Play (Visualisation)");
    btnPerlin->setSize({"80%", buttonHeight});
    btnPerlin->setPosition({"center", startY});
    btnPerlin->onClick([this] { startGame(GameState::PERLIN_PLAY, MapMode::INFINITE); });
    panel->add(btnPerlin);

    // 2. Free Mode (Monde infini, Caméra libre/Joueur)
    auto btnFree = tgui::Button::create("2. Free Mode (Monde Infini)");
    btnFree->setSize({"80%", buttonHeight});
    btnFree->setPosition({"center", startY + buttonHeight + spacing});
    btnFree->onClick([this] { startGame(GameState::FREE_MODE, MapMode::INFINITE); });
    panel->add(btnFree);

    // 3. Simulation Mode (Monde fini, Observer)
    auto btnSim = tgui::Button::create("3. Simulation Mode (Monde Fini)");
    btnSim->setSize({"80%", buttonHeight});
    btnSim->setPosition({"center", startY + (buttonHeight + spacing) * 2});
    btnSim->onClick([this] { startGame(GameState::SIMULATION_MODE, MapMode::FINITE); });
    panel->add(btnSim);

    // 4. Survival Mode (Monde fini, Joueur principal)
    auto btnSurv = tgui::Button::create("4. Survival Mode");
    btnSurv->setSize({"80%", buttonHeight});
    btnSurv->setPosition({"center", startY + (buttonHeight + spacing) * 3});
    btnSurv->onClick([this] { startGame(GameState::SURVIVAL_MODE, MapMode::FINITE); });
    panel->add(btnSurv);

    // Bouton Quitter
    auto btnQuit = tgui::Button::create("Quitter");
    btnQuit->setSize({"80%", buttonHeight});
    btnQuit->setPosition({"center", startY + (buttonHeight + spacing) * 4 + 50.f});
    btnQuit->onClick([this] { window.close(); });
    panel->add(btnQuit);
}

// =======================================================================
// --- FONCTIONS UI EN JEU ---
// =======================================================================

void Game::setupStatsLabel() {
    // Label pour les stats de l'entité sélectionnée (coin supérieur gauche)
    statsLabel = tgui::Label::create();
    statsLabel->setPosition(10, 10);
    statsLabel->setTextSize(14);
    statsLabel->getRenderer()->setTextColor(tgui::Color::White);
    statsLabel->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 150));
    statsLabel->getRenderer()->setPadding({5, 5, 5, 5});
    gui.add(statsLabel, "StatsLabel");
    
    // Créer le panneau de configuration Perlin (masqué par défaut)
    setupPerlinConfigPanel();
    setupSpawnerPanel();
}

void Game::updateStatsDisplay() {
    if (!statsLabel) return;

    // Vérifier si l'entité sélectionnée est toujours valide
    auto it = std::find(entities.begin(), entities.end(), selectedEntity);
    if (it == entities.end()) {
        selectedEntity = nullptr;
    }

    // Comptages globaux
    int preyCount = 0, predatorCount = 0, foodCount = 0;
    for (const Entity* e : entities) {
        switch (e->type) {
            case EntityType::PREY:      ++preyCount; break;
            case EntityType::PREDATOR:  ++predatorCount; break;
            case EntityType::FOOD:      ++foodCount; break;
            default: break;
        }
    }

    std::string text = "TAB: Sélection | SPACE: Déselection | F1: Stats console\n";
    text += "Entités: " + std::to_string(entities.size()) + "\n";
    text += "Proies: " + std::to_string(preyCount) +
            " | Prédateurs: " + std::to_string(predatorCount) +
            " | Nourriture: " + std::to_string(foodCount) + "\n";

    if (selectedEntity != nullptr && selectedEntity->type != EntityType::FOOD) {
        text += "\n--- ENTITÉ SÉLECTIONNÉE ---\n";
        text += selectedEntity->getStats();
    } else {
        text += "\nAucune entité sélectionnée";
    }

    statsLabel->setText(text);
}

void Game::setupPerlinConfigPanel() {
    // Panneau de configuration Perlin
    perlinPanel = tgui::Panel::create();
    perlinPanel->setSize({"40%", "100%"});
    perlinPanel->setPosition({"35%", "5%"});
    perlinPanel->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40, 230));
    perlinPanel->setVisible(false);
    gui.add(perlinPanel, "PerlinPanel");
    
    // Espacement des éléments
    float yPos = 10.f;
    float spacing = 50.f;
    
    // Titre
    auto title = tgui::Label::create("Configuration Perlin");
    title->setTextSize(20);
    title->setPosition({"center", yPos});
    title->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(title);
    yPos += 40;
    
    // Seed
    auto seedLabel = tgui::Label::create("Seed: " + std::to_string(config.world.seed));
    seedLabel->setPosition({10, yPos});
    seedLabel->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(seedLabel, "SeedLabel");
    yPos += 20;
    
    auto seedSlider = tgui::Slider::create(0, 10000);
    seedSlider->setSize({"80%", 20});
    seedSlider->setPosition({"center", yPos});
    seedSlider->getRenderer()->setBorderColor(tgui::Color::White);
    seedSlider->setValue(static_cast<float>(config.world.seed));
    seedSlider->onValueChange([this, seedLabel](float value) {
        config.world.seed = static_cast<int>(value);
        seedLabel->setText("Seed: " + std::to_string(config.world.seed));
    });
    perlinPanel->add(seedSlider);
    yPos += spacing;
    
    // Frequency
    auto freqLabel = tgui::Label::create("Frequency: " + std::to_string(config.world.frequency));
    freqLabel->setPosition({10, yPos});
    freqLabel->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(freqLabel, "FreqLabel");
    yPos += 20;
    
    auto freqSlider = tgui::Slider::create(0.001f, 0.5f);
    freqSlider->setSize({"80%", 20});
    freqSlider->setPosition({"center", yPos});
    freqSlider->getRenderer()->setBorderColor(tgui::Color::White);
    freqSlider->setValue(config.world.frequency);
    freqSlider->onValueChange([this, freqLabel](float value) {
        config.world.frequency = value;
        freqLabel->setText("Frequency: " + std::to_string(config.world.frequency));
    });
    perlinPanel->add(freqSlider);
    yPos += spacing;
    
    // Octaves
    auto octLabel = tgui::Label::create("Octaves: " + std::to_string(config.world.octaves));
    octLabel->setPosition({10, yPos});
    octLabel->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(octLabel, "OctLabel");
    yPos += 20;
    
    auto octSlider = tgui::Slider::create(1, 10);
    octSlider->setSize({"80%", 20});
    octSlider->setPosition({"center", yPos});
    octSlider->getRenderer()->setBorderColor(tgui::Color::White);
    octSlider->setValue(static_cast<float>(config.world.octaves));
    octSlider->onValueChange([this, octLabel](float value) {
        config.world.octaves = static_cast<int>(value);
        octLabel->setText("Octaves: " + std::to_string(config.world.octaves));
    });
    perlinPanel->add(octSlider);
    yPos += spacing;
    
    // Persistence
    auto persLabel = tgui::Label::create("Persistence: " + std::to_string(config.world.persistence));
    persLabel->setPosition({10, yPos});
    persLabel->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(persLabel, "PersLabel");
    yPos += 20;
    
    auto persSlider = tgui::Slider::create(0.1f, 1.0f);
    persSlider->setSize({"80%", 20});
    persSlider->setPosition({"center", yPos});
    persSlider->getRenderer()->setBorderColor(tgui::Color::White);
    persSlider->setValue(config.world.persistence);
    persSlider->onValueChange([this, persLabel](float value) {
        config.world.persistence = value;
        persLabel->setText("Persistence: " + std::to_string(config.world.persistence));
    });
    perlinPanel->add(persSlider);
    yPos += spacing;
    
    // Lacunarity
    auto lacLabel = tgui::Label::create("Lacunarity: " + std::to_string(config.world.lacunarity));
    lacLabel->setPosition({10, yPos});
    lacLabel->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(lacLabel, "LacLabel");
    yPos += 20;
    
    auto lacSlider = tgui::Slider::create(1.0f, 4.0f);
    lacSlider->setSize({"80%", 20});
    lacSlider->setPosition({"center", yPos});
    lacSlider->getRenderer()->setBorderColor(tgui::Color::White);
    lacSlider->setValue(config.world.lacunarity);
    lacSlider->onValueChange([this, lacLabel](float value) {
        config.world.lacunarity = value;
        lacLabel->setText("Lacunarity: " + std::to_string(config.world.lacunarity));
    });
    perlinPanel->add(lacSlider);
    yPos += spacing;
    
    // Contrast
    auto contLabel = tgui::Label::create("Contrast: " + std::to_string(config.world.contrast));
    contLabel->setPosition({10, yPos});
    contLabel->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(contLabel, "ContLabel");
    yPos += 20;
    
    auto contSlider = tgui::Slider::create(1.0f, 5.0f);
    contSlider->setSize({"80%", 20});
    contSlider->setPosition({"center", yPos});
    contSlider->getRenderer()->setBorderColor(tgui::Color::White);
    contSlider->setValue(config.world.contrast);
    contSlider->onValueChange([this, contLabel](float value) {
        config.world.contrast = value;
        contLabel->setText("Contrast: " + std::to_string(config.world.contrast));
    });
    perlinPanel->add(contSlider);
    yPos += spacing;
    
    // Height Scale
    auto heightLabel = tgui::Label::create("Height Scale: " + std::to_string(config.world.heightScale));
    heightLabel->setPosition({10, yPos});
    heightLabel->getRenderer()->setTextColor(tgui::Color::White);
    perlinPanel->add(heightLabel, "HeightLabel");
    yPos += 20;
    
    auto heightSlider = tgui::Slider::create(10.0f, 200.0f);
    heightSlider->setSize({"80%", 20});
    heightSlider->setPosition({"center", yPos});
    heightSlider->getRenderer()->setBorderColor(tgui::Color::White);
    heightSlider->setValue(config.world.heightScale);
    heightSlider->onValueChange([this, heightLabel](float value) {
        config.world.heightScale = value;
        heightLabel->setText("Height Scale: " + std::to_string(config.world.heightScale));
    });
    perlinPanel->add(heightSlider);
    yPos += spacing;

    // Boutons
    auto applyBtn = tgui::Button::create("Appliquer (Recharger Carte)");
    applyBtn->setSize({"80%", 40});
    applyBtn->setPosition({"center", yPos});
    applyBtn->onClick([this]() { applyPerlinConfig(); });
    perlinPanel->add(applyBtn);
    yPos += 50;
    
    auto saveBtn = tgui::Button::create("Sauvegarder Config");
    saveBtn->setSize({"38%", 40});
    saveBtn->setPosition({"10%", yPos});
    saveBtn->onClick([this]() { 
        config.save(); 
        std::cout << "Configuration sauvegardée !" << std::endl;
    });
    perlinPanel->add(saveBtn);
    
    auto loadBtn = tgui::Button::create("Charger Config");
    loadBtn->setSize({"38%", 40});
    loadBtn->setPosition({"52%", yPos});
    loadBtn->onClick([this]() { 
        config.load(); 
        std::cout << "Configuration chargée !" << std::endl;
        // Recréer le panneau avec les nouvelles valeurs
        gui.remove(perlinPanel);
        setupPerlinConfigPanel();
        perlinPanel->setVisible(true);
    });
    perlinPanel->add(loadBtn);
}

void Game::applyPerlinConfig() {
    std::cout << "Application de la nouvelle configuration Perlin..." << std::endl;
    
    // Réinitialiser la carte avec la nouvelle config
    MapMode currentMode = MapMode::INFINITE; // Mode Perlin Play est toujours INFINITE
    
    map.reset(currentMode, config.world);
    
    std::cout << "Carte rechargée avec succès !" << std::endl;
}

void Game::togglePerlinPanel() {
    if (!perlinPanel) return;
    
    showPerlinPanel = !showPerlinPanel;
    perlinPanel->setVisible(showPerlinPanel);
    
    std::cout << (showPerlinPanel ? "Panneau Perlin affiché" : "Panneau Perlin masqué") << std::endl;
}


void Game::toggleSpawnerPanel() {
    if (!spawnerPanel) return;
    
    showSpawnerPanel = !showSpawnerPanel;
    spawnerPanel->setVisible(showSpawnerPanel);
    
    std::cout << (showSpawnerPanel ? "Panneau Spawner affiché" : "Panneau Spawner masqué") << std::endl;
}

void Game::setupSpawnerPanel() {
    spawnerPanel = tgui::Panel::create();
    spawnerPanel->setSize({"30%", "60%"});
    spawnerPanel->setPosition({"35%", "20%"});
    spawnerPanel->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40, 230));
    spawnerPanel->setVisible(false);
    gui.add(spawnerPanel, "SpawnerPanel");

    float yPos = 10.f;
    float spacing = 50.f;

    //Titre
    auto title = tgui::Label::create("Configuration Spawner");
    title->setTextSize(20);
    title->setPosition({"center", yPos});
    title->getRenderer()->setTextColor(tgui::Color::White);
    spawnerPanel->add(title);
    yPos += 40;

    //Maximum d'entités
    auto maxEntityLabel = tgui::Label::create("Max Entités: " + std::to_string(spawner.getMaxTotalEntities()));
    maxEntityLabel->setPosition({10, yPos});
    maxEntityLabel->getRenderer()->setTextColor(tgui::Color::White);
    spawnerPanel->add(maxEntityLabel, "MaxEntityLabel");
    yPos+=20;

    auto maxEntitySlider = tgui::Slider::create(0,3000);
    maxEntitySlider->setSize({"80%", 20});
    maxEntitySlider->setPosition({"center", yPos});
    maxEntitySlider->getRenderer()->setBorderColor(tgui::Color::White);
    maxEntitySlider->setValue(static_cast<float>(spawner.getMaxTotalEntities()));
    maxEntitySlider->onValueChange([this, maxEntityLabel](float value) {
        spawner.setMaxTotalEntities(static_cast<int>(value));
        maxEntityLabel->setText("Max Entités: " + std::to_string(spawner.getMaxTotalEntities()));
    });
    spawnerPanel->add(maxEntitySlider);
    yPos += spacing;

    //Maximum proies
    auto maxPreyLabel = tgui::Label::create("Max Proies: " + std::to_string(spawner.getMaxTotalPrey()));
    maxPreyLabel->setPosition({10, yPos});
    maxPreyLabel->getRenderer()->setTextColor(tgui::Color::White);
    spawnerPanel->add(maxPreyLabel, "MaxPreyLabel");
    yPos += 20;

    auto maxPreySlider = tgui::Slider::create(0,1000);
    maxPreySlider->setSize({"80%", 20});
    maxPreySlider->setPosition({"center", yPos});
    maxPreySlider->getRenderer()->setBorderColor(tgui::Color::White);
    maxPreySlider->setValue(static_cast<float>(spawner.getMaxTotalPrey()));
    maxPreySlider->onValueChange([this, maxPreyLabel](float value) {
        spawner.setMaxTotalPrey(static_cast<int>(value));
        maxPreyLabel->setText("Max Proies: " + std::to_string(spawner.getMaxTotalPrey()));
    });
    spawnerPanel->add(maxPreySlider);
    yPos += spacing;

    //Maximum prédateurs
    auto maxPredatorLabel = tgui::Label::create("Max Prédateurs: " + std::to_string(spawner.getMaxTotalPredators()));
    maxPredatorLabel->setPosition({10, yPos});
    maxPredatorLabel->getRenderer()->setTextColor(tgui::Color::White);
    spawnerPanel->add(maxPredatorLabel, "MaxPredatorLabel");
    yPos += 20;

    auto maxPredatorSlider = tgui::Slider::create(0,1000);
    maxPredatorSlider->setSize({"80%", 20});
    maxPredatorSlider->setPosition({"center", yPos});
    maxPredatorSlider->getRenderer()->setBorderColor(tgui::Color::White);
    maxPredatorSlider->setValue(static_cast<float>(spawner.getMaxTotalPredators()));
    maxPredatorSlider->onValueChange([this, maxPredatorLabel](float value) {
        spawner.setMaxTotalPredators(static_cast<int>(value));
        maxPredatorLabel->setText("Max Prédateurs: " + std::to_string(spawner.getMaxTotalPredators()));
    });
    spawnerPanel->add(maxPredatorSlider);
    yPos += spacing;

    //Maximum nourriture
    auto maxFoodLabel = tgui::Label::create("Max Nourriture: " + std::to_string(spawner.getMaxTotalFood()));
    maxFoodLabel->setPosition({10, yPos});
    maxFoodLabel->getRenderer()->setTextColor(tgui::Color::White);
    spawnerPanel->add(maxFoodLabel, "MaxFoodLabel");
    yPos += 20;

    auto maxFoodSlider = tgui::Slider::create(0,1000);
    maxFoodSlider->setSize({"80%", 20});
    maxFoodSlider->setPosition({"center", yPos});
    maxFoodSlider->getRenderer()->setBorderColor(tgui::Color::White);
    maxFoodSlider->setValue(static_cast<float>(spawner.getMaxTotalFood()));
    maxFoodSlider->onValueChange([this, maxFoodLabel](float value) {
        spawner.setMaxTotalFood(static_cast<int>(value));
        maxFoodLabel->setText("Max Nourriture: " + std::to_string(spawner.getMaxTotalFood()));
    });
    spawnerPanel->add(maxFoodSlider);
    yPos += spacing;
    
     // Boutons
    yPos += 20;
    
    auto despawnBtn = tgui::Button::create("Despawner Toutes les Entités");
    despawnBtn->setSize({"80%", 40});
    despawnBtn->setPosition({"center", yPos});
    despawnBtn->onClick([this]() { spawner.despawnAllEntities(entities); });
    spawnerPanel->add(despawnBtn);
}