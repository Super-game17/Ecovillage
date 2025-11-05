// Map.hpp
#ifndef MAP_HPP
#define MAP_HPP

#include <SFML/Graphics.hpp>
#include "perlinnoise.hpp"
#include <map>
#include <vector>
#include <memory>

// Structure pour stocker les données d'un seul bloc (voxel)
struct BlockData {
    int height;
    sf::Color color;
};

// Un Chunk est une collection de données de blocs, stockée dans un Vector 2D
struct Chunk {
    // Par exemple, un chunk de 16x16 tuiles
    static const int SIZE = 16; 
    std::vector<std::vector<BlockData>> blocks;
    bool generated;

    Chunk() : generated(false) {
        blocks.resize(SIZE, std::vector<BlockData>(SIZE));
    }
};

// La classe qui gère tous les chunks
class MapManager {
public:
    MapManager(unsigned int windowWidth, unsigned int windowHeight, PerlinNoise& pn);

    // Met à jour et génère les chunks autour de la position de la caméra
    void update(sf::Vector2f cameraCenter);

    // Dessine uniquement les chunks visibles
    void draw(sf::RenderWindow& window, sf::Sprite& blockSprite);

private:
    // La fonction isoToScreen doit être accessible à la MapManager
    sf::Vector2f isoToScreen(int x, int y, int z_height);

    sf::Vector2i screenToIso(sf::Vector2f screenPos);
    
    // Génère les données brutes pour un chunk spécifique
    void generateChunk(int chunkX, int chunkY);

    // Stocke tous les chunks chargés. La clé est un pair (chunkX, chunkY)
    std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks;
    
    PerlinNoise& pn;
    unsigned int windowW, windowH;
    
    // Paramètres de terrain
    const int TILE_WIDTH = 100;
    const int TILE_HEIGHT = 50;
    const double FREQUENCY = 0.1;
    const int HEIGHT_MULTIPLIER = 5;
    const int RENDER_DISTANCE = 3; // 3 chunks autour du centre
};

#endif // MAP_HPP
