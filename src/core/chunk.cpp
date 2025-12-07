#include "chunk.hpp"
#include "map.hpp"
#include "entity.hpp"


void Chunk::addBlock(float worldX, float worldY, int zLevel, const sf::Color& color) {
    // Dimensions (récupérées de utilitaires.hpp)
    float halfTileW = tileWidth / 2.0f; 
    float tileHeightFull = tileHeight * 2.0f; 

    // Position isométrique
    auto [isoX, isoY] = isoToScreen(worldX, worldY, zLevel, tileWidth, tileHeight, swidth, sheight);

    sf::Vector2f tilePos(isoX, isoY);

    // --- CALCUL DE LA CLÉ DE PROFONDEUR ---
    // On doit savoir dans quelle "tranche" locale ce bloc se trouve.
    // worldX et worldY sont les coordonnées globales.
    // On récupère les coordonnées locales dans le chunk.
    int localX = static_cast<int>(worldX) - (chunkX * SIZE);
    int localY = static_cast<int>(worldY) - (chunkY * SIZE);

    int depthIndex = localX + localY; // C'est notre clé de tri !

    // Coordonnées de texture (Pour l'instant tout la texture, plus tard l'atlas)
    sf::Vector2f texCoordStart(0.0f, 0.0f);
    sf::Vector2f texCoordEnd(100.0f, 100.0f);

    // Création des vertices (6 pour 2 triangles)
    sf::Vertex v[6] = {
        // Triangle 1
        { sf::Vector2f(-halfTileW, 0.0f) + tilePos, color, texCoordStart },
        { sf::Vector2f(halfTileW, 0.0f) + tilePos, color, sf::Vector2f(texCoordEnd.x, texCoordStart.y) },
        { sf::Vector2f(halfTileW, tileHeightFull) + tilePos, color, texCoordEnd },
        
        // Triangle 2
        { sf::Vector2f(-halfTileW, 0.0f) + tilePos, color, texCoordStart },
        { sf::Vector2f(-halfTileW, tileHeightFull) + tilePos, color, sf::Vector2f(texCoordStart.x, texCoordEnd.y) },
        { sf::Vector2f(halfTileW, tileHeightFull) + tilePos, color, texCoordEnd }
    };

    for (int i = 0; i < 6; i++) vertices.push_back(v[i]);

    // Au lieu de vertices.push_back(...), on ajoute à la slice correspondante pour la profondeur
    for (int i = 0; i < 6; i++) {
        slices[depthIndex].push_back(v[i]);
    }
}

void Chunk::generateTree(int rootX, int rootY, int zGroundLevel) {
    // 1. Paramètres de base
    sf::Color trunkColor(101, 67, 33);   // Marron
    sf::Color leavesColor(0, 100, 0);  // Vert foncé

    // Utilisation du hash de l'utilitaire pour déterminer le type d'arbre (déterministe)
    float rType = randomHash(rootX + 1000, rootY + 1000); // Décalage pour varier les résultats
    
    bool bigTree = (rType < 0.1f); // 10% de chance pour un grand arbre

    if (bigTree) {
        //std::cout << "Grand arbre à (" << rootX << ", " << rootY << ")" << "Random hash = " << rType << std::endl;
        trunkColor = sf::Color(211, 211, 211); // Tronc plus clair pour grand arbre
        leavesColor = sf::Color(0, 100, 0); // Vert plus vif pour grand arbre
    }
    
    int height = bigTree ? 6 : 4;  // Hauteur du tronc
    int radius = bigTree ? 2 : 1;  // Rayon du feuillage

    // 2. Construction du Tronc
    for (int h = 0; h < height; h++) {
        // On empile les blocs en augmentant Z
        addBlock(rootX, rootY, zGroundLevel + h + 1, trunkColor);
    }

    // 3. Construction des Feuilles (Sphère/Ellipsoïde)
    // On boucle autour du sommet de l'arbre
    int leafCenterZ = zGroundLevel + height;

    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            for (int z = -radius; z <= radius; z++) {
                
                // Formule de la sphère: x² + y² + z² <= r²
                // On ajoute un peu de randomness ou de tolérance pour ne pas faire un cube parfait
                if (x*x + y*y + z*z <= radius*radius + 1) {
                    
                    // Ne pas écraser le tronc (optionnel, mais plus propre)
                    if (x == 0 && y == 0 && z < 0) continue;

                    addBlock(rootX + x, rootY + y, leafCenterZ + z, leavesColor);
                }
            }
        }
    }
}

Chunk::Chunk(int cx, int cy, int tileW, int tileH, Map& carte)
        : chunkX(cx), chunkY(cy) {

       // vertices.reserve(SIZE * SIZE * 6 * 2); // 6 vertices par tile (2 triangles) * 2 (pour les arbres potentiels)
    
       // Génération des blocs du chunk, coordonnées globales
        int baseX = cx * SIZE;
        int baseY = cy * SIZE;
        
        // Parcours de chaque position dans le chunk
        for (int y = 0; y < SIZE; ++y) {
            for (int x = 0; x < SIZE; ++x) {
                // Coordonnées mondiales pour les blocs
                int worldX = baseX + x;
                int worldY = baseY + y;
                
                int zLevel = carte.getGroundLevel(static_cast<float>(worldX), static_cast<float>(worldY));
                // Couleur basée sur la hauteur
                sf::Color color;
                bool canHaveTree = false;

                if (zLevel <= 3) {
                    color = sf::Color(30, 80, 180); // Eau profonde
                } else if (zLevel <= 5) {
                    color = sf::Color(70, 130, 220); // Eau peu profonde
                } else if (zLevel <= 6) {
                    color = sf::Color(194, 178, 128); // Plage
                } else if (zLevel <= 10) {
                    color = sf::Color(34, 139, 34); // Herbe/plaines
                    canHaveTree = true;
                } else if (zLevel <= 12) {
                    color = sf::Color(60, 100, 60); // Collines
                } else if (zLevel <= 14) {
                    color = sf::Color(100, 100, 100); // Roche
                } else {
                    color = sf::Color(250, 250, 250); // Neige
                } 
                addBlock(worldX, worldY, zLevel, color);  

                if (canHaveTree) {
                    // Décider de planter un arbre (2% de chance)
                    float treeChance = randomHash(worldX , worldY);
                    if (treeChance < 0.02f) {
                        generateTree(worldX, worldY, zLevel);
                    }
                }
            }
        }
        
        // Construire les buffers GPU une seule fois après avoir généré tous les vertices
        buildBuffers();
        buildFastBuffer();
    }
    
void Chunk::buildBuffers() {
    // Transférer les slices vers GPU
    for (auto& [depth, vertices] : slices) {
        if (vertices.empty()) continue;
        
        // Création et remplissage du VertexBuffer
        (void)layerBuffers[depth].create(vertices.size());
        // Définir le type de primitive et l'usage
        layerBuffers[depth].setPrimitiveType(sf::PrimitiveType::Triangles);
        layerBuffers[depth].setUsage(sf::VertexBuffer::Usage::Static);
        // Mettre à jour les données
        (void)layerBuffers[depth].update(vertices.data());
    }
}

void::Chunk::buildFastBuffer() {
    // Création et remplissage du VertexBuffer
    (void)vertexBuffer.create(vertices.size());
    // Définir le type de primitive et l'usage
    vertexBuffer.setPrimitiveType(sf::PrimitiveType::Triangles);
    vertexBuffer.setUsage(sf::VertexBuffer::Usage::Static);
    // Mettre à jour les données
    (void)vertexBuffer.update(vertices.data());
}

void::Chunk::drawFast(sf::RenderTarget& target, sf::RenderStates states) const {
    // Dessiner directement depuis le buffer GPU
    target.draw(vertexBuffer, states);
}

void Chunk::drawLayer(sf::RenderTarget& target, sf::RenderStates states, int localDepth, int globalDepth,const Entity* player) const {
    
    // Récupérer le buffer GPU pour cette couche
    auto it = layerBuffers.find(localDepth);
    // Si pas de buffer pour cette couche, rien à dessiner
    if (it == layerBuffers.end()) return;
    
    if (player == nullptr) {
        // Pas de joueur spécifié, dessiner normalement
        target.draw(it->second, states);
        return;
    }
    // Simple verification : on fait le check seulement pour les blocs capables de cacher le joueur
    bool checkTransparency = (globalDepth > player->getDepth()) && (globalDepth < player->getDepth() + 10);

    if (!checkTransparency) {
        // Dessiner directement depuis le buffer GPU
        target.draw(it->second, states);
        return;
    }
    // On doit dessiner avec transparence les blocs qui couvrent >70% du joueur
    auto sliceIt = slices.find(localDepth);
    
    //Calculs d'overlay intelligent on va modifier les couleurs des vertices avant de dessiner
    //mais cette operation est lourde pour donc on le fait uniquement si necessaire
    
    std::vector<sf::Vertex> modifiedVertices = sliceIt->second;
    sf::FloatRect playerBounds = player->shape.getGlobalBounds();
    
    bool modified = false;

    // Parcourir les vertices par blocs (6 vertices par bloc)
    for (size_t i = 0; i < modifiedVertices.size(); i += 6) {
        // Calculer les bounds du bloc à partir des positions des vertices
        float minX = std::min({modifiedVertices[i].position.x, modifiedVertices[i+1].position.x, 
                                modifiedVertices[i+2].position.x, modifiedVertices[i+3].position.x,
                                modifiedVertices[i+4].position.x, modifiedVertices[i+5].position.x});
        float maxX = std::max({modifiedVertices[i].position.x, modifiedVertices[i+1].position.x, 
                                modifiedVertices[i+2].position.x, modifiedVertices[i+3].position.x,
                                modifiedVertices[i+4].position.x, modifiedVertices[i+5].position.x});
        float minY = std::min({modifiedVertices[i].position.y, modifiedVertices[i+1].position.y, 
                                modifiedVertices[i+2].position.y, modifiedVertices[i+3].position.y,
                                modifiedVertices[i+4].position.y, modifiedVertices[i+5].position.y});
        float maxY = std::max({modifiedVertices[i].position.y, modifiedVertices[i+1].position.y, 
                                modifiedVertices[i+2].position.y, modifiedVertices[i+3].position.y,
                                modifiedVertices[i+4].position.y, modifiedVertices[i+5].position.y});
        // Créer le rectangle du bloc
        sf::FloatRect blockBounds({minX, minY}, {maxX - minX, maxY - minY});
        
        // Calculer l'overlap avec le joueur
        float overlap = calculateOverlap(blockBounds, playerBounds);
        
        // Appliquer transparence si overlap > 70%
        if (overlap > 0.7f) {
            modified = true;
            for (size_t j = i; j < i + 6; ++j) {
                modifiedVertices[j].color.a = 128; // 50% transparent, on modifie l'alpha
            }
        }
    }
    
    // Dessiner avec les vertices modifiés
    target.draw(modifiedVertices.data(), modifiedVertices.size(), sf::PrimitiveType::Triangles, states);
    }

sf::FloatRect Chunk::getBlockBounds(int worldX, int worldY, int zLevel) const {
    auto [screenX, screenY] = isoToScreen(worldX, worldY, zLevel, tileWidth, tileHeight, swidth, sheight);
    return sf::FloatRect({screenX - tileWidth/2.f, screenY - tileHeight}, {static_cast<float>(tileWidth), tileHeight * 2.f});
}

float Chunk::calculateOverlap(const sf::FloatRect& block, const sf::FloatRect& player) {
    auto intersection = block.findIntersection(player);
    if (!intersection.has_value()) return 0.0f;
    
    float intersectionArea = intersection->size.x * intersection->size.y;
    float playerArea = player.size.x * player.size.y;
    
    return intersectionArea / playerArea; // Retourne 0.0 à 1.0
}