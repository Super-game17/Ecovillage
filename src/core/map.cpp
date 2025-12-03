#include "map.hpp"
#include "entity.hpp"
#include <algorithm>
#include <vector>
#include <cmath>

void Map::initNoise(const WorldConfig& config){
    currentConfig = config; // Sauvegarde la config actuelle
    
    noise.SetSeed(config.seed);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(config.frequency);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(config.octaves);
    noise.SetFractalLacunarity(config.lacunarity);
    noise.SetFractalGain(config.persistence);
}

void Map::reset(MapMode mode, const WorldConfig& config) {
    // Tout nettoyer
    chunks.clear();
    chunksToRender.clear();
    lastCenterChunkX = -99999;
    lastCenterChunkY = -99999;
    
    // Appliquer les nouveaux paramètres
    currentMode = mode;
    initNoise(config);
    
    // Si le mode est FINI, on peut prégénérer une zone ici (ex: 4x4 chunks)
    // Pour l'instant on laisse l'update le faire, mais on limitera l'update
}

int Map::getGroundLevel(float worldX, float worldY) const{
    // Obtenir la valeur du bruit pour cette coordonnée
    //    On utilise les coordonnées MONDE (worldX, worldY)
    //    GetNoise() renvoie une valeur entre -1.0 et 1.0
    float noiseValue = noise.GetNoise(worldX, worldY);
    float noise01 = (noiseValue + 1.0f) * 0.5f; // Convertir [-1,1] en [0,1]
    // Élévation exponentielle (comme dans ton exemple avec pow)
    noise01 = std::pow(noise01, currentConfig.contrast); // Augmente le contraste
    // Convertir le bruit en niveaux de hauteur entiers (ex: 0 à 10)
    // (noiseValue + 1.0) -> [0, 2]
    // * 5.0 -> [0, 10]
    // 5. Calculer la hauteur finale
    int zLevel = static_cast<int>(noise01 * currentConfig.heightScale); 
    return zLevel;
}


void Map::update (const sf::Vector2f& cameraPos, const sf::Texture& texture){

    sf::Vector2i tileCenter = isoToCartesian(cameraPos.x, cameraPos.y,tileWidth, tileHeight);
    int centerChunkX = static_cast<int>(std::floor(static_cast<float>(tileCenter.x) / Chunk::SIZE));
    int centerChunkY = static_cast<int>(std::floor(static_cast<float>(tileCenter.y) / Chunk::SIZE));

    // OPTIMIZATION: Only update chunks list if we moved to a different chunk
    if (centerChunkX == lastCenterChunkX && centerChunkY == lastCenterChunkY) {
        return;
    }
    lastCenterChunkX = centerChunkX;
    lastCenterChunkY = centerChunkY;

    chunksToRender.clear();

    // Limites pour le mode FINI (Par exemple un carré de 6x6 chunks autour de 0,0)
    int limitMin = -3;
    int limitMax = 3;

    // Générer et collecter les chunks visibles
        for (int y = -renderDistance + centerChunkY; y <= renderDistance + centerChunkY; y++) {
            for (int x = -renderDistance + centerChunkX; x <= renderDistance + centerChunkX; x++) {
                // Si mode FINI, on ne génère pas au delà des limites
                if (currentMode == MapMode::FINITE) {
                    if (x < limitMin || x > limitMax || y < limitMin || y > limitMax) continue;
                }
                ChunkCoord chunkCoord(x, y);
                // Créer le chunk s'il n'existe pas
                if (chunks.find(chunkCoord) == chunks.end()) {
                    chunks.emplace(chunkCoord, Chunk(x, y, tileWidth, tileHeight, *this));
                    //std::cout << "Chunk créé: (" << x << ", " << y << ")" << std::endl;
                }
                
                chunksToRender.push_back(&chunks.at(chunkCoord));
            }
        }
        // === DÉBUT DE LA MODIFICATION ===
        
        // ON TRIE LES CHUNKS AVANT DE LES DESSINER
        // C'est l'algorithme du peintre pour les chunks
        std::sort(chunksToRender.begin(), chunksToRender.end(), [](const Chunk* a, const Chunk* b) {
            // On compare la "profondeur" des chunks.
            // Les chunks avec un (X+Y) plus petit sont plus "loin" 
            // et doivent être dessinés en premier.
            return (a->chunkX + a->chunkY) < (b->chunkX + b->chunkY);
        });
        // === FIN DE LA MODIFICATION ===

        if (currentMode == MapMode::INFINITE) {
        // === DÉBUT DU DÉCHARGEMENT DES CHUNKS ===
        
        // On garde une "zone tampon" de 2 chunks
        // pour éviter de recharger/décharger sans arrêt
        const int unloadDistance = renderDistance + 2; 
        
        // Boucle spéciale pour supprimer en toute sécurité
        // "it" est notre "curseur" manuel dans la map
        for (auto it = chunks.begin(); it != chunks.end(); /* pas d'incrément ici */) {
            
            const ChunkCoord& coord = it->first; // Coordonnées du chunk
            
            // On calcule la distance (la plus grande, en X ou Y)
            int dist_x = std::abs(coord.x - centerChunkX);
            int dist_y = std::abs(coord.y - centerChunkY);
            
            // On utilise la distance "Chebyshev" (distance du Roi aux échecs)
            // C'est la bonne distance pour un carré
            int distance = std::max(dist_x, dist_y);
            
            // Si le chunk est trop loin...
            if (distance > unloadDistance) {
                // On le supprime de la map
                //std::cout << "Chunk déchargé: (" << coord.x << ", " << coord.y << ")" << std::endl;
                
                // chunks.erase(it) supprime l'élément ET renvoie un itérateur
                // sur l'élément SUIVANT. C'est la clé de la sécurité.
                it = chunks.erase(it);
            } 
            else {
                // Le chunk est assez proche, on passe au suivant.
                // C'est seulement ici qu'on incrémente manuellement.
                ++it;
            }
        }
        // === FIN DU DÉCHARGEMENT DES CHUNKS ===
    }
}

// VERSION OPTIMISÉE POUR PERLIN-PLAY (Ton ancien code rapide)
void Map::renderFast(sf::RenderWindow& window, sf::Texture& texture) const {
    sf::RenderStates states;
    states.texture = &texture;
    states.transform.translate(chunkOrigin);
    
    for (const auto* chunk : chunksToRender) {
        chunk->drawFast(window, states);
    }
}

void Map::render(sf::RenderWindow& window, sf::Texture& texture, const std::vector<Entity*>& entities, const Entity* player) const {
    // Dessiner tous les chunks visibles
    sf::RenderStates states;
    states.texture = &texture;
    states.transform.translate(chunkOrigin);
    
    if (chunksToRender.empty()) return;

    // OPTIMIZATION: Flatten the render loop
    // Instead of looping depths -> chunks, we collect all layers and sort them.
    struct RenderItem {
        int globalDepth;
        int localDepth;
        const Chunk* chunk;
    };

    // Reserve memory to avoid allocations (estimate: chunks * avg_layers_per_chunk)
    static std::vector<RenderItem> renderQueue; // Static to reuse memory
    renderQueue.clear();
    renderQueue.reserve(chunksToRender.size() * 16);

    for (const auto* chunk : chunksToRender) {
        // Iterate over the chunk's existing buffers directly
        for (const auto& [localDepth, buffer] : chunk->layerBuffers) {
            int globalDepth = (chunk->chunkX * Chunk::SIZE) + (chunk->chunkY * Chunk::SIZE) + localDepth;
            renderQueue.push_back({globalDepth, localDepth, chunk});
        }
    }

    // Sort by global depth (Painter's Algorithm)
    std::sort(renderQueue.begin(), renderQueue.end(), [](const RenderItem& a, const RenderItem& b) {
        return a.globalDepth < b.globalDepth;
    });


    // 2. Préparer et trier les entités
    std::vector<Entity*> sortedEntities = entities;
    std::sort(sortedEntities.begin(), sortedEntities.end(), [](Entity* a, Entity* b) {
        return a->getDepth() < b->getDepth();
    });

    // 3. Boucle de rendu mixte (Terrain + Entités)
    auto entityIt = sortedEntities.begin();
    if(player == nullptr){
    for (const auto& item : renderQueue) {

         // A. Dessiner les entités qui sont DERRIÈRE ou AU MÊME NIVEAU que cette couche de terrain
        // Note : On compare getDepth() (gridX + gridY) avec globalDepth.
        while (entityIt != sortedEntities.end() && (*entityIt)->getDepth() < item.globalDepth) {
            (*entityIt)->draw(window, *this);
            ++entityIt;
        }

        // Draw the layer
        // We pass localDepth directly to avoid map lookups inside drawLayer
        item.chunk->drawLayer(window, states, item.localDepth, item.globalDepth);
    }
    }else{
        // Draw sorted layers
        int playerDepth = player->getDepth();
        bool playerDrawn = false;
        for (const auto& item : renderQueue) {
            while (entityIt != sortedEntities.end() && (*entityIt)->getDepth() < item.globalDepth) {
            (*entityIt)->draw(window, *this);
            ++entityIt;
        }
        // Draw player if we reached their depth
        if (!playerDrawn && item.globalDepth > playerDepth) {
            player->draw(window, *this);
            playerDrawn = true;
        }

        // Draw the layer
        // We pass localDepth directly to avoid map lookups inside drawLayer
        item.chunk->drawLayer(window, states, item.localDepth, item.globalDepth, playerDepth, player->shape.getPosition());
        }
        if (!playerDrawn)
        {
            player->draw(window, *this);
        }
    }

    // 4. Dessiner les entités restantes (celles qui sont devant tout le terrain)
    while (entityIt != sortedEntities.end()) {
        (*entityIt)->draw(window, *this);
        ++entityIt;
    }
}
// Vérifie si une case contient un obstacle (Arbre ou Eau)
bool Map::isObstacle(int x, int y) const {
    int zLevel = getGroundLevel((float)x, (float)y);

    // 1. L'eau bloque (niveau <= 5)
    if (zLevel <= 5) return true;

    // 2. Les falaises bloquent (on simplifie : si on est trop bas par rapport à l'entité)
    // (Pour une collision parfaite, il faudrait passer le Z de l'entité, mais restons simple)

    // 3. Arbre ? (Même condition que dans Chunk)
    if (zLevel >= 7 && zLevel <= 10) { 
        if (randomHash(x, y) < 0.02f) { // 2% de chance, comme avant
            return true;
        }
    }

    return false;
}

bool Map::isWater(int x, int y) const {

    // L'eau est définie comme les niveaux <= 5
    return getGroundLevel((float)x, (float)y) <= 5;
}