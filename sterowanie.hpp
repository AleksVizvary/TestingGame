#include <optional>
#include <utility>
#include <SFML/Graphics.hpp>
#include <iostream>

// Klawisze "O" i "P" przytrzymane razem przez ok. 2 sekundy uruchamiaja podnoszenie
// Klawisze "K" i "L" wciskane na przemian (nie razem) powodują picie
// Klawisze "A" i "D" dają przemieszczanie się po planszy 
// Strzałki "<",">" i "^" tworzą wektor siły rzutu

std::optional<sf::Vector2f> sterowanieRzutem(float dt)
{
    const float chargeSpeed = 0.4;

    static bool byloCharging = false;
    static float x = 0.f;
    static float y = 0.f;

bool up    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
bool left  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);

    bool charging = up || left || right;

    // ŁADOWANIE
    if (charging)
    {
        if (up)    y += chargeSpeed * dt;
        if (right) x += chargeSpeed * dt;
        if (left)  x -= chargeSpeed * dt;
    }

    if (x >= 1 || x <= -1) x = 0.f;
    if (y >= 1) y = 0.f;

    // MOMENT PUSZCZENIA
    if (byloCharging && !charging)
    {   
        x = std::round(x * 100.0) / 100.0;
        y = std::round(y * 100.0) / 100.0;

        sf::Vector2f wynik{x, y};
        std::cout << "\033[2J\033[H";
        std::cout << "\nwektor rzutu: x=" << x << " y=" << y << std::endl;
        // reset
        x = 0.f;
        y = 0.f;
        byloCharging = false;
        return wynik;
    }

    byloCharging = charging;
    return std::nullopt;
}


std::optional<float> sterowanieRuchem(float dt)
{

    static float x = 0.f;

    bool left  = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A);
    bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D);

    bool charging = left || right;

    // ŁADOWANIE
    if (charging)
    {
        if (right) x += 50 * dt;
        if (left)  x -= 50 * dt;
     
    x = std::round(x * 10.0) / 10.0;
    float wynik = x;
    std::cout << "\033[2J\033[H";
    std::cout << "\nruch o wektor x: " << x << std::endl;
    // reset
    x = 0.f;
    return wynik;
    }

    return std::nullopt;
}


std::optional<float> sterowaniePiciem(float dt)
{

    static bool kPressed = false;
    static float timer = 0.f;

    bool K = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::K);
    bool L = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::L);

    if (K)
    {
        kPressed = true;
        timer = 0.f;
    }

    if (kPressed)
    {
        timer += dt;

        if (L && timer < 0.05f && timer > 0.03f)   // 300 ms
        {
            kPressed = false;
            std::cout << "\033[2J\033[H";
            std::cout << "\nwektor picia o przesunięty o: 1" << std::endl;
            return 1; 
        }

        if (timer >= 0.05f)
        {
            kPressed = 0;    // timeout
        }
    }
    
    return 0;
}


std::optional<bool> sterowaniePodnoszenia(float dt)
{
    static float czasTrzymania = 0;

    bool O = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::O);
    bool P = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::P);
    if (O && P) czasTrzymania += 0.1;
    
    if (czasTrzymania >= 5) 
    {
        std::cout << "\033[2J\033[H";
        std::cout << "\nUruchomione podniesienie\n";
        czasTrzymania = 0;
        return true;
    }
    return false;
}
