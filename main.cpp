#include <optional>
#include <utility>
#include <SFML/Graphics.hpp>
#include "GameLoop.hpp"



int main()
{
    float gravity = 980.f;

    sf::Clock clock;
    float dt = clock.restart().asSeconds();

    GameLoop game(0, 200, 100);

    // main loop
    float angle = 0.9;
    // input → angle
    if (true)
        game.Gracz1.startRzut(angle);
        
    game.update(dt);

}

