#ifndef MAP_HPP
#define MAP_HPP
#include "chunk.hpp"

struct Map {
    
    unsigned int swidth = 1280;
    unsigned int sheight = 720;
    // Map pour stocker les chunks
    std::map<ChunkCoord, Chunk> chunks;
    // Vecteur des chunks à rendre
    std::vector<const Chunk*> chunksToRender;
    // Origine des chunks
    sf::Vector2f chunkOrigin;
    // Distance de rendu
    int renderDistance = 3;

    // CONSTRUCTEUR : Initialisation correcte de chunkOrigin
    Map() : chunkOrigin(static_cast<float>(swidth) / 2.0f, static_cast<float>(sheight) / 2.0f) {}

    // Méthodes pour gérer la map (déclarations)
    void update(const sf::Vector2f& cameraPos, const sf::Texture& texture);
    void render(sf::RenderWindow& window, sf::Texture& texture) const;
};

#endif // MAP_HPP