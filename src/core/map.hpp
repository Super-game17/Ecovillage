#ifndef MAP_HPP
#define MAP_HPP
#include "chunk.hpp"
#include "gameConfig.hpp"

class Entity;

enum class MapMode {
    INFINITE, //Free mode, Perlin play
    FINITE // Simulation, Survival mode
};

class Map {
public:
    //Inițialisation des attributs de la classe Map
    // Map pour stocker les chunks
    std::map<ChunkCoord, Chunk> chunks;
    // Vecteur des chunks à rendre
    std::vector<const Chunk*> chunksToRender;
    // Origine des chunks
    sf::Vector2f chunkOrigin;
    // Distance de rendu
    int renderDistance = 3;
    FastNoiseLite noise; // Le bruit est stocké ici maintenant

    // Optimization: Track last update position
    int lastCenterChunkX = -99999;
    int lastCenterChunkY = -99999;

    // Paramètres actuels du monde
    WorldConfig currentConfig;
    MapMode currentMode = MapMode::INFINITE;
    void initNoise(const WorldConfig& config);    // Pour le configurer

    // CONSTRUCTEUR : Initialisation correcte de chunkOrigin
    Map() : chunkOrigin(swidth / 2.0f, sheight / 2.0f) {
        initNoise(currentConfig);
    }

    // Réinitialise totalement la map (pour changer de mode)
    void reset(MapMode mode, const WorldConfig& config);

    //Méthode pour savoir la hauteur du sol n'importe où
    int getGroundLevel(float x, float y) const;

    // Méthodes pour gérer la map (déclarations)
    void update(const sf::Vector2f& cameraPos, const sf::Texture& texture);
    // MODIFICATION : On accepte maintenant une liste d'entités à dessiner
    // On garde 'player' pour savoir autour de qui calculer la transparence des arbres
    void render(sf::RenderWindow& window, sf::Texture& texture, const std::vector<Entity*>& entities, const Entity* player = nullptr) const;

    // Rendu Rapide (Pour Perlin-Play - sans tri d'entités complexe)
    void renderFast(sf::RenderWindow& window, sf::Texture& texture) const;
    
    bool isObstacle(int x, int y) const;
    bool isWater(int x, int y) const;
    
};

#endif // MAP_HPP
