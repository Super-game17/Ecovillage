#include "map.hpp"

void Map::update (const sf::Vector2f& cameraPos, const sf::Texture& texture){

    sf::Vector2i tileCenter = isoToCartesian(cameraPos.x, cameraPos.y,tileWidth, tileHeight);
    int centerChunkX = static_cast<int>(std::floor(static_cast<float>(tileCenter.x) / Chunk::SIZE));
    int centerChunkY = static_cast<int>(std::floor(static_cast<float>(tileCenter.y) / Chunk::SIZE));
    chunksToRender.clear();

    // Générer et collecter les chunks visibles
        for (int y = -renderDistance + centerChunkY; y <= renderDistance + centerChunkY; y++) {
            for (int x = -renderDistance + centerChunkX; x <= renderDistance + centerChunkX; x++) {
                ChunkCoord chunkCoord(x, y);
                
                // Créer le chunk s'il n'existe pas
                if (chunks.find(chunkCoord) == chunks.end()) {
                    chunks.emplace(chunkCoord, Chunk(x, y, tileWidth, tileHeight, texture));
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

void Map::render(sf::RenderWindow& window, sf::Texture& texture) const {
    // Dessiner tous les chunks visibles
        sf::RenderStates states;
        states.texture = &texture;
        states.transform.translate(chunkOrigin);
        
        for (const auto* chunk : chunksToRender) {
            chunk->draw(window, states);
        }
}