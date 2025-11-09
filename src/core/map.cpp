// Map.cpp (version modifiée)
// Remplace ton map.cpp par ce fichier (ou adapte les fonctions indiquées)

#include "map.hpp"
#include <cmath>
#include <iostream>

MapManager::MapManager(unsigned int windowWidth, unsigned int windowHeight, PerlinNoise& pn_ref)
    : windowW(windowWidth), windowH(windowHeight), pn(pn_ref) {}

// isoToScreen : retourne la position **monde** (world coords) correspondant à la tile (x,y,z)
// On garde la même origine monde que ton ancien code : offsetX = windowW/2, offsetY = windowH/4
sf::Vector2f MapManager::isoToScreen(int x, int y, int z_height) {
    float originX = windowW / 2.f;
    float originY = windowH / 4.f;

    float worldX = originX + (x - y) * (TILE_WIDTH / 2.f);
    float worldY = originY + (x + y) * (TILE_HEIGHT / 2.f) - z_height * TILE_HEIGHT;

    return sf::Vector2f(worldX, worldY);
}

// screenToIso : convertit une position **monde** (world coords) en coordonnées de tile (x,y).
// Ici screenPos est une position en world coords ; c'est volontaire parce que nous passons cameraCenter (view center en world coords).
sf::Vector2i MapManager::screenToIso(sf::Vector2f worldPos) {
    // Même origine que isoToScreen
    float offsetX = windowW / 2.f;
    float offsetY = windowH / 4.f;

    // Ajuste la position monde par rapport à l'origine monde
    float adjustedX = worldPos.x - offsetX;
    float adjustedY = worldPos.y - offsetY;

    // Inverse des formules:
    // adjustedX = (x - y) * TILE_WIDTH/2
    // adjustedY = (x + y) * TILE_HEIGHT/2

    float eq1 = adjustedX / (TILE_WIDTH / 2.f); // x - y
    float eq2 = adjustedY / (TILE_HEIGHT / 2.f); // x + y

    float fx = (eq1 + eq2) / 2.f;
    float fy = (eq2 - eq1) / 2.f;

    // On renvoie la tuile la plus proche ; on utilise round pour réduire les effets de bord.
    int ix = static_cast<int>(std::floor(fx + 0.00001f));
    int iy = static_cast<int>(std::floor(fy + 0.00001f));

    return sf::Vector2i(ix, iy);
}


// Génération d'un chunk (inchangé - j'ai gardé ta logique)
void MapManager::generateChunk(int chunkX, int chunkY) {
    std::unique_ptr<Chunk> newChunk = std::make_unique<Chunk>();

    for (int y = 0; y < Chunk::SIZE; ++y) {
        for (int x = 0; x < Chunk::SIZE; ++x) {
            int worldX = chunkX * Chunk::SIZE + x;
            int worldY = chunkY * Chunk::SIZE + y;

            double noiseVal = pn.noise(worldX * FREQUENCY, worldY * FREQUENCY, 0.0);
            int z = static_cast<int>(std::floor(noiseVal * HEIGHT_MULTIPLIER));

            newChunk->blocks[x][y].height = z;
            if (z < 0) newChunk->blocks[x][y].color = sf::Color(0, 50, 150);
            else if (z < 3) newChunk->blocks[x][y].color = sf::Color(50, 200, 50);
            else newChunk->blocks[x][y].color = sf::Color(150, 150, 150);
        }
    }
    newChunk->generated = true;
    chunks[{chunkX, chunkY}] = std::move(newChunk);
}


// Met à jour les chunks autour de la caméra
void MapManager::update(sf::Vector2f cameraCenter) {
    // 1) obtenir la position de grille sous le centre de la caméra (cameraCenter est en world coords — c'est bon)
    sf::Vector2i playerGridPos = screenToIso(cameraCenter);

    // 2) chunk index — important a: utiliser une seule floor qui gère bien les négatifs
    float fx = static_cast<float>(playerGridPos.x) / static_cast<float>(Chunk::SIZE);
    float fy = static_cast<float>(playerGridPos.y) / static_cast<float>(Chunk::SIZE);

    int centerChunkX = static_cast<int>(std::floor(fx));
    int centerChunkY = static_cast<int>(std::floor(fy));

    // Debug utile : décommente pour voir les valeurs pendant le test
    std::cerr << "playerGridPos: ("<<playerGridPos.x<<","<<playerGridPos.y<<") centerChunk: ("<<centerChunkX<<","<<centerChunkY<<")\n";

    // 3) génération autour du chunk central
    for (int cy = centerChunkY - RENDER_DISTANCE; cy <= centerChunkY + RENDER_DISTANCE; ++cy) {
        for (int cx = centerChunkX - RENDER_DISTANCE; cx <= centerChunkX + RENDER_DISTANCE; ++cx) {
            std::pair<int, int> chunkKey = {cx, cy};

            if (chunks.find(chunkKey) == chunks.end()) {
                generateChunk(cx, cy);
            }
        }
    }

    // décharger les chunks trop loin
    std::vector<std::pair<int, int>> chunksToUnload;
    for (const auto& pair : chunks) {
        int cx = pair.first.first;
        int cy = pair.first.second;

        if (std::abs(cx - centerChunkX) > RENDER_DISTANCE + 1 ||
            std::abs(cy - centerChunkY) > RENDER_DISTANCE + 1) {
            chunksToUnload.push_back(pair.first);
        }
    }
    for (const auto& key : chunksToUnload) {
        chunks.erase(key);
    }
}


// Dessine les chunks — IMPORTANT : blockSprite est dessiné en *coordonnées monde*,
// et comme tu fais window.setView(view) avant d'appeler mapManager.draw, SFML gère la camera.
// Donc ici on doit fournir des positions monde cohérentes (isoToScreen), et ne pas re-transformer par la view.
void MapManager::draw(sf::RenderWindow& window, sf::Sprite& blockSprite) {
    for (const auto& pair : chunks) {
        int chunkX = pair.first.first;
        int chunkY = pair.first.second;
        const auto& chunk = pair.second;

        if (!chunk->generated) continue;

        for (int y = 0; y < Chunk::SIZE; ++y) {
            for (int x = 0; x < Chunk::SIZE; ++x) {
                int worldX = chunkX * Chunk::SIZE + x;
                int worldY = chunkY * Chunk::SIZE + y;
                int z = chunk->blocks[x][y].height;
                sf::Color color = chunk->blocks[x][y].color;

                // isoToScreen renvoie une position monde cohérente en utilisant la même origine,
                // donc elle se dessinera correctement quand la view est active.
                sf::Vector2f screenPos = isoToScreen(worldX, worldY, z);

                if (screenPos.x > -TILE_WIDTH && screenPos.x < windowW + TILE_WIDTH &&
                    screenPos.y > -TILE_HEIGHT * 2 && screenPos.y < windowH + TILE_HEIGHT * 2) {

                    blockSprite.setPosition(screenPos);
                    blockSprite.setColor(color);
                    window.draw(blockSprite);
                }
            }
        }
    }
}
