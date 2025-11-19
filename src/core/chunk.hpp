#ifndef CHUNK_HPP
#define CHUNK_HPP
#include "SFML/Graphics.hpp"
#include "FastNoiseLite.h"
#include "utilitaires.hpp"
#include<map>
#include <iostream>


class Map; // Déclaration anticipée
// Structure pour représenter les coordonnées d'un chunk par rapport à la grille de chunks
struct ChunkCoord {
    int x, y;
    
    ChunkCoord(int x_, int y_) : x(x_), y(y_) {}
    
    bool operator<(const ChunkCoord& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

// Structure de Chunk
class Chunk {
public:
    static constexpr int SIZE = 16;
    std::vector<sf::Vertex> vertices;
    sf::VertexBuffer vertexBuffer;
    int chunkX, chunkY;
    sf::Vector2f position;

    Chunk() = default;

    Chunk(int cx, int cy, int tileW, int tileH, Map& carte) ;

    // Helper pour ajouter un "cube/tuile" visuel au vecteur de vertices
    void addTile(float worldX, float worldY, int zLevel, const sf::Color& color);

    // Helper pour générer un arbre à une position donnée
    void generateTree(int worldX, int worldY, int zGroundLevel);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif //CHUNK_HPP