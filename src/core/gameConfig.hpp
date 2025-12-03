#ifndef GAME_CONFIG_HPP
#define GAME_CONFIG_HPP

#include "nlohmann/json.hpp" 
#include <fstream>
#include <iostream>

using json = nlohmann::json;

// Paramètres pour la génération du monde
struct WorldConfig {
    int seed = 1337;
    float frequency = 0.008f;
    int octaves = 6;
    float persistence = 0.5f; // Gain
    float lacunarity = 2.0f;
    float contrast = 3.5f;    // Puissance (pow)
    float heightScale = 80.0f; 
};

struct GameConfig {
    WorldConfig world;
    int renderDistance = 3;
    
    // Sauvegarder
    void save(const std::string& filename = "config.json") {
        json j;
        j["world"] = {
            {"seed", world.seed},
            {"frequency", world.frequency},
            {"octaves", world.octaves},
            {"persistence", world.persistence},
            {"lacunarity", world.lacunarity},
            {"contrast", world.contrast},
            {"heightScale", world.heightScale}
        };
        j["renderDistance"] = renderDistance;

        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }

    // Charger
    void load(const std::string& filename = "config.json") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Config introuvable, création par défaut." << std::endl;
            save(filename);
            return;
        }
        try {
            json j;
            file >> j;
            if(j.contains("world")) {
                auto& w = j["world"];
                world.seed = w.value("seed", 1337);
                world.frequency = w.value("frequency", 0.008f);
                world.octaves = w.value("octaves", 6);
                world.persistence = w.value("persistence", 0.5f);
                world.lacunarity = w.value("lacunarity", 2.0f);
                world.contrast = w.value("contrast", 3.5f);
                world.heightScale = w.value("heightScale", 80.0f);
            }
            renderDistance = j.value("renderDistance", 4);
        } catch (const std::exception& e) {
            std::cerr << "Erreur JSON: " << e.what() << std::endl;
        }
    }
};

#endif