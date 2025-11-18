#include "chunk.hpp"


void Chunk::addTile(float worldX, float worldY, int zLevel, const sf::Color& color) {
    // Dimensions (récupérées de tes constantes ou membres)
    // Note: Assure-toi que tileW/tileH sont accessibles ici (membres de classe ou passés en arg)
    float halfTileW = tileWidth / 2.0f; 
    float tileHeightFull = tileHeight * 2.0f; 
    float heightPerZLevel = tileHeight; // Hauteur visuelle d'un bloc Z

    // Position isométrique
    float isoX = (worldX - worldY) * halfTileW;
    
    float baseIsoY = (worldX + worldY) * tileHeight / 2.f;
    float zOffset = zLevel * heightPerZLevel;
    float isoY = baseIsoY - zOffset;

    sf::Vector2f tilePos(isoX, isoY);

    // Coordonnées de texture (Pour l'instant tout la texture, plus tard l'atlas)
    // Je suppose que tu gardes texSize en membre ou tu le passes. 
    // Pour simplifier ici, je mets des valeurs par défaut, à adapter selon ton code
    sf::Vector2f texCoordStart(0.0f, 0.0f);
    sf::Vector2f texCoordEnd(100.0f, 100.0f); // Remplace par la taille réelle de ta texture

    // Création des vertices (copié de ton code original)
    sf::Vertex v[6] = {
        { sf::Vector2f(-halfTileW, 0.0f) + tilePos, color, texCoordStart },
        { sf::Vector2f(halfTileW, 0.0f) + tilePos, color, sf::Vector2f(texCoordEnd.x, texCoordStart.y) },
        { sf::Vector2f(halfTileW, tileHeightFull) + tilePos, color, texCoordEnd },
        
        { sf::Vector2f(-halfTileW, 0.0f) + tilePos, color, texCoordStart },
        { sf::Vector2f(-halfTileW, tileHeightFull) + tilePos, color, sf::Vector2f(texCoordStart.x, texCoordEnd.y) },
        { sf::Vector2f(halfTileW, tileHeightFull) + tilePos, color, texCoordEnd }
    };

    for (int i = 0; i < 6; i++) vertices.push_back(v[i]);
}
// Petit helper pour avoir un nombre aléatoire stable basé sur la position
// Retourne entre 0.0 et 1.0
float Chunk::randomHash(int x, int y) {
    // 1. On convertit les coordonnées en unsigned pour "casser" le signe
    // Le 'u' force le compilateur à traiter ces grands nombres comme non-signés
    unsigned int seed = (unsigned int)x * 15485863u + (unsigned int)y * 2038074743u; 
    
    // 2. On mélange les bits (XOR et décalages)
    seed = (seed ^ (seed >> 13)) * 1274126177u;
    seed = seed ^ (seed >> 16);

    // 3. On ramène le résultat entre 0.0 et 1.0
    // Le modulo 10000 donne un entier entre 0 et 9999
    return (float)(seed % 10000) / 10000.0f;
}

void Chunk::generateTree(int rootX, int rootY, int zGroundLevel) {
    // 1. Paramètres de base (couleurs fixes pour l'instant)
    sf::Color trunkColor(101, 67, 33);   // Marron
    sf::Color leavesColor(0, 100, 0);  // Vert foncé

    // Utilisation du hash pour déterminer le type d'arbre (déterministe)
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
        addTile(rootX, rootY, zGroundLevel + h + 1, trunkColor);
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

                    addTile(rootX + x, rootY + y, leafCenterZ + z, leavesColor);
                }
            }
        }
    }
}

Chunk::Chunk(int cx, int cy, int tileW, int tileH, const sf::Texture& texture)
        : chunkX(cx), chunkY(cy) {
            // 1. Initialiser le générateur de bruit
        //    (Fais-le une fois au début du constructeur)
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFrequency(0.008f); // TRÈS IMPORTANT: joue avec cette valeur !
                                   // Plus elle est petite, plus le terrain est "zoomé"

        // 2. Ajoute des octaves pour plus de détails (comme Minecraft)
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(6); // Comme dans ton exemple (6 octaves)
        noise.SetFractalLacunarity(2.0f);
        noise.SetFractalGain(0.45f); // Comme dans ton exemple

        // C'est la hauteur en pixels de "un" niveau de Z.
        // Puisque vos tuiles (X=0,Y=0) et (X=0,Y=1) sont décalées de 
        // halfTileH (25px), c'est la hauteur logique pour un cube.
        const float heightPerZLevel = tileH; // Ex: 25.0f


        vertices.reserve(SIZE * SIZE * 6 * 2); // 6 vertices par tile (2 triangles)
    
        int baseX = cx * SIZE;
        int baseY = cy * SIZE;
        
        float halfTileW = tileW / 2.0f;
        float tileHeightFull = tileH * 2.0f;
        
        sf::Vector2u texSize = texture.getSize();
        
        for (int y = 0; y < SIZE; ++y) {
            for (int x = 0; x < SIZE; ++x) {
                int worldX = baseX + x;
                int worldY = baseY + y;

                // 2. Obtenir la valeur du bruit pour cette coordonnée
                //    On utilise les coordonnées MONDE (worldX, worldY)
                //    GetNoise() renvoie une valeur entre -1.0 et 1.0
                float noiseValue = noise.GetNoise((float)worldX, (float)worldY);
                float noise01 = (noiseValue + 1.0f) * 0.5f; // Convertir [-1,1] en [0,1]

                // 4. Élévation exponentielle (comme dans ton exemple avec pow)
                noise01 = std::pow(noise01, 3.5f); // Augmente le contraste
                // Convertir le bruit en niveaux de hauteur entiers (ex: 0 à 10)
                // (noiseValue + 1.0) -> [0, 2]
                // * 5.0 -> [0, 10]
                // 5. Calculer la hauteur finale
                int zLevel = static_cast<int>(noise01 * 80.0f); // 0 à 80 blocs

                
                // --- NOUVEAU : Couleur basée sur la hauteur ---
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
                addTile(worldX, worldY, zLevel, color);  

                if (canHaveTree) {
                    // Décider de planter un arbre (2% de chance)
                    float treeChance = randomHash(worldX , worldY);
                    if (treeChance < 0.02f) {
                        generateTree(worldX, worldY, zLevel);
                    }
                }
            }
        }
        
    // Créer le VertexBuffer
    vertexBuffer.setUsage(sf::VertexBuffer::Usage::Static);
    // SFML primitive type enum: use the PrimitiveType::Triangles value
    vertexBuffer.setPrimitiveType(sf::PrimitiveType::Triangles);
    // Some SFML functions are [[nodiscard]]; explicitly ignore their return values
    (void)vertexBuffer.create(static_cast<std::size_t>(vertices.size()));
    (void)vertexBuffer.update(vertices.data());
    }
    
void Chunk::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(vertexBuffer, states);
    }