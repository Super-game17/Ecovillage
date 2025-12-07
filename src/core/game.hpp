#ifndef GAME_HPP
#define GAME_HPP

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "camera.hpp"
#include "gameConfig.hpp"
#include "spawner.hpp"

// Les 4 modes de jeu + le menu
enum class GameState {
    MENU,
    PERLIN_PLAY,
    FREE_MODE,
    SIMULATION_MODE,
    SURVIVAL_MODE
};

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;
    tgui::Gui gui;
    Camera camera;
    Map map;
    GameConfig config;
    EntitySpawner spawner; 
    sf::Texture textureBlocks;
    sf::Texture deerText;
    sf::Texture bearText;
    sf::Clock gameClock;

    GameState currentState = GameState::MENU;
    std::vector<Entity*> entities;
    Entity* selectedEntity = nullptr; // L'entité actuellement sélectionnée
    size_t selectedIndex = 0; // Index de l'entité sélectionnée dans le vecteur

    // --- Fonctions de Gestion d'État ---
    void setupMenu();
    void startGame(GameState newState, MapMode mapMode);
    void setupPerlinPlay();
    void setupSimulationMode();
    void setupFreeMode();
    void setupSurvivalMode();

    // Widgets UI pour le jeu
    tgui::Label::Ptr statsLabel;
    tgui::Panel::Ptr perlinPanel;
    bool showPerlinPanel = false;


    // --- Fonctions de Sélection ---
    void selectNextEntity();
    void selectPreviousEntity();
    void deselectEntity();
    void updateSelectedEntity(); // Met à jour le pointeur après modifications du vecteur

    // Nouvelles fonctions pour l'UI en jeu
    void setupStatsLabel();         // Créer le label de stats (modes avec entités)
    void updateStatsDisplay();      // Mettre à jour les stats affichées
    void setupPerlinConfigPanel();  // Créer le panneau Perlin (mode Perlin Play uniquement)
    void applyPerlinConfig();       // Appliquer la config et recharger
    void togglePerlinPanel();       // Afficher/masquer le panneau Perlin
    
    // --- Boucle Principale ---
    void handleEvents();
    void update(float deltaTime);
    void render();
    // --- Fonction de Spawn Initial ---
    void spawnInitialEntities(int preyCount, int predCount, int spawnRadius);
};

#endif // GAME_HPP