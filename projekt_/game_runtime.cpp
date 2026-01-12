#include "game_runtime.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <optional>

#include "helper.hpp"

namespace {

sf::Vector2f mainWin = {800.0f, 600.0f};

constexpr const char* kButtonTexture = "../assets/img/button.png";

sf::Color toSfmlColor(const Color& color) {
    return sf::Color(color.r, color.g, color.b, color.a);
}

sf::Vector2f toSfmlVec(const Vec2& vec) {
    return {vec.x, vec.y};
}

class SfmlRenderBackend final : public IRenderBackend {
public:
    SfmlRenderBackend(sf::RenderWindow& window,
                      sf::View& view,
                      sf::RectangleShape& background,
                      sf::CircleShape& can,
                      sf::CircleShape& playerBall,
                      sf::CircleShape& botBall,
                      QTEbar& playerBar,
                      greyBar& playerFill,
                      QTEbar& botBar,
                      greyBar& botFill,
                      charSprite* playerSprite,
                      charSprite* botSprite,
                      sf::Font& font)
        : window_(window),
          view_(view),
          background_(background),
          can_(can),
          playerBall_(playerBall),
          botBall_(botBall),
          playerBar_(playerBar),
          playerFill_(playerFill),
          botBar_(botBar),
          botFill_(botFill),
          playerSprite_(playerSprite),
          botSprite_(botSprite),
          font_(font) {}

    void setBackground(sf::Texture& menuTexture, sf::Texture& gameTexture, GameFlowState state) {
        background_.setTexture(state == GameFlowState::Game ? &gameTexture : &menuTexture);
    }

    void draw(const GameModel& model, const TextSystem& text) override {
        window_.clear(sf::Color::Black);
        window_.setView(view_);
        window_.draw(background_);

        can_.setPosition(toSfmlVec(model.positions.can));
        can_.setFillColor(toSfmlColor(model.visual.canColor));
        playerBall_.setPosition(toSfmlVec(model.positions.ball));
        botBall_.setPosition(toSfmlVec(model.positions.botBall));

        if (playerSprite_) {
            playerSprite_->setPos(model.positions.player.x, model.positions.player.y);
            if (model.run.playerFacingRight) {
                playerSprite_->flip(1);
            } else {
                playerSprite_->flip(-1);
            }
        }

        if (botSprite_) {
            botSprite_->setPos(model.positions.bot.x, model.positions.bot.y);
        }

        window_.draw(playerBall_);
        window_.draw(can_);
        window_.draw(botBall_);

        if (botSprite_) {
            botSprite_->draw(window_);
        }
        if (playerSprite_) {
            playerSprite_->draw(window_);
        }

        window_.draw(playerBar_);
        window_.draw(botBar_);

        const float playerBarBaseY = 540.f;
        const float botBarBaseY = 540.f;
        playerFill_.setPosition({645 + 100, playerBarBaseY - (5.f * model.drinks.player)});
        botFill_.setPosition({145 - 100, botBarBaseY - (5.f * model.drinks.bot)});

        window_.draw(playerFill_);
        window_.draw(botFill_);

        drawPowerBars(model);
        drawText(text);

        window_.display();
    }

private:
    void drawPowerBars(const GameModel& model) {
        if (!model.throwing.isFlying && (model.throwing.up > 0.0f || model.throwing.left > 0.0f)) {
            if (model.throwing.up > 0.0f) {
                sf::RectangleShape powerBar(sf::Vector2f(10.f, -model.throwing.up / 10.f));
                powerBar.setFillColor(sf::Color::Red);
                powerBar.setPosition(playerBall_.getPosition());
                window_.draw(powerBar);
            }
            if (model.throwing.left > 0.0f) {
                sf::RectangleShape powerBar(sf::Vector2f(-model.throwing.left / 10.f, 10.f));
                powerBar.setFillColor(sf::Color::Red);
                powerBar.setPosition(playerBall_.getPosition());
                window_.draw(powerBar);
            }
        }
    }

    void drawText(const TextSystem& text) {
        for (const auto& [id, label] : text.labels()) {
            sf::Text sfLabel(font_, label.text, label.size);
            sfLabel.setFillColor(toSfmlColor(label.color));
            if (label.style == TextStyle::Bold) {
                sfLabel.setStyle(sf::Text::Bold);
            }
            sfLabel.setPosition(toSfmlVec(label.position));
            window_.draw(sfLabel);
        }
    }

    sf::RenderWindow& window_;
    sf::View& view_;
    sf::RectangleShape& background_;
    sf::CircleShape& can_;
    sf::CircleShape& playerBall_;
    sf::CircleShape& botBall_;
    QTEbar& playerBar_;
    greyBar& playerFill_;
    QTEbar& botBar_;
    greyBar& botFill_;
    charSprite* playerSprite_;
    charSprite* botSprite_;
    sf::Font& font_;
};

class KeyboardController final : public IController {
public:
    ActionState sample(const InputState&, const ControllerContext&, const DifficultyController&, float) override {
        ActionState actions;
        actions.chargeUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W) ||
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up);
        actions.chargeLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A) ||
                             sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left);
        actions.spaceDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space);
        actions.backPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape);
        return actions;
    }
};

class AIController final : public IController {
public:
    ActionState sample(const InputState&, const ControllerContext& context, const DifficultyController& difficulty, float dt) override {
        ActionState actions;
        if (!context.isGameActive) {
            return actions;
        }

        if (!context.isBotTurn || !context.isIdle) {
            decisionTimer_ = 0.0f;
            return actions;
        }

        decisionTimer_ += dt;
        if (decisionTimer_ >= difficulty.botDecisionDelay()) {
            actions.requestThrow = true;
            decisionTimer_ = 0.0f;
        }
        return actions;
    }

private:
    float decisionTimer_ = 0.0f;
};

} // namespace

struct Game::Impl {
    GameConfig config{};
    GameCore core;

    sf::RenderWindow window;
    sf::View view{sf::FloatRect({0.f, 0.f}, {800.f, 600.f})};

    sf::Texture mainMenuBG{};
    sf::Texture gameBG{};
    sf::RectangleShape background{{800.f, 600.f}};

    sf::CircleShape can{15.f};
    sf::CircleShape playerBall{10.f};
    sf::CircleShape botBall{10.f};

    sf::Font font{};

    sf::SoundBuffer soundBuffer{};
    sf::Sound sound{};

    Button playButton{{254.f, 104.f}, {273.f, 260.f}, sf::Color(96, 178, 37), sf::Color(109, 204, 42), "START", font, 30, kButtonTexture};
    Button exitButton{{254.f, 104.f}, {273.f, 400.f}, sf::Color(178, 37, 37), sf::Color(204, 42, 42), "WYJSCIE", font, 30, kButtonTexture};

    charSprite playerSprite{sf::Vector2f({0.f, 0.f})};
    charSprite botSprite{sf::Vector2f({0.f, 0.f})};

    QTEbar playerBar{20.f, 50.f, 0.f};
    greyBar playerFill{30.f, 10.f, 0.f};
    QTEbar botBar{20.f, 50.f, 0.f};
    greyBar botFill{30.f, 10.f, 0.f};

    SfmlRenderBackend renderer;

    InputState input{};
    sf::Clock clock{};

    Impl(const GameConfig& cfg)
        : config(cfg),
          core(GameCore::init(cfg,
                              std::make_unique<KeyboardController>(),
                              std::make_unique<AIController>(),
                              nullptr)),
          window(sf::VideoMode({800, 600}), "Flanki"),
          renderer(window, view, background, can, playerBall, botBall, playerBar, playerFill, botBar, botFill, &playerSprite, &botSprite, font) {
        window.setFramerateLimit(60);
        view.setCenter({400.f, 300.f});
        updateViewViewport(window, view);
        window.setView(view);

        mainMenuBG.loadFromFile("../assets/img/mainmenu.png");
        gameBG.loadFromFile("../assets/img/gamebg.png");
        background.setPosition({0.f, 0.f});
        background.setTexture(&mainMenuBG);

        font.openFromFile("../assets/fonts/DejaVuSans.ttf");

        soundBuffer.loadFromFile("../assets/music/clank.mp3");
        sound.setBuffer(soundBuffer);

        can.setFillColor(sf::Color::Yellow);
        can.setPosition({400.f, 300.f});

        playerBall.setFillColor(sf::Color::Black);
        playerBall.setPosition({700.f, 200.f});

        botBall.setFillColor(sf::Color::White);
        botBall.setPosition({100.f, 200.f});

        playerBar.setPosition({650 + 100, 400});
        playerFill.setPosition({645 + 100, 540});
        botBar.setPosition({150 - 100, 400});
        botFill.setPosition({145 - 100, 540});

        playerSprite = charSprite(toSfmlVec(config.player.startPos), {config.player.look.hatID, config.player.look.hairID, config.player.look.faceID,
                                                                       toSfmlColor(config.player.look.topColor), toSfmlColor(config.player.look.pantsColor),
                                                                       toSfmlColor(config.player.look.shoeColor), toSfmlColor(config.player.look.hairColor)});
        botSprite = charSprite(toSfmlVec(config.bot.startPos), {config.bot.look.hatID, config.bot.look.hairID, config.bot.look.faceID,
                                                                toSfmlColor(config.bot.look.topColor), toSfmlColor(config.bot.look.pantsColor),
                                                                toSfmlColor(config.bot.look.shoeColor), toSfmlColor(config.bot.look.hairColor)});

        core = GameCore::init(cfg,
                              std::make_unique<KeyboardController>(),
                              std::make_unique<AIController>(),
                              &renderer);
    }

    bool isRunning() const {
        return window.isOpen() && core.isRunning();
    }

    void handleInput() {
        InputState newInput{};
        newInput.closeRequested = false;
        newInput.mouseLeftPressed = false;

        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosUI = window.mapPixelToCoords(mousePosI);
        newInput.mouseUi = {mousePosUI.x, mousePosUI.y};

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                newInput.closeRequested = true;
            } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                updateViewViewport(window, view);
                view.setSize({800.f, 600.f});
                view.setCenter({400.f, 300.f});
                window.setView(view);
                (void)resized;
            } else if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    newInput.mouseLeftPressed = true;
                }
            }
        }

        input = newInput;
        core.handleInput(input);
    }

    void update() {
        float dt = clock.restart().asSeconds();
        core.update(dt);
        if (core.consumeSoundEvent()) {
            sound.play();
        }
    }

    void draw() {
        renderer.setBackground(mainMenuBG, gameBG, core.model().flowState);
        if (core.model().flowState == GameFlowState::Menu) {
            sf::Vector2f mousePos{input.mouseUi.x, input.mouseUi.y};
            if (playButton.isMouseOver(mousePos)) {
                playButton.hover();
            } else {
                playButton.unhover();
            }
            if (exitButton.isMouseOver(mousePos)) {
                exitButton.hover();
            } else {
                exitButton.unhover();
            }
            window.clear(sf::Color::Black);
            window.setView(view);
            window.draw(background);
            playButton.draw(window);
            exitButton.draw(window);
            window.display();
            return;
        }
        core.draw();
    }
};

Game::Game(std::unique_ptr<Game::Impl> impl) : impl_(std::move(impl)) {}

Game Game::init(const GameConfig& config) {
    return Game(std::make_unique<Game::Impl>(config));
}

bool Game::isRunning() const {
    return impl_->isRunning();
}

void Game::handleInput() {
    impl_->handleInput();
}

void Game::update() {
    impl_->update();
}

void Game::draw() {
    impl_->draw();
}
