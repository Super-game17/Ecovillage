#ifndef SRC_CORE_GRILLE_ISO_HPP
#define SRC_CORE_GRILLE_ISO_HPP

#include<SFML/Graphics.hpp>
void afficher_grille_iso(sf::RenderWindow& window,unsigned int width, unsigned int height);
float wave(float amplitude, float frequency, float phase, sf::Clock& clock, int x, int y);
sf::Vector2f isoToScreen(int x, int y, int z,int tileWidth, int tileHeight, unsigned int width, unsigned int height);
// Fonction pour convertir coordonnées isométriques en cartésiennes
sf::Vector2i isoToCartesian (int worldX, int worldY, int tileWidth, int tileHeight);
#endif // SRC_CORE_GRILLE_ISO_HPP