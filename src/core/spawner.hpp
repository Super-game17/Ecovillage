#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include "entity.hpp"
#include "map.hpp"

// Statistiques d'entités par chunk
struct ChunkEntityStats {
    int preyCount = 0;
    int predatorCount = 0;
    int foodCount = 0;
    
    int getTotalCount() const {
        return preyCount + predatorCount + foodCount;
    }
};

class EntitySpawner {
private:

    // Limites GLOBALES (en plus des limites par chunk)
    int maxTotalEntities = 500;      // Limite totale d'entités
    int maxTotalPrey = 200;          // Limite totale de proies
    int maxTotalPredators = 50;      // Limite totale de prédateurs
    int maxTotalFood = 300;          // Limite totale de nourriture


    // Limites par chunk
    int maxPreyPerChunk = 4;
    int maxPredatorPerChunk = 2;
    int maxFoodPerChunk = 12;
    int maxTotalPerChunk = 20;
    
    // Statistiques par chunk
    std::map<ChunkCoord, ChunkEntityStats> chunkStats; // ChunkCoord au lieu de ChunkKey
    
    // Timers de spawn
    float preySpawnTimer = 0.f;
    float predatorSpawnTimer = 0.f;
    float foodSpawnTimer = 0.f;
    
    // Intervalles de spawn (secondes)
    float preySpawnInterval = 15.0f;
    float predatorSpawnInterval = 30.0f;
    float foodSpawnInterval = 20.0f;
    
    // Distance de spawn autour de la caméra (en chunks)
    int spawnRadius = 3;
    int keepRadius = 5; // rayon de conservation; au-delà on despawn


    // Méthodes privées
    
    void updateChunkStats(const std::vector<Entity*>& entities);
    bool findFreePosition(const Map& map, const Chunk* chunk, int& outX, int& outY, int maxAttempts = 20);
    void despawnFarEntities(std::vector<Entity*>& entities, const ChunkCoord& centerChunk, const Entity* pinA);

public:
    EntitySpawner();
    
    void update(float deltaTime, std::vector<Entity*>& entities, const Map& map,
                const Entity* pinA = nullptr, sf::Texture* deerTexture = nullptr, sf::Texture* bearTexture = nullptr);

    void updateFoodOnly(float deltaTime, std::vector<Entity*>& entities, const Map& map);
    
    // Getters/Setters pour les limites
    void setMaxPreyPerChunk(int max) { maxPreyPerChunk = max; }
    void setMaxPredatorPerChunk(int max) { maxPredatorPerChunk = max; }
    void setMaxFoodPerChunk(int max) { maxFoodPerChunk = max; }
    void setMaxTotalPerChunk(int max) { maxTotalPerChunk = max; }
    void setSpawnRadius(int radius) { spawnRadius = radius; }
    void setKeepRadius(int radius) { keepRadius = radius; }
    
    // Setters pour les limites globales
    void setMaxTotalEntities(int max) { maxTotalEntities = max; }
    void setMaxTotalPrey(int max) { maxTotalPrey = max; }
    void setMaxTotalPredators(int max) { maxTotalPredators = max; }
    void setMaxTotalFood(int max) { maxTotalFood = max; }

    ChunkEntityStats getChunkStats(const ChunkCoord& chunk) const;
    void printStats() const;
};