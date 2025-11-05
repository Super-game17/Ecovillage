#include<iostream>
#include<SFML/Graphics.hpp>
#include"Grille_iso.hpp"
#include"perlinnoise.hpp" // Include Perlin noise header
#include <vector>
#include <cmath>
#include <numeric>

// Assuming isoToScreen function and afficher_grille_iso function are defined in Grille_iso.hpp or linked properly

int main(){
    unsigned int width = 1280;
    unsigned int height = 720;
    const int cols = 30; // Increased grid size for better noise visualization
    const int rows = 30;
    const int tileWidth = 100;
    const int tileHeight = 50;
    
    // Perlin noise parameters
    const double frequency = 0.1; // Lower frequency makes larger "hills"
    const int heightMultiplier = 5; // How high the terrain can go

    // --- Navigation variables ---
    sf::Vector2f cameraPosition(width / 2.f, height / 2.f); // Initial camera center
    float cameraSpeed = 15.0f; 
    // ----------------------------

    int gridX = 0; 
    int gridY = 0;
    const int widthTiles = 1;
    const int depthTiles = 1;
    int z_offset = 0; // Renamed 'z' to 'z_offset' to avoid conflict

    sf::RenderWindow window (sf::VideoMode({width,height}), "Teeetet");
    // --- Setup the camera view ---
    sf::View cameraView(sf::FloatRect({0.f, 0.f},{(float)width, (float)height}));
    window.setView(cameraView);
    // -----------------------------
    sf::Texture texture;
    if(!texture.loadFromFile("assets/sprites/block.png")){
        std::cerr<<"Erreur lors du chargement de la texture"<<std::endl;
        return -1;
    }
     
    sf::Sprite block(texture);

    // Initialize Perlin Noise generator with a fixed seed for consistent terrain
    PerlinNoise pn(12345); 

    std::cerr<< "Origin (x,y) " << block.getOrigin().x << "y: " << block.getOrigin().y <<  std::endl;

    while (window.isOpen()){
        // Points de la base (au sol) - this part is for your interactive block
        // It should also incorporate the terrain height

        // === Event Handling and Camera Movement ===
        // Handle camera movement using WASD keys independently of the interactive block movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
            cameraPosition.y -= cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
            cameraPosition.y += cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
            cameraPosition.x -= cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
            cameraPosition.x += cameraSpeed;
        }

        // Apply the new camera position to the view
        cameraView.setCenter(cameraPosition);
        window.setView(cameraView);

        
        // Calculate the base height for the interactive block
        double noiseVal = pn.noise(gridX * frequency, gridY * frequency, 0.0);
        int baseHeight = static_cast<int>(std::floor(noiseVal * heightMultiplier));
        int currentZ = baseHeight + z_offset;


        sf::Vector2f baseP1 = isoToScreen(gridX, gridY, currentZ,tileWidth, tileHeight, width, height);
        sf::Vector2f baseP2 = isoToScreen(gridX + widthTiles, gridY, currentZ, tileWidth, tileHeight, width, height);
        sf::Vector2f baseP3 = isoToScreen(gridX + widthTiles, gridY + depthTiles, currentZ, tileWidth, tileHeight, width, height);
        sf::Vector2f baseP4 = isoToScreen(gridX, gridY + depthTiles, currentZ, tileWidth, tileHeight, width, height);

        // Face de BASE (sol du bâtiment)
        sf::ConvexShape baseFace(4);
        baseFace.setPoint(0, baseP1);
        baseFace.setPoint(1, baseP2);
        baseFace.setPoint(2, baseP3);
        baseFace.setPoint(3, baseP4);
        baseFace.setFillColor(sf::Color(255, 165, 0));
        baseFace.setOutlineThickness(1.5f);
        baseFace.setOutlineColor(sf::Color::Black);


        while (const std::optional event = window.pollEvent()){
            // ... (event handling code remains the same) ...
            if (event->is<sf::Event::Closed>()){
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape){
                    window.close();
                }
                if(keyPressed->scancode == sf::Keyboard::Scancode::Up){
                    gridY -= 1;
                }
                if(keyPressed->scancode == sf::Keyboard::Scancode::Down){
                    gridY +=1;
                }
                if(keyPressed->scancode == sf::Keyboard::Scancode::Left){
                    gridX-=1;
                }
                if(keyPressed->scancode == sf::Keyboard::Scancode::Right){
                    gridX +=1;
                }
                if(keyPressed->scancode == sf::Keyboard::Scancode::B){
                    z_offset +=1; // Modify z_offset
                }
                if(keyPressed->scancode == sf::Keyboard::Scancode::C){
                    z_offset -=1; // Modify z_offset
                }
            }
        }
        
        window.clear(sf::Color::Black);

        // Drawing the generated terrain
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                // Get a noise value for this coordinate
                double noiseValue = pn.noise(x * frequency, y * frequency, 0.0);
                // Map the noise value to a height (e.g., 0 to 5)
                int z = static_cast<int>(std::floor(noiseValue * heightMultiplier));
                
                block.setPosition(isoToScreen(x, y, z, tileWidth, tileHeight, width, height));
                window.draw(block);
            }
        }

        afficher_grille_iso(window, width, height);
        // Draw the interactive base after the main terrain
        // to ensure it is rendered on top of the terrain blocks
        window.draw(baseFace);
        
        
        window.display();
    }
    return 0;
}
