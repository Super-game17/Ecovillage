#include "spawner.hpp"
#include "utilitaires.hpp"
#include <iostream>
#include <cstdlib>

EntitySpawner::EntitySpawner() {
    // Constructeur par défaut, les valeurs sont déjà initialisées
    // keepRadius peut suivre spawnRadius + 1 si besoin
}

ChunkKey EntitySpawner::getChunkKey(int gridX, int gridY) const {
    // Conversion des coordonnées de grille en coordonnées de chunk
    // En supposant que CHUNK_SIZE = 16 (ajustez selon votre Map)
    const int CHUNK_SIZE = 16;
    return ChunkKey{gridX / CHUNK_SIZE, gridY / CHUNK_SIZE};
}

bool EntitySpawner::canSpawnInChunk(const ChunkKey& chunk, EntityType type) const {
    auto it = chunkStats.find(chunk);
    if (it == chunkStats.end()) {
        return true; // Chunk vide, on peut spawn
    }
    
    const ChunkEntityStats& stats = it->second;
    
    // Vérifier le total
    if (stats.getTotalCount() >= maxTotalPerChunk) {
        return false;
    }
    
    // Vérifier selon le type
    switch(type) {
        case EntityType::PREY:
            return stats.preyCount < maxPreyPerChunk;
        case EntityType::PREDATOR:
            return stats.predatorCount < maxPredatorPerChunk;
        case EntityType::FOOD:
            return stats.foodCount < maxFoodPerChunk;
        default:
            return true;
    }
}

void EntitySpawner::updateChunkStats(const std::vector<Entity*>& entities) {
    // Réinitialiser les stats
    chunkStats.clear();
    
    // Compter les entités par chunk
    for (const Entity* e : entities) {
        if (!e->isAlive) continue;
        
        ChunkKey chunk = getChunkKey(e->gridX, e->gridY);
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

bool EntitySpawner::findFreePosition(const Map& map, int chunkX, int chunkY, int& outX, int& outY, int maxAttempts) {
    const int CHUNK_SIZE = 16;
    int baseX = chunkX * CHUNK_SIZE;
    int baseY = chunkY * CHUNK_SIZE;
    
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        int x = baseX + (rand() % CHUNK_SIZE);
        int y = baseY + (rand() % CHUNK_SIZE);
        
        if (!map.isObstacle(x, y) && !map.isWater(x, y)) {
            outX = x;
            outY = y;
            return true;
        }
    }
    
    return false;
}

void EntitySpawner::update(float deltaTime, std::vector<Entity*>& entities, const Map& map, const sf::Vector2f& cameraCenter,
                           const Entity* pinA, const Entity* pinB) {
    // Mettre à jour les statistiques
    updateChunkStats(entities);

    // Calculer le chunk central (caméra)
    int cameraTileX = static_cast<int>(cameraCenter.x / 100); // tileWidth = 100
    int cameraTileY = static_cast<int>(cameraCenter.y / 50); // tileHeight = 50
    ChunkKey centerChunk = getChunkKey(cameraTileX, cameraTileY);

    // Ajuster dynamiquement le keepRadius au moins à la distance de rendu
    int effectiveKeepRadius = std::max(keepRadius, map.renderDistance + 1);

    // Mettre à jour les timers
    preySpawnTimer += deltaTime;
    predatorSpawnTimer += deltaTime;
    foodSpawnTimer += deltaTime;

    // --- SPAWN PREY ---
    if (preySpawnTimer >= preySpawnInterval) {
        preySpawnTimer = 0.f;
        
        // Essayer de spawn dans les chunks autour de la caméra
        for (int dx = -spawnRadius; dx <= spawnRadius; dx++) {
            for (int dy = -spawnRadius; dy <= spawnRadius; dy++) {
                ChunkKey targetChunk{centerChunk.x + dx, centerChunk.y + dy};
                if (canSpawnInChunk(targetChunk, EntityType::PREY)) {
                    int spawnX, spawnY;
                    if (findFreePosition(map, targetChunk.x, targetChunk.y, spawnX, spawnY)) {
                        Prey* p = new Prey(spawnX, spawnY);
                        p->updateVisualPosition(map);
                        entities.push_back(p);
                        
                        // Mettre à jour les stats immédiatement
                        chunkStats[targetChunk].preyCount++;
                        
                        // Une seule proie par cycle
                        goto prey_spawned;
                    }
                }
            }
        }
        prey_spawned:;
    }

    // --- SPAWN PREDATOR ---
    if (predatorSpawnTimer >= predatorSpawnInterval) {
        predatorSpawnTimer = 0.f;
        for (int dx = -spawnRadius; dx <= spawnRadius; dx++) {
            for (int dy = -spawnRadius; dy <= spawnRadius; dy++) {
                ChunkKey targetChunk{centerChunk.x + dx, centerChunk.y + dy};
                if (canSpawnInChunk(targetChunk, EntityType::PREDATOR)) {
                    int spawnX, spawnY;
                    if (findFreePosition(map, targetChunk.x, targetChunk.y, spawnX, spawnY)) {
                        Predator* p = new Predator(spawnX, spawnY);
                        p->updateVisualPosition(map);
                        entities.push_back(p);
                        chunkStats[targetChunk].predatorCount++;
                        goto predator_spawned;
                    }
                }
            }
        }
        predator_spawned:;
    }

    // --- SPAWN FOOD ---
    if (foodSpawnTimer >= foodSpawnInterval) {
        foodSpawnTimer = 0.f;
        
        // Spawn multiple food items
        int foodToSpawn = 3; // Nombre de nourriture à spawn par cycle
        for (int i = 0; i < foodToSpawn; i++) {
            for (int dx = -spawnRadius; dx <= spawnRadius; dx++) {
                for (int dy = -spawnRadius; dy <= spawnRadius; dy++) {
                    ChunkKey targetChunk{centerChunk.x + dx, centerChunk.y + dy};
                    if (canSpawnInChunk(targetChunk, EntityType::FOOD)) {
                        int spawnX, spawnY;
                        if (findFreePosition(map, targetChunk.x, targetChunk.y, spawnX, spawnY)) {
                            Food* f = new Food(spawnX, spawnY);
                            f->updateVisualPosition(map);
                            entities.push_back(f);
                            chunkStats[targetChunk].foodCount++;
                            goto next_food;
                        }
                    }
                }
            }
            next_food:;
        }
    }

    // DESPAWN: supprimer ce qui est hors du rayon de conservation
    // Utilise effectiveKeepRadius (>= renderDistance + 1)
    {
        // Truc simple: désactiver temporairement keepRadius avec effectiveKeepRadius local
        // et passer à la fonction utilitaire
        int savedKeep = keepRadius;
        keepRadius = effectiveKeepRadius;
        despawnFarEntities(entities, centerChunk, map, pinA, pinB);
        keepRadius = savedKeep;
    }

    // Recalculer les stats après spawn/despawn
    updateChunkStats(entities);
}

void EntitySpawner::despawnFarEntities(std::vector<Entity*>& entities, const ChunkKey& centerChunk, const Map& map,
                                       const Entity* pinA, const Entity* pinB) {
    auto isPinned = [&](const Entity* e) {
        return (e == pinA) || (e == pinB);
    };

    for (size_t i = 0; i < entities.size();) {
        Entity* e = entities[i];
        if (!e->isAlive) {
            delete e;
            entities.erase(entities.begin() + i);
            continue;
        }

        ChunkKey eChunk = getChunkKey(e->gridX, e->gridY);
        int dx = std::abs(eChunk.x - centerChunk.x);
        int dy = std::abs(eChunk.y - centerChunk.y);

        if ((dx > keepRadius || dy > keepRadius) && !isPinned(e)) {
            delete e;
            entities.erase(entities.begin() + i);
            continue;
        }

        ++i;
    }
}

void EntitySpawner::updateFoodOnly(float deltaTime, std::vector<Entity*>& entities, const Map& map, const sf::Vector2f& cameraCenter) {
    // Mettre à jour les statistiques
    updateChunkStats(entities);

    // Calculer le chunk central (caméra)
    int cameraTileX = static_cast<int>(cameraCenter.x/100);
    int cameraTileY = static_cast<int>(cameraCenter.y/50);
    ChunkKey centerChunk = getChunkKey(cameraTileX, cameraTileY);

    // Mettre à jour le timer de nourriture
    foodSpawnTimer += deltaTime;

    // --- SPAWN FOOD UNIQUEMENT ---
    if (foodSpawnTimer >= foodSpawnInterval) {
        foodSpawnTimer = 0.f;
        
        // Spawn multiple food items
        int foodToSpawn = 5; // Plus de nourriture en mode simulation
        for (int i = 0; i < foodToSpawn; i++) {
            for (int dx = -spawnRadius; dx <= spawnRadius; dx++) {
                for (int dy = -spawnRadius; dy <= spawnRadius; dy++) {
                    ChunkKey targetChunk{centerChunk.x + dx, centerChunk.y + dy};
                    if (canSpawnInChunk(targetChunk, EntityType::FOOD)) {
                        int spawnX, spawnY;
                        if (findFreePosition(map, targetChunk.x, targetChunk.y, spawnX, spawnY)) {
                            Food* f = new Food(spawnX, spawnY);
                            f->updateVisualPosition(map);
                            entities.push_back(f);
                            chunkStats[targetChunk].foodCount++;
                            goto next_food;
                        }
                    }
                }
            }
            next_food:;
        }
    }

    // Nettoyer les entités mortes (nourriture mangée)
    for (size_t i = 0; i < entities.size();) {
        Entity* e = entities[i];
        if (!e->isAlive) {
            delete e;
            entities.erase(entities.begin() + i);
            continue;
        }
        ++i;
    }

    // Recalculer les stats après spawn
    updateChunkStats(entities);
}

void EntitySpawner::clearChunkStats(const ChunkKey& chunk) {
    chunkStats.erase(chunk);
}

ChunkEntityStats EntitySpawner::getChunkStats(const ChunkKey& chunk) const {
    auto it = chunkStats.find(chunk);
    if (it != chunkStats.end()) {
        return it->second;
    }
    return ChunkEntityStats();
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