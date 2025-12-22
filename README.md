# 🌱 Ecovillage

**Ecovillage** est un projet de simulation et de visualisation procédurale développé en **C++ avec SFML**, visant à poser les bases d’un village virtuel généré dynamiquement, cohérent et évolutif.

Le projet explore la génération de terrain isométrique, la gestion de chunks, l’utilisation du bruit de Perlin/FastNoise, et les fondations techniques nécessaires à un futur jeu ou simulateur de village durable.

---

## 🎯 Objectifs du projet

- Générer un **terrain isométrique procédural** cohérent
- Utiliser un **système de chunks** pour optimiser le rendu
- Explorer les notions de **caméra, profondeur et hauteur**
- Poser les bases d’un **village vivant** (sol utilisable, entités, végétation, eau, etc.)
- Apprendre et maîtriser une **architecture C++ propre et modulaire**

---

## 🧠 Fonctionnalités actuelles

- ✅ Fenêtre SFML (1280x720)
- ✅ Rendu isométrique (conversion iso ↔ screen)
- ✅ Génération procédurale avec Perlin Noise / FastNoise
- ✅ Système de chunks dynamiques
- ✅ Caméra mobile (WASD)
- ✅ Gestion de la hauteur (z) du terrain
- ✅ Rendu optimisé avec distance de rendu
- ✅ Génération d'arbres simples
- ✅ Entités mobiles avec déplacement fluide
- ✅ Sol solide et interactif

---

## 🚧 Fonctionnalités en cours / prévues

- 🌳 Génération de plus végétation (herbe, fleurs, etc...)
- 🌊 Animation de l’eau (vagues)
- 🏘️ Bâtiments et structures du village
- 🖥️ Interface graphique (TGUI)
- 💾 Sauvegarde / chargement du monde

---

## 🛠️ Technologies utilisées

- **Langage** : C++
- **Graphismes** : SFML
- **Génération procédurale** : Perlin Noise / FastNoise
- **Architecture** : Chunks, Camera, MapManager
- **Outils** : CMake / vcpkg (selon configuration)

---

## 📁 Structure du projet (simplifiée)
```bash
Ecovillage/
│
├── assets/
│ ├── sounds/
│ │ ├── music.mp3
│ └── sprites/
│   ├── Bear1.png
│   ├── DeerAtlas.png
│   └── block.png
│
├── src/
│ ├── main.cpp
│ └── core/
│   ├── utilitaires.cpp / .hpp
│   ├── camera.hpp
│   ├── chunk.cpp / .hpp
│   ├── map.cpp / .hpp
│   ├── entity.cpp / .hpp
│   ├── spawner.cpp / .hpp
│   ├── game.cpp / .hpp
│   └── FastNoiseLite.h
│
│
└── README.md
```

---

## ▶️ Compilation et exécution

### Prérequis
- Compilateur C++17 ou plus
- Vcpkg
- CMake 
- SFML installé (3.x.x)
- nlohmann-json
- Tgui


### Exemple (CMake)
```bash
mkdir build
cd build
cmake ..
make
./Ecovillage
```
(Les commandes peuvent varier selon l’OS)

## 📸 Aperçu

(Ajoute ici des captures d’écran ou un GIF de la génération du terrain)

## 👥 Équipe

Imane – Conception, présentation, vision du projet

Charis – Développement C++ / Architecture / Génération procédurale


## 📚 Contexte

Projet réalisé dans un cadre académique, avec une volonté claire d’aller au-delà des attentes et de construire une base sérieuse pour un projet plus ambitieux à long terme.

## 📄 Licence

Ce projet est à but éducatif.
MIT.

## 🌍 Vision

Ecovillage n’est pas seulement un terrain généré.
C’est un espace où la technique sert une idée :
celle d’un monde cohérent, durable et vivant.
