#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <SFML/Graphics.hpp>
#include <algorithm> // pour std::clamp

class Camera {
private:
    sf::View view;
    float speed;
    float zoomLevel;
    float zoomMin;
    float zoomMax;

public:
    Camera(float viewWidth, float viewHeight, sf::Vector2f initialCenter)
        : speed(15.0f), zoomLevel(1.0f), zoomMin(0.3f), zoomMax(10.0f) {
        view.setSize({viewWidth, viewHeight});
        view.setCenter(initialCenter);
    }

    void update(sf::RenderWindow& window) {
        window.setView(view);
    }

    void handleInput(const sf::RenderWindow& window) {
        // --- Déplacement (WASD) ---
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

        // --- Zoom clavier (optionnel) ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add)) { // touche +
            zoom(0.98f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract)) { // touche -
            zoom(1.02f);
        }
    }

    // --- Gestion du zoom via molette ---
    void handleEvent(const sf::Event& event) {
        if (event.is<sf::Event::MouseWheelScrolled>()) {
            float delta = event.getIf<sf::Event::MouseWheelScrolled>()->delta;
            if (delta > 0)
                zoom(0.9f);  // zoom avant
            else if (delta < 0)
                zoom(1.1f);  // zoom arrière
        }
    }

    void zoom(float factor) {
        float newZoom = zoomLevel * factor;
        newZoom = std::clamp(newZoom, zoomMin, zoomMax);

        // Appliquer uniquement si le zoom change
        if (newZoom != zoomLevel) {
            factor = newZoom / zoomLevel;
            zoomLevel = newZoom;
            view.zoom(factor);
        }
    }

    sf::Vector2f getCenter() const {
        return view.getCenter();
    }

    float getZoomLevel() const {
        return zoomLevel;
    }
};

#endif // CAMERA_HPP
