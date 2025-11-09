#include<iostream>
#include<vector>
#include<map>
#include<cmath>
#include<SFML/Graphics.hpp>
#include<algorithm>
#include"Grille_iso.hpp"
#include"camera.hpp"
#include "FastNoiseLite.h"

// Structure pour les coordonnées de chunk avec opérateur de comparaison
struct ChunkCoord {
    int x, y;
    
    ChunkCoord(int x_, int y_) : x(x_), y(y_) {}
    
    bool operator<(const ChunkCoord& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};


// Structure de Chunk
struct Chunk {
    static constexpr int SIZE = 16;
    std::vector<sf::Vertex> vertices;
    sf::VertexBuffer vertexBuffer;
    int chunkX, chunkY;
    sf::Vector2f position;

    Chunk() = default;

    Chunk(int cx, int cy, int tileW, int tileH, const sf::Texture& texture) 
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


        vertices.reserve(SIZE * SIZE * 6); // 6 vertices par tile (2 triangles)
    
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

                
                // Position isométrique
                float isoX = (worldX - worldY) * halfTileW;
                // Position isométrique Y (MODIFIÉE)
                // 1. Position de base (z=0)
                float baseIsoY = (worldX + worldY) * tileH /2.f;
                // 2. Décalage vers le HAUT (soustraire) basé sur la hauteur Z
                float zOffset = zLevel * heightPerZLevel;
                float isoY = baseIsoY - zOffset;
                
                // --- NOUVEAU : Couleur basée sur la hauteur ---
                sf::Color color;
if (zLevel <= 3) {
    color = sf::Color(30, 80, 180); // Eau profonde
} else if (zLevel <= 5) {
    color = sf::Color(70, 130, 220); // Eau peu profonde
} else if (zLevel <= 6) {
    color = sf::Color(194, 178, 128); // Plage
} else if (zLevel <= 10) {
    color = sf::Color(34, 139, 34); // Herbe/plaines
} else if (zLevel <= 12) {
    color = sf::Color(60, 100, 60); // Collines
} else if (zLevel <= 14) {
    color = sf::Color(100, 100, 100); // Roche
} else {
    color = sf::Color(250, 250, 250); // Neige
}
                
                // Coordonnées de texture (texture complète)
                sf::Vector2f texCoordStart(0.0f, 0.0f);
                sf::Vector2f texCoordEnd(texSize.x, texSize.y);
                
                // Position du tile
                sf::Vector2f tilePos(isoX, isoY);
                
                // Créer les 6 vertices pour 2 triangles
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
                
                for (int i = 0; i < 6; i++) {
                    vertices.push_back(v[i]);
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
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(vertexBuffer, states);
    }
};

int main(){
    unsigned int width = 1280;
    unsigned int height = 720;
    const int tileWidth = 100;
    const int tileHeight = 50;
    
    Camera camera(static_cast<float>(width), static_cast<float>(height), sf::Vector2f(width/2.f, height/2.f));
    sf::RenderWindow window(sf::VideoMode({width, height}), "Chunk System");
    
    sf::Texture texture;
    if(!texture.loadFromFile("assets/sprites/block.png")){
        std::cerr << "Erreur lors du chargement de la texture" << std::endl;
        return -1;
    }
    texture.setSmooth(true);
    
    // Map pour stocker les chunks
    std::map<ChunkCoord, Chunk> chunks;
    
    // Vecteur des chunks à rendre
    std::vector<const Chunk*> chunksToRender;
    
    // Origine des chunks
    sf::Vector2f chunkOrigin(width / 2.0f, height / 2.0f);
    
    // Distance de rendu
    int renderDistance = 3;
    
    // Fonction pour convertir coordonnées isométriques en cartésiennes
   // tileW et tileH doivent être capturés / passés
    auto isometricToCartesian = [tileWidth, tileHeight](float worldX, float worldY) -> sf::Vector2i {
    // worldX/worldY doivent être des coords MONDE (après retrait de chunkOrigin)
    float halfW = tileWidth / 2.0f;
    float halfH = tileHeight / 2.0f;

    float eq1 = worldX / halfW; // x - y
    float eq2 = worldY / halfH; // x + y

    float fx = (eq1 + eq2) / 2.0f;
    float fy = (eq2 - eq1) / 2.0f;

    int ix = static_cast<int>(std::floor(fx + 0.00001f));
    int iy = static_cast<int>(std::floor(fy + 0.00001f));
    return sf::Vector2i(ix, iy);
};

    while (window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()){
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape){
                    window.close();
                }
                // Augmenter/diminuer la distance de rendu
                else if(keyPressed->scancode == sf::Keyboard::Scancode::Up){
                    renderDistance++;
                    std::cout << "Render distance: " << renderDistance << std::endl;
                }
                else if(keyPressed->scancode == sf::Keyboard::Scancode::Down && renderDistance > 1){
                    renderDistance--;
                    std::cout << "Render distance: " << renderDistance << std::endl;
                }
            }
            camera.handleEvent(*event);
        }

        camera.handleInput(window);
        camera.update(window);
        
        // Obtenir la position de la caméra
        sf::View view = window.getView();
        sf::Vector2f cameraPos = view.getCenter() - chunkOrigin;
        
        // Convertir en coordonnées de chunk
        float chunkSize = Chunk::SIZE * tileWidth;
        // cameraPos : world coords RELATIVES à l'origine des chunks (après - chunkOrigin)
    sf::Vector2i tileCenter = isometricToCartesian(cameraPos.x, cameraPos.y);
    int centerChunkX = static_cast<int>(std::floor(static_cast<float>(tileCenter.x) / Chunk::SIZE));
    int centerChunkY = static_cast<int>(std::floor(static_cast<float>(tileCenter.y) / Chunk::SIZE));

        
        // Effacer la liste des chunks à rendre
        chunksToRender.clear();
        
        // Générer et collecter les chunks visibles
        for (int y = -renderDistance + centerChunkY; y <= renderDistance + centerChunkY; y++) {
            for (int x = -renderDistance + centerChunkX; x <= renderDistance + centerChunkX; x++) {
                ChunkCoord chunkCoord(x, y);
                
                // Créer le chunk s'il n'existe pas
                if (chunks.find(chunkCoord) == chunks.end()) {
                    chunks.emplace(chunkCoord, Chunk(x, y, tileWidth, tileHeight, texture));
                    std::cout << "Chunk créé: (" << x << ", " << y << ")" << std::endl;
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
                std::cout << "Chunk déchargé: (" << coord.x << ", " << coord.y << ")" << std::endl;
                
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
        window.clear(sf::Color::Black);
        
        // Dessiner tous les chunks visibles
        sf::RenderStates states;
        states.texture = &texture;
        states.transform.translate(chunkOrigin);
        
        for (const auto* chunk : chunksToRender) {
            chunk->draw(window, states);
        }

        window.display();
    }
    
    return 0;
}
