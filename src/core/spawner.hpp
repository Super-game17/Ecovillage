#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include "entity.hpp"
#include "map.hpp"

// Structure pour identifier un chunk
struct ChunkKey {
    int x, y;
    
    bool operator==(const ChunkKey& other) const {
        return x == other.x && y == other.y;
    }
};

// Hash pour ChunkKey
namespace std {
    template<>
    struct hash<ChunkKey> {
        size_t operator()(const ChunkKey& k) const {
            return hash<int>()(k.x) ^ (hash<int>()(k.y) << 1);
        }
    };
}

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
    // Limites par chunk
    int maxPreyPerChunk = 8;
    int maxPredatorPerChunk = 3;
    int maxFoodPerChunk = 15;
    int maxTotalPerChunk = 20;
    
    // Statistiques par chunk
    std::unordered_map<ChunkKey, ChunkEntityStats> chunkStats;
    
    // Timers de spawn
    float preySpawnTimer = 0.f;
    float predatorSpawnTimer = 0.f;
    float foodSpawnTimer = 0.f;
    
    // Intervalles de spawn (secondes)
    float preySpawnInterval = 5.0f;
    float predatorSpawnInterval = 10.0f;
    float foodSpawnInterval = 2.0f;
    
    // Distance de spawn autour de la caméra (en chunks)
    int spawnRadius = 3;
    int keepRadius = 5; // rayon de conservation; au-delà on despawn

    // Méthodes privées
    ChunkKey getChunkKey(int gridX, int gridY) const;
    bool canSpawnInChunk(const ChunkKey& chunk, EntityType type) const;
    void updateChunkStats(const std::vector<Entity*>& entities);
    bool findFreePosition(const Map& map, int chunkX, int chunkY, int& outX, int& outY, int maxAttempts = 10);
    void despawnFarEntities(std::vector<Entity*>& entities, const ChunkKey& centerChunk, const Map& map,
                            const Entity* pinA, const Entity* pinB);

public:
    EntitySpawner();
    // Mise à jour avec épingles pour ne pas despawn la sélection/contrôle
    void update(float deltaTime, std::vector<Entity*>& entities, const Map& map, const sf::Vector2f& cameraCenter,
                const Entity* pinA = nullptr, const Entity* pinB = nullptr);

    // Nettoyage des stats pour un chunk
    void clearChunkStats(const ChunkKey& chunk);
    
    // Getters/Setters pour les limites
    void setMaxPreyPerChunk(int max) { maxPreyPerChunk = max; }
    void setMaxPredatorPerChunk(int max) { maxPredatorPerChunk = max; }
    void setMaxFoodPerChunk(int max) { maxFoodPerChunk = max; }
    void setMaxTotalPerChunk(int max) { maxTotalPerChunk = max; }
    void setSpawnRadius(int radius) { spawnRadius = radius; }
    void setKeepRadius(int radius) { keepRadius = radius; }
    int getMaxPreyPerChunk() const { return maxPreyPerChunk; }
    int getMaxPredatorPerChunk() const { return maxPredatorPerChunk; }
    int getMaxFoodPerChunk() const { return maxFoodPerChunk; }
    int getMaxTotalPerChunk() const { return maxTotalPerChunk; }
    int getSpawnRadius() const { return spawnRadius; }
    int getKeepRadius() const { return keepRadius; }
    
    // Obtenir les stats d'un chunk
    ChunkEntityStats getChunkStats(const ChunkKey& chunk) const;
    
    // Debug: afficher les stats
    void printStats() const;
    // Mise à jour uniquement de la nourriture (pour mode simulation)
    void updateFoodOnly(float deltaTime, std::vector<Entity*>& entities, const Map& map, const sf::Vector2f& cameraCenter);

};