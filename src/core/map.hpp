#ifndef MAP_HPP
#define MAP_HPP
#include "chunk.hpp"

class Map {
public:
    // Map pour stocker les chunks
    std::map<ChunkCoord, Chunk> chunks;
    // Vecteur des chunks à rendre
    std::vector<const Chunk*> chunksToRender;
    // Origine des chunks
    sf::Vector2f chunkOrigin;
    // Distance de rendu
    int renderDistance = 3;
    FastNoiseLite noise; // Le bruit est stocké ici maintenant
    void initNoise();    // Pour le configurer

    // CONSTRUCTEUR : Initialisation correcte de chunkOrigin
    Map() : chunkOrigin(swidth / 2.0f, sheight / 2.0f) {
        initNoise();
    }

    // Nouvelle méthode pour savoir la hauteur du sol n'importe où
    int getGroundLevel(float x, float y) const;

    // Méthodes pour gérer la map (déclarations)
    void update(const sf::Vector2f& cameraPos, const sf::Texture& texture);
    void render(sf::RenderWindow& window, sf::Texture& texture) const;
    bool isObstacle(int x, int y) const;

    
};

#endif // MAP_HPP