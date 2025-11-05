// Camera.hpp
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(float viewWidth, float viewHeight, sf::Vector2f initialCenter)
        : speed(15.0f) {
        view.setSize({viewWidth, viewHeight});
        view.setCenter(initialCenter);
    }

    void update(sf::RenderWindow& window) {
        // Appliquer la nouvelle position à la vue SFML
        window.setView(view);
    }

    // Gère le mouvement en fonction des touches pressées
    void handleInput() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
            view.move({0.f, -speed});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
            view.move({0.f, speed});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
            view.move({-speed, 0.f});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
            view.move({speed, 0.f});
        }
    }

    // Méthode utilitaire pour obtenir la position du centre de la caméra
    sf::Vector2f getCenter() const {
        return view.getCenter();
    }

private:
    sf::View view;
    float speed;
};

#endif // CAMERA_HPP
