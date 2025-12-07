#ifndef CHUNK_HPP
#define CHUNK_HPP
#include "FastNoiseLite.h"
#include "utilitaires.hpp"
#include<map>
#include <iostream>


class Map; // Déclaration anticipée
class Entity; // Déclaration anticipée
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
    //Initialisation des attributs de la classe Chunk
    static constexpr int SIZE = 16;
    std::map<int, std::vector<sf::Vertex>> slices; // Vertices par profondeur CPU
    std::map<int, sf::VertexBuffer> layerBuffers; // GPU buffers par profondeur
    std::vector<sf::Vertex> vertices;//Pour rendre le chunk rapidement Vertices CPU
    sf::VertexBuffer vertexBuffer;//Buffer GPU pour rendu rapide
    //Positions du chunk dans la grille de chunks
    int chunkX, chunkY;

    //Constructeur du Chunk
    Chunk(int cx, int cy, int tileW, int tileH, Map& carte) ;

    //Différentes méthodes pour gérer le chunk
    // Méthode pour ajouter un bloc au chunk
    void addBlock(float worldX, float worldY, int zLevel, const sf::Color& color);
    // Méthode pour générer un arbre à une position donnée
    void generateTree(int worldX, int worldY, int zGroundLevel);
    // Méthode pour construire les buffers GPU à partir des vertices CPU
    void buildBuffers();
    void buildFastBuffer();

    // Méthode pour dessiner une couche spécifique du chunk
    void drawLayer(sf::RenderTarget& target, sf::RenderStates states, int localDepth, int globalDepth,const Entity* player = nullptr) const;

    // Méthode pour le rendu rapide (sans tri par profondeur)
    void drawFast(sf::RenderTarget& target, sf::RenderStates states) const;
    
    //Méthode pour overlay intelligent
    sf::FloatRect getBlockBounds(int worldX, int worldY, int zLevel) const;

    // Méthode pour calculer overlap entre deux rectangles
    static float calculateOverlap(const sf::FloatRect& block, const sf::FloatRect& player);
};

#endif //CHUNK_HPP
