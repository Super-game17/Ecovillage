#include "spawner.hpp"
#include "utilitaires.hpp"
#include <iostream>
#include <cstdlib>
#include <algorithm>

EntitySpawner::EntitySpawner() = default;

void EntitySpawner::updateChunkStats(const std::vector<Entity*>& entities) {
    chunkStats.clear();
    
    for (const Entity* e : entities) {
        if (!e->isAlive) continue;
        
        ChunkCoord chunk(e->gridX / Chunk::SIZE, e->gridY / Chunk::SIZE);
        ChunkEntityStats& stats = chunkStats[chunk];
        
        switch(e->type) {
            case EntityType::PREY:
                stats.preyCount++;
                break;
            case EntityType::PREDATOR:
                stats.predatorCount++;
                break;
            case EntityType::FOOD:
                stats.foodCount++;
                break;
            default:
                break;
        }
    }
}

bool EntitySpawner::findFreePosition(const Map& map, const Chunk* chunk, int& outX, int& outY, int maxAttempts) {
    if (!chunk) {
        outX = 0;
        outY = 0;
        return false;
    }
    
    int baseX = chunk->chunkX * Chunk::SIZE;
    int baseY = chunk->chunkY * Chunk::SIZE;
    
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        int x = baseX + (rand() % Chunk::SIZE);
        int y = baseY + (rand() % Chunk::SIZE);
        
        if (!map.isObstacle(x, y) && !map.isWater(x, y)) {
            outX = x;
            outY = y;
            return true;
        }
    }
    
    outX = baseX + (Chunk::SIZE / 2);
    outY = baseY + (Chunk::SIZE / 2);
    return false;
}

void EntitySpawner::update(float deltaTime, std::vector<Entity*>& entities, const Map& map,
                           const Entity* pinA, sf::Texture* deerTexture, sf::Texture* bearTexture) {
    
    // Nettoyer les morts
    for (size_t i = 0; i < entities.size();) {
        if (!entities[i]->isAlive && entities[i] != pinA) {
            delete entities[i];
            entities.erase(entities.begin() + i);
            continue;
        }
        ++i;
    }
    
    // Mettre à jour les stats une seule fois
    updateChunkStats(entities);
    
    // COMPTER LES ENTITÉS GLOBALES
    int globalPreyCount = 0;
    int globalPredCount = 0;
    int globalFoodCount = 0;
    int globalTotalCount = 0;
    
    for (const auto& [chunk, stats] : chunkStats) {
        globalPreyCount += stats.preyCount;
        globalPredCount += stats.predatorCount;
        globalFoodCount += stats.foodCount;
    }
    globalTotalCount = globalPreyCount + globalPredCount + globalFoodCount;
    
    // Mettre à jour les timers
    preySpawnTimer += deltaTime;
    predatorSpawnTimer += deltaTime;
    foodSpawnTimer += deltaTime;

    // === SPAWN DANS LES CHUNKS VISIBLES ===
    // On utilise map.chunksToRender qui contient déjà les chunks à rendre autour de la caméra
    for (const Chunk* chunk : map.chunksToRender) {
        if (!chunk) continue;
        
        ChunkCoord coord(chunk->chunkX, chunk->chunkY);
        ChunkEntityStats& stats = chunkStats[coord];

        // --- SPAWN PREY ---
        if (preySpawnTimer >= preySpawnInterval && 
            stats.preyCount < maxPreyPerChunk &&
            globalPreyCount < maxTotalPrey &&
            globalTotalCount < maxTotalEntities) {
            
            int spawnX = 0, spawnY = 0;
            if (findFreePosition(map, chunk, spawnX, spawnY)) {
                auto* p = new Prey(spawnX, spawnY, deerTexture);
                p->updateVisualPosition(map);
                entities.push_back(p);
                stats.preyCount++;
                globalPreyCount++;
                globalTotalCount++;
            }
        }

        // --- SPAWN PREDATOR ---
        if (predatorSpawnTimer >= predatorSpawnInterval && 
            stats.predatorCount < maxPredatorPerChunk &&
            globalPredCount < maxTotalPredators &&
            globalTotalCount < maxTotalEntities) {
            
            int spawnX = 0, spawnY = 0;
            if (findFreePosition(map, chunk, spawnX, spawnY)) {
                auto* p = new Predator(spawnX, spawnY, bearTexture);
                p->updateVisualPosition(map);
                entities.push_back(p);
                stats.predatorCount++;
                globalPredCount++;
                globalTotalCount++;
            }
        }

        // --- SPAWN FOOD ---
        if (foodSpawnTimer >= foodSpawnInterval && 
            stats.foodCount < maxFoodPerChunk &&
            globalFoodCount < maxTotalFood &&
            globalTotalCount < maxTotalEntities) {
            
            int spawnX = 0, spawnY = 0;
            if (findFreePosition(map, chunk, spawnX, spawnY)) {
                auto* f = new Food(spawnX, spawnY);
                f->updateVisualPosition(map);
                entities.push_back(f);
                stats.foodCount++;
                globalFoodCount++;
                globalTotalCount++;
            }
        }
    }

    // Reset timers
    if (preySpawnTimer >= preySpawnInterval) preySpawnTimer = 0.f;
    if (predatorSpawnTimer >= predatorSpawnInterval) predatorSpawnTimer = 0.f;
    if (foodSpawnTimer >= foodSpawnInterval) foodSpawnTimer = 0.f;

    // === DESPAWN LES ENTITÉS LOINTAINES ===
    despawnFarEntities(entities, map.chunks.empty() ? ChunkCoord(0, 0) : 
        ChunkCoord(map.lastCenterChunkX, map.lastCenterChunkY), pinA);
    
    // Recalculer les stats après spawn/despawn
    updateChunkStats(entities);
}

void EntitySpawner::despawnFarEntities(std::vector<Entity*>& entities, const ChunkCoord& centerChunk, const Entity* pinA) {
    for (size_t i = 0; i < entities.size();) {
        Entity* e = entities[i];
        
        if (e == pinA) { ++i; continue; }
        
        ChunkCoord eChunk(e->gridX / Chunk::SIZE, e->gridY / Chunk::SIZE);
        int dx = std::abs(eChunk.x - centerChunk.x);
        int dy = std::abs(eChunk.y - centerChunk.y);

        // Despawn si au-delà du rayon de conservation
        if (std::max(dx, dy) > keepRadius) {
            int despawnX = e->gridX;
            int despawnY = e->gridY;
            delete e;
            entities.erase(entities.begin() + i);
            std::cout << "Entity despawned at (" << despawnX << ", " << despawnY << ")\n";
            continue;
        }

        ++i;
    }
}

void EntitySpawner::updateFoodOnly(float deltaTime, std::vector<Entity*>& entities, const Map& map) {
    updateChunkStats(entities);
    foodSpawnTimer += deltaTime;

    if (foodSpawnTimer >= foodSpawnInterval) {
        foodSpawnTimer = 0.f;
        
        for (const Chunk* chunk : map.chunksToRender) {
            if (!chunk) continue;
            
            ChunkCoord coord(chunk->chunkX, chunk->chunkY);
            ChunkEntityStats& stats = chunkStats[coord];

            if (stats.foodCount < maxFoodPerChunk) {
                int spawnX = 0, spawnY = 0;
                if (findFreePosition(map, chunk, spawnX, spawnY)) {
                    auto* f = new Food(spawnX, spawnY);
                    f->updateVisualPosition(map);
                    entities.push_back(f);
                    stats.foodCount++;
                }
            }
        }
    }

    // Nettoyer les morts
    for (size_t i = 0; i < entities.size();) {
        if (!entities[i]->isAlive) {
            delete entities[i];
            entities.erase(entities.begin() + i);
            continue;
        }
        ++i;
    }

    updateChunkStats(entities);
}

ChunkEntityStats EntitySpawner::getChunkStats(const ChunkCoord& chunk) const {
    auto it = chunkStats.find(chunk);
    return (it != chunkStats.end()) ? it->second : ChunkEntityStats();
}

void EntitySpawner::printStats() const {
    std::cout << "=== SPAWNER STATS ===" << std::endl;
    std::cout << "Total chunks tracked: " << chunkStats.size() << std::endl;
    for (const auto& [chunk, stats] : chunkStats) {
        std::cout << "Chunk (" << chunk.x << "," << chunk.y << "): "
                  << "Prey=" << stats.preyCount << " "
                  << "Predator=" << stats.predatorCount << " "
                  << "Food=" << stats.foodCount << std::endl;
    }
}