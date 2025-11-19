#include "utilitaires.hpp"

// Define the global variables
unsigned int swidth = 1280;
unsigned int sheight = 720;

void afficher_grille_iso(sf::RenderWindow& window,unsigned int width, unsigned int height) {
    // Paramètres de la grille
    const int mapW = 10;
    const int mapH = 10;
    const float tileW = 100.f; // largeur du losange (px)
    const float tileH = 50.f; // hauteur du losange (px) (souvent tileH = tileW / 2)

    // Préparer la forme losange (top tile)
    sf::ConvexShape diamond;
    diamond.setPointCount(4);
    // points relatifs (centre à 0,0) -> sera repositionné par setPosition()
    diamond.setPoint(0, { 0.f, -tileH/2.f });   // top
    diamond.setPoint(1, { tileW/2.f, 0.f });    // right
    diamond.setPoint(2, { 0.f, tileH/2.f });    // bottom
    diamond.setPoint(3, { -tileW/2.f, 0.f });   // left
    diamond.setFillColor(sf::Color(120, 200, 120));
    diamond.setOutlineThickness(1.f);
    diamond.setOutlineColor(sf::Color(40, 80, 40));
    for (int y = 0; y < mapH; ++y) {
            for (int x = 0; x < mapW; ++x) {
                // Formule de projection isometrique (top view)
                diamond.setPosition(isoToScreen(x, y, 0, tileW, tileH, width, height));
                
                window.draw(diamond);

                // (optionnel) dessiner les coordonnées pour debug
                // sf::Text txt(...); window.draw(txt);
            }
        }

}

float wave(float amplitude, float frequency, float phase, sf::Clock& clock, int x, int y){
    float t = clock.getElapsedTime().asSeconds();
    return amplitude * sinf(frequency * t + phase + ( x+y )* phase);
}

sf::Vector2f isoToScreen(int x, int y, int z, int tileWidth, int tileHeight, unsigned int width, unsigned int height) {
    float screenX = (width / 2.f) + (x - y) * (tileWidth / 2.f);
    // On retire la hauteur 'z' à la position y pour faire monter l'objet
    float screenY = (height / 4.f) + (x + y) * (tileHeight / 2.f) - z * tileHeight; 
    return sf::Vector2f(screenX, screenY);
}

sf::Vector2i isoToCartesian (int worldX, int worldY, int tileWidth, int tileHeight){
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
}
// Petit helper pour avoir un nombre aléatoire stable basé sur la position
// Retourne entre 0.0 et 1.0
float randomHash(int x, int y) {
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
