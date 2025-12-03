#ifndef UTILITAIRES_HPP
#define UTILITAIRES_HPP

#include<SFML/Graphics.hpp>

//Variables utilitaires
extern unsigned int swidth;
extern unsigned int sheight;
const int tileWidth = 100;
const int tileHeight = 50;



//Fonctions utilitaires
void afficher_grille_iso(sf::RenderWindow& window,unsigned int width, unsigned int height);
float wave(float amplitude, float frequency, float phase, sf::Clock& clock, int x, int y);
sf::Vector2f isoToScreen(int x, int y, int z,int tileWidth, int tileHeight, unsigned int width, unsigned int height);
// Fonction pour convertir coordonnées isométriques en cartésiennes
sf::Vector2i isoToCartesian (int worldX, int worldY, int tileWidth, int tileHeight);
//fonction pour generer un nombre aleatoire entre 0 et 1
float randomHash(int x, int y);
sf::FloatRect getPlayerBounds(sf::Vector2f playerPos);
#endif // UTILITAIRES_HPP