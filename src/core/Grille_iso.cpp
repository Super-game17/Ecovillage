#include "Grille_iso.hpp"

void afficher_grille_iso(sf::RenderWindow& window,unsigned int width, unsigned int height) {
    // Paramètres de la grille
    const int mapW = 10;
    const int mapH = 10;
    const float tileW = 100.f; // largeur du losange (px)
    const float tileH = 50.f; // hauteur du losange (px) (souvent tileH = tileW / 2)

    // Origine écran (où la case (0,0) sera dessinée)
    const float originX = width / 2.f;
    const float originY = height / 4.f;

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
                float screenX = originX + (x - y) * (tileW / 2.f);
                float screenY = originY + (x + y) * (tileH / 2.f);

                diamond.setPosition({screenX, screenY});
                
                window.draw(diamond);

                // (optionnel) dessiner les coordonnées pour debug
                // sf::Text txt(...); window.draw(txt);
            }
        }

}