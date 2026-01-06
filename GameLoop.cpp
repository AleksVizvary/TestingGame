#include <SFML/Graphics.hpp>
#include <optional>

// =======================================================
// ENUMY FSM
// =======================================================
enum class Phase { Menu, Turn, GameOver };
enum class Turn  { Player, Bot };
enum class Action { None, Rzut, Bieg, Picie };

// =======================================================
// ===== LOGIKA (FSM – ZERO SFML) =========================
// =======================================================

struct LogicCharacter {
    bool running  = false;
    bool returned = false;

    void resetTurn() {
        running = false;
        returned = false;
    }
};

struct LogicBall {
    bool flying = false;
    bool stopped() const { return !flying; }
};

struct Game {
    Phase phase  = Phase::Menu;
    Turn  turn   = Turn::Player;
    Action action = Action::None;

    LogicCharacter player;
    LogicCharacter bot;
    LogicBall ball;
};

// =======================================================
// ===== RENDER / SFML ===================================
// =======================================================

struct HealthBar {
    sf::RectangleShape frame;
    sf::RectangleShape fill;

    HealthBar() {
        frame.setSize({124.f, 18.f});
        frame.setFillColor(sf::Color::White);
        fill.setSize({112.f, 12.f});
        fill.setFillColor(sf::Color::Magenta);
    }
};

struct Label {
    sf::Text text;
    Label(const sf::Font& font, const std::string& name)
        : text(font, name, 22) {
        text.setFillColor(sf::Color::White);
    }
};

struct RenderCharacter {
    sf::Sprite sprite;
    HealthBar hp;
    Label label;

    RenderCharacter(
        const sf::Texture& tex,
        const sf::Font& font,
        const std::string& name,
        float x, float y
    ) : label(font, name), sprite(tex) {
        sprite.setPosition({x, y});
        auto b = sprite.getLocalBounds();
        sprite.setOrigin(b.size / 2.f);
    }

    void draw(sf::RenderTarget& t) const {
        t.draw(sprite);
        t.draw(label.text);
        t.draw(hp.frame);
        t.draw(hp.fill);
    }
};

struct World {
    sf::Sprite tile;

    World(const sf::Texture& tex, float y) : tile(tex) {
        tile.setPosition({0.f, y});
    }

    void draw(sf::RenderTarget& t) const {
        float w = tile.getLocalBounds().size.x;
        for (int i = 0; i < 40; ++i) {
            sf::Sprite s = tile;
            s.setPosition({i * w, tile.getPosition().y});
            t.draw(s);
        }
    }
};

struct RenderBall {
    sf::CircleShape shape{10.f};

    RenderBall() {
        shape.setFillColor(sf::Color::White);
    }
};

// =======================================================
// ===== INPUT ===========================================
// =======================================================

bool pressedThrow() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
}

bool pressedRun() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R);
}

// =======================================================
// ===== FSM UPDATE ======================================
// =======================================================

void updatePlayerTurn(float dt, Game& game)
{
    switch (game.action)
    {
        case Action::None:
            if (pressedThrow())
                game.action = Action::Rzut;
            else if (pressedRun())
                game.action = Action::Bieg;
            break;

        case Action::Rzut:
            game.ball.flying = false;
            if (game.ball.stopped())
                game.action = Action::Bieg;
            break;

        case Action::Bieg:
            game.player.running = true;
            game.player.returned = true;

            if (game.player.returned) {
                game.player.resetTurn();
                game.action = Action::None;
                game.turn = Turn::Bot;
            }
            break;

        default:
            break;
    }
}

void updateBotTurn(Game& game)
{
    game.ball.flying = false;
    if (game.ball.stopped()) {
        game.turn = Turn::Player;
        game.action = Action::None;
    }
}

void updateGame(float dt, Game& game)
{
    switch (game.phase)
    {
        case Phase::Menu:
            game.phase = Phase::Turn;
            break;

        case Phase::Turn:
            if (game.turn == Turn::Player)
                updatePlayerTurn(dt, game);
            else
                updateBotTurn(game);
            break;

        case Phase::GameOver:
            break;
    }
}

// =======================================================
// ===== MAIN ============================================
// =======================================================

sf::Texture loadTexture(const std::string& f) {
    sf::Texture t;
    t.loadFromFile(f);
    return t;
}

sf::Font loadFont(const std::string& f) {
    sf::Font font;
    font.openFromFile(f);
    return font;
}

int main()
{
    sf::RenderWindow window(
        sf::VideoMode({1280u, 720u}),
        "FLANKI FSM"
    );
    window.setFramerateLimit(60);

    sf::Clock clock;

    auto font = loadFont("robo.ttf");
    auto heroTex = loadTexture("Mashroom.png");
    auto botTex  = loadTexture("Viking.png");
    auto floorTex = loadTexture("floor.png");

    Game game;

    RenderCharacter hero(heroTex, font, "Mashroom", 200.f, 400.f);
    RenderCharacter bot(botTex, font, "Viking", 900.f, 400.f);
    World world(floorTex, 460.f);
    RenderBall ball;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())
                window.close();
        }

        updateGame(dt, game);

        // ===== POŁĄCZENIE LOGIKI → RENDER =====
        if (game.turn == Turn::Player && game.action == Action::Bieg) {
            hero.sprite.move({200.f * dt, 0.f});
        }

        window.clear(sf::Color::Black);
        world.draw(window);
        hero.draw(window);
        bot.draw(window);
        window.display();
    }

    return 0;
}
