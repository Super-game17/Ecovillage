#include<iostream>
#include<SFML/Graphics.hpp>

int main(){
    unsigned int width = 600;
    unsigned int height = 360;

    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode({width,height}), "Teeetet");
    sf::Texture texture("assets/sprites/block.png");
    texture.setSmooth(true);

    sf::Sprite block(texture);

    block.setPosition({300.f, 180.f});
    block.setScale({0.3f, 0.3f});
    while (window->isOpen()){

        while (const std::optional event = window->pollEvent()){

            if (event->is<sf::Event::Closed>()){

                window->close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){

                if(keyPressed->scancode == sf::Keyboard::Scancode::Escape){

                    window->close();
                }
            }

        window->clear(sf::Color::Black);
        window->draw(block);
        window->display();

        }
    }

    delete window;
    return 0;
}