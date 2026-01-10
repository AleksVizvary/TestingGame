#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include "globals.hpp"
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include <random>
#include <fstream>
#include <chrono>
#include <thread>
#include "helper.hpp"
#include <cmath> 


sf::Vector2f mainWin = { 800.0f, 600.0f };
void updateViewViewport(const sf::RenderWindow&, sf::View&);
enum class GameState { Menu, Game, GameMenu };
std::string buttText = "../assets/img/button.png";


sf::Vector2f enemyBasePos = sf::Vector2f({ 50.0, 215.0 });
sf::Vector2f playerBasePos = sf::Vector2f({ 750.0, 215.0 });

charLook playerChar = { 1, 2, 4, sf::Color(255, 0, 0), sf::Color(0, 255, 0), sf::Color(0, 0, 255), sf::Color(255, 255, 0) };
charSprite playerSP(playerBasePos, playerChar);
charLook enemyChar = { 2, 1, 3, sf::Color(0, 200, 255), sf::Color(255, 80, 80), sf::Color(200, 200, 200), sf::Color(80, 255, 120) };
charSprite enemySP(enemyBasePos, enemyChar);


struct GameStart {
    sf::Vector2f velocity = { 0.f, 0.f };

    float up = 0.0f;
    float left = 0.0f;

    bool isCharging = false;
    bool isFlying = false;

    float maxCharge = 800.0f;

    float bot_ball_x = 100.f;
    float ball_x = 700.f;
    float ball_y = 200.f;

    bool turn = false;

    bool hasHit = false;
    bool hitEvent = false;

    sf::Vector2f initialVelocity;

    int myDrink = 0;
    int enemyDrink = 0;
    bool isSpaceActive = false;

    float graczX = 750.f;
    float graczY = 215.f;
    float botX   = 50.f;
    float botY   = 215.f;

    bool graczBiegnie = false;
    bool botBiegnie   = false;

    bool graczWraca = false;
    bool botWraca   = false;

    float graczHomeX = 750.f;
    float botHomeX   = 50.f;

    float botRunSpeed = 1.f;
    bool graczPije = false;
    bool botPije   = false;

    float botDrinkAcc = 0.f;

    bool spaceHeld = false; 
    int scorePlayer = 0;
    int scoreBot = 0;
    int round = 1;          // zamiast level jak wolisz

    bool graczFacingRight = true;
};

void logic(GameState &currentState, GameStart &game, sf::CircleShape &ball, sf::CircleShape &can, float ramp_up, sf::CircleShape &ball2, float gravity, 
    float dt, sf::Sound &sound, greyBar &visBar, int &level, sf::Text &levelDisplay, greyBar& visEnemyBar);

void odbicie(sf::CircleShape &ball, float pozycja_x, GameStart &game, float dt, sf::CircleShape &can, sf::Sound &sound);

void rzutBot(sf::CircleShape &can, sf::CircleShape &ball2, float gravity, GameStart &game, int& level);

void rzutGracz(GameStart &game, float ramp_up);

void bounce(sf::CircleShape &ball, sf::CircleShape &can, GameStart &game, sf::Sound &sound);

void groundReset(sf::CircleShape &ball, GameStart &game, float ball_x);

void drawGame(GameState currentState, Button &playButton, sf::RenderWindow &window, Button &exitButton, sf::CircleShape &ball, sf::CircleShape &can, sf::CircleShape &ball2, GameStart &game, sf::Text &aim, sf::Text &move, sf::Text &drink, QTEbar &drinkBar, greyBar &visBar, sf::Text &levelDisplay, QTEbar& enemyBar, greyBar& visEnemyBar, sf::Text scoreText, sf::Text roundText);

void drawBars(GameStart &game, sf::CircleShape &ball, sf::RenderWindow &window);

void drawPowerbar(GameStart &game, sf::CircleShape &ball, sf::RenderWindow &window, float direction_up, float direction_left);

void handleMenu(const std::optional<sf::Event> &event, Button &playButton, sf::Vector2f &mousePos, GameState &currentState, Button &exitButton, sf::RenderWindow &window);

void eventLoop(const std::optional<sf::Event> &event, sf::RenderWindow &window, sf::View &view, GameState &currentState, Button &playButton, sf::Vector2f &mousePos, Button &exitButton);

void drinkCounter(GameStart& game, greyBar &visBar);

void drinkCounterEnemy(GameStart& game, greyBar &visEnemyBar);

int main()
{   

    // SETUP
    GameStart game;
    playerSP.flip(1), game.graczFacingRight = false;


    //  Rozmiar okna, resize
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Flanki");
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect({ 0.f, 0.f }, mainWin));
    view.setCenter(sf::Vector2f(mainWin.x / 2.f, mainWin.y / 2.f));
    updateViewViewport(window, view);
    window.setView(view);

    // stan gry (menu czy gra)
    GameState currentState = GameState::Menu;

    // Assety
    sf::Font font;
    if (!font.openFromFile("../assets/fonts/DejaVuSans.ttf")) return -1;

    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("../assets/music/clank.mp3")) return -1;
    
    // Przyciski 
    sf::Clock keyTimer;
    Button playButton({ 254.f, 104.f }, { 273.f, 260.f }, sf::Color(96, 178, 37), sf::Color(109, 204, 42), "START", font, 30, buttText);
    Button exitButton({ 254.f, 104.f }, { 273.f, 400.f }, sf::Color(178, 37, 37), sf::Color(204, 42, 42), "WYJSCIE", font, 30, buttText);

    // Tło
    sf::Texture mainMenuBG;
    if (!mainMenuBG.loadFromFile("../assets/img/mainmenu.png")) return -1;
    sf::Texture gameBG;
    if (!gameBG.loadFromFile("../assets/img/gamebg.png")) return -1;

    sf::RectangleShape logicalBackground(mainWin);
    logicalBackground.setPosition({ 0.f, 0.f });
    logicalBackground.setTexture(&mainMenuBG);

    // Obiekty - puszki
    sf::CircleShape can(15.f);
    can.setFillColor(sf::Color::Yellow);
    can.setPosition({ 400.f, 300.f });

    sf::CircleShape ball(10.f);
    ball.setFillColor(sf::Color::Black);
    ball.setPosition({ game.ball_x, game.ball_y });

    sf::CircleShape ball2(10.f);
    ball2.setFillColor(sf::Color::White);
    ball2.setPosition({ game.bot_ball_x, game.ball_y });

    sf::Sound sound(buffer);

    // Punkty

    sf::Text scoreText(font, "", 18);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({20.f, 20.f});

    sf::Text roundText(font, "", 18);
    roundText.setFillColor(sf::Color::White);
    roundText.setPosition({20.f, 45.f});
    
    
    sf::Text aim(font, "Celowanie - Strzalki(Gora/Lewo)");
    aim.setCharacterSize(15);
    aim.setStyle(sf::Text::Bold);
    aim.setFillColor(sf::Color::Red);
    aim.setPosition({ 400, 50});

    sf::Text drink(font, "Klikanie Spacji po trafieniu uzupelnia pasek");
    drink.setCharacterSize(15);
    drink.setStyle(sf::Text::Bold);
    drink.setFillColor(sf::Color::Red);
    drink.setPosition({ 400, 80});

    sf::Text move(font, "Jak trafi przeciwnik to ... (placeholder)");
    move.setCharacterSize(15);
    move.setStyle(sf::Text::Bold);
    move.setFillColor(sf::Color::Red);
    move.setPosition({ 400, 110 });
    
    // gracz bar 
    QTEbar drinkBar(20.f, 50.f, 0.f);
    drinkBar.setPosition({ 650 + 100, 400 });
    
    greyBar visBar(30.f, 10.f, 0.f);
    visBar.setPosition({ 645 + 100, 540 });

    // enemy bar 
    QTEbar enemyBar(20.f, 50.f, 0.f);
    enemyBar.setPosition({ 150 - 100, 400 });

    greyBar visEnemyBar(30.f, 10.f, 0.f);
    visEnemyBar.setPosition({ 145 - 100, 540 });

    

    int level = 1;
    //char levelChar = '1';
    sf::Text levelDisplay(font, std::to_string(level));
    levelDisplay.setCharacterSize(15);
    levelDisplay.setStyle(sf::Text::Bold);
    levelDisplay.setFillColor(sf::Color::Red);
    levelDisplay.setPosition({ 300, 110 });


    // fizyka i czas
    float gravity = 980.f;
    sf::Clock clock;

    while (window.isOpen()) {

        if (currentState == GameState::Game) {
            logicalBackground.setTexture(&gameBG);
        }

        float dt = clock.restart().asSeconds();

        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosUI = window.mapPixelToCoords(mousePosI); // bez view

        sf::Vector2f mousePos = window.mapPixelToCoords(mousePosI, view);
        float ramp_up = 250.f * dt * 2;

        while (const std::optional event = window.pollEvent())
        eventLoop(event, window, view, currentState, playButton, mousePosUI, exitButton);
            
        logic(currentState, game, ball, can, ramp_up, ball2, gravity, dt, sound, visBar, level, levelDisplay, visEnemyBar);
        
        scoreText.setString("Gracz: " + std::to_string(game.scorePlayer) + "\tBot: " + std::to_string(game.scoreBot));
        roundText.setString("Runda: " + std::to_string(game.round));

        window.clear(sf::Color::Black);
        window.setView(view);
        window.draw(logicalBackground);

        drawGame(currentState, playButton, window, exitButton, ball, can, ball2, game, aim, move, drink, drinkBar, visBar, levelDisplay, enemyBar, visEnemyBar, scoreText, roundText);

        window.display();

        std::cout
        << "\033[2J\033[H"
        << "turn=" << game.turn
        << " flying=" << game.isFlying
        << " graczBiegnie=" << game.graczBiegnie
        << " botBiegnie=" << game.botBiegnie
        << " charging=" << game.isCharging
        << " up=" << game.up << " left=" << game.left
        << "\n";
    }
}


void eventLoop(const std::optional<sf::Event> &event, sf::RenderWindow &window, sf::View &view, GameState &currentState, Button &playButton, sf::Vector2f &mousePos, Button &exitButton){

    // 1.1 zamknięcie
    if (event->is<sf::Event::Closed>())
        window.close();

    // 1.2 resize okna
    else if (const auto *resized = event->getIf<sf::Event::Resized>())
    {
        updateViewViewport(window, view);
        view.setSize(mainWin);
        view.setCenter({mainWin.x / 2.f, mainWin.y / 2.f});
        window.setView(view);
    }

    // 1.3 menu
    else if (currentState == GameState::Menu)
    {
        handleMenu(event, playButton, mousePos, currentState, exitButton, window);
    }
}

void handleMenu(const std::optional<sf::Event> &event, Button &playButton, sf::Vector2f &mousePos, GameState &currentState, Button &exitButton, sf::RenderWindow &window)
{
    if (playButton.isMouseOver(mousePos)) {
        playButton.hover();
    }
    else {
        playButton.unhover();
    }
    if (exitButton.isMouseOver(mousePos)) {
        exitButton.hover();
    }
    else {
        exitButton.unhover();
    }
    if (const auto *mouseEvent = event->getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            if (playButton.isMouseOver(mousePos))
                currentState = GameState::Game;
            if (exitButton.isMouseOver(mousePos))
                window.close();
        }
    }
}


void logic(GameState &currentState, GameStart &game,
           sf::CircleShape &ball, sf::CircleShape &can,
           float ramp_up, sf::CircleShape &ball2,
           float gravity, float dt, sf::Sound &sound,
           greyBar &visBar, int &level, sf::Text &levelDisplay,
           greyBar &visEnemyBar)
{
    if (currentState != GameState::Game) return;

    const int   DRINK_MAX         = 30;
    const float MID_OFFSET        = 150.f;   // żeby nie wchodzili w puszkę
    const float PLAYER_RUN_STEP   = 10.f;   // px na jedno kliknięcie spacji
    const float BOT_SPEED_BASE    = 90.f;   // px/s (bazowo wolniej)
    const float BOT_DRINK_INTERVAL= 0.45f;  // sekundy na 1 "łyk" (wolno)

    const float midX = can.getPosition().x;
    const float midPlayerX = midX + MID_OFFSET;
    const float midBotX    = midX - MID_OFFSET;

    // klik
    const bool spaceDown    = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space);
    const bool spacePressed = spaceDown && !game.spaceHeld;
    game.spaceHeld = spaceDown;

    auto resetRound = [&]() {
        const int keepP = game.scorePlayer;
        const int keepB = game.scoreBot;
        const int keepR = game.round;

        game = GameStart();
        game.scorePlayer = keepP;
        game.scoreBot    = keepB;
        game.round       = keepR;

        visBar.setPosition({645 + 100, 540});
        visEnemyBar.setPosition({145 - 100, 540});
        can.setFillColor(sf::Color::Yellow);

        ball.setPosition({game.ball_x, game.ball_y});
        ball2.setPosition({game.bot_ball_x, game.ball_y});

        playerSP.setPos(game.graczX, game.graczY);
        enemySP.setPos(game.botX, game.botY);
    };

    // ESC: reset do menu
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape)) {
        currentState = GameState::Menu;
        resetRound();
        return;
    }

    // ===== 1 RZUT / ŁADOWANIE =====
    if (!game.isFlying && !game.graczBiegnie && !game.botBiegnie && !game.graczPije && !game.botPije)
    {
        if (!game.turn) rzutGracz(game, ramp_up);
        else            rzutBot(can, ball2, gravity, game, level);
    }

    // ===== 2 LOT =====
    if (game.isFlying)
    {
        game.velocity.y += gravity * dt;

        if (!game.turn) odbicie(ball,  game.ball_x,     game, dt, can, sound);
        else            odbicie(ball2, game.bot_ball_x, game, dt, can, sound);

        // event trafienia przychodzi z bounce
        if (game.hitEvent)
        {
            if (!game.turn) {
                game.graczBiegnie = true;
                game.graczWraca   = false;

                game.botPije      = true;
                game.graczPije    = false;
                game.botDrinkAcc  = 0.f;
            } else {
                game.botBiegnie = true;
                game.botWraca   = false;

                game.botRunSpeed = 0.7f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.5f; // 0.7–1.2

                game.graczPije = true;
                game.botPije   = false;
            }

            game.hitEvent = false; // zużyte
        }
    }

    // ===== 3 RUCH PO TRAFIENIU =====

    // gracz bieg
    if (game.graczBiegnie && spacePressed)
    {
        const float targetX = game.graczWraca ? game.graczHomeX : midPlayerX;
        const float dir     = (targetX > game.graczX) ? 1.f : -1.f;

        // === FLIP ===
        if (dir > 0.f && !game.graczFacingRight) {
            playerSP.flip(1);
            game.graczFacingRight = true;
        }
        else if (dir < 0.f && game.graczFacingRight) {
            playerSP.flip(1);
            game.graczFacingRight = false;
        }

        game.graczX += dir * PLAYER_RUN_STEP;

        const bool reached =
            (dir > 0.f && game.graczX >= targetX) ||
            (dir < 0.f && game.graczX <= targetX);

        if (reached) {
            game.graczX = targetX;

            if (!game.graczWraca) {
                game.graczWraca = true;
            } else {
                game.graczWraca   = false;
                game.graczBiegnie = false;
                game.botPije      = false;
            }
        }
    }


    // bot bieg
    if (game.botBiegnie)
    {
        const float targetX = game.botWraca ? game.botHomeX : midBotX;
        const float dir     = (targetX > game.botX) ? 1.f : -1.f;

        const float v = BOT_SPEED_BASE * game.botRunSpeed;
        game.botX += dir * v * dt;

        const bool reached =
            (dir > 0.f && game.botX >= targetX) ||
            (dir < 0.f && game.botX <= targetX);

        if (reached) {
            game.botX = targetX;

            if (!game.botWraca) 
            {
                game.botWraca = true;
                // enemySP.flip();
            }
            else {
                game.botWraca   = false;
                game.botBiegnie = false;
                game.graczPije  = false;
            }
        }
    }

    // sync spriteów
    playerSP.setPos(game.graczX, game.graczY);
    enemySP.setPos(game.botX,   game.botY);

    // ===== 4 PICIE =====
    if (game.graczPije && spacePressed) {
        drinkCounter(game, visBar);
    }

    if (game.botPije) {
        game.botDrinkAcc += dt;
        if (game.botDrinkAcc >= BOT_DRINK_INTERVAL) {
            drinkCounterEnemy(game, visEnemyBar);
            game.botDrinkAcc -= BOT_DRINK_INTERVAL;
        }
    }

    // ===== 5 PUNKTY =====
    if (game.enemyDrink >= DRINK_MAX) {
        game.scoreBot++;
        game.round++;
        resetRound();
    }
    else if (game.myDrink >= DRINK_MAX) {
        game.scorePlayer++;
        game.round++;
        resetRound();
    }
}

void odbicie(sf::CircleShape &ball, float pozycja_x, GameStart &game, float dt, sf::CircleShape &can, sf::Sound &sound)
{
    ball.move(game.velocity * dt);        // ruch puszki
    bounce(ball, can, game, sound);       // odbicie
    groundReset(ball, game, pozycja_x); // reset po odbiciu
}

void rzutBot(sf::CircleShape &can, sf::CircleShape &ball2, float gravity, GameStart &game, int &level)
{

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(185.0f, 800.0f);
    float margin;

    if (level < 7)
    {
        std::uniform_real_distribution<> dis2(-160.0f + (10 * level), 160.0f - (10 * level));
        margin = dis2(gen);
    }
    else {
        std::uniform_real_distribution<> dis2(-100.0f , 100.0f );
        margin = dis2(gen);
    }

    float random_x = dis(gen);

    // Flight trajectory math
    //  t = Distance/Vx
    // Height = (Vy​*t)−(1/2​*Gravity*t^2)
    // Vy = ((Gravity*Distance) / (2*Vx)) + (Height * Vx / Distance)

    // Distance ball2 from can

    // x axis
    float distance = std::abs(can.getPosition().x - ball2.getPosition().x);

    // y axis
    float height = can.getPosition().y - ball2.getPosition().y;

    // Velocityx * Velocityy = gravity*distance / 2
    float physicsConstant = (gravity * distance) / 2.0f;

    // Veloicty = gravity * distance /2 / velocityx - ((height * random_x)/ distance) (- because y axis in sfml upside down)
    float random_y = (physicsConstant / random_x) - ((height * random_x) / distance);

    std::cout << random_x << ' ' << random_y << std::endl;

    // error (0 = enemy always hits)
    //int error = 200;
    //srand(time(NULL));
    //float margin = rand() % (2*error) - error;
     
    game.velocity = sf::Vector2f(random_x, -(margin + random_y));
    std::cout  << "margin" << margin<< std::endl;
    
    // game.velocity = sf::Vector2f(687.352, -175.465);
    game.initialVelocity = game.velocity;
    game.hasHit = false;

    game.isFlying = true;
}

void rzutGracz(GameStart &game, float ramp_up)
{

    bool chargingUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up);
    bool chargingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left);

    // Ładowanie do rzutu
    if (chargingUp || chargingLeft)
    {
        game.isCharging = true;
        // Charging until max
        if (chargingUp)
            game.up += ramp_up;
        if (chargingLeft)
            game.left += ramp_up;

        // If charged to max:

        // Cap at max

        /*if (game.up > game.maxCharge) game.up = game.maxCharge;
        if (game.left > game.maxCharge) game.left = game.maxCharge;*/

        // Reset bar
        if (game.up > game.maxCharge)
            game.up = 0.0f;
        if (game.left > game.maxCharge)
            game.left = 0.0f;
    }
    // Rzut
    else if (game.isCharging)
    {
        game.isCharging = false;
        game.isFlying = true;
        game.velocity = sf::Vector2f(-game.left, -game.up);
        game.initialVelocity = game.velocity;
        game.hasHit = false;
        game.up = 0.0f;
        game.left = 0.0f;
    }
}

void bounce(sf::CircleShape &ball, sf::CircleShape &can, GameStart &game, sf::Sound &sound)
{
    if (ball.getGlobalBounds().findIntersection(can.getGlobalBounds()) && !game.hasHit)
    {
        game.velocity.x = -game.velocity.x;
        game.velocity.y = -game.velocity.y;

        can.setFillColor(sf::Color::Magenta);

        game.hasHit   = true;   // blokada na resztę lotu
        game.hitEvent = true;   // JEDNORAZOWY event do logic()
    }
}

void drinkCounter(GameStart& game, greyBar &visBar)
{
    constexpr int DRINK_MAX = 30;
    if (game.myDrink >= DRINK_MAX) return;

    game.myDrink++;
    visBar.move({0.f, -5.f});
}

void drinkCounterEnemy(GameStart& game, greyBar& visEnemyBar)
{
    constexpr int DRINK_MAX = 30;
    if (game.enemyDrink >= DRINK_MAX) return;

    game.enemyDrink++;
    visEnemyBar.move({0.f, -5.f});
}

void groundReset(sf::CircleShape &ball, GameStart &game, float ball_x)
{
    if (ball.getPosition().y > 600.f)
    {
        game.isFlying = false;
        game.velocity = {0.f, 0.f};
        ball.setPosition({ball_x, game.ball_y});
        game.turn = !game.turn;
    }

}

void drawGame(GameState currentState, Button& playButton, sf::RenderWindow& window,
    Button& exitButton, sf::CircleShape& ball, sf::CircleShape& can,
    sf::CircleShape& ball2, GameStart& game,
    sf::Text& aim, sf::Text& move, sf::Text& drink, QTEbar &drinkBar, greyBar &visBar, sf::Text &levelDisplay, QTEbar& enemyBar, greyBar& visEnemyBar, sf::Text scoreText, sf::Text roundText)
{
    if (currentState == GameState::Menu)
    {
        playButton.draw(window);
        exitButton.draw(window);
    }
    else if (currentState == GameState::Game)
    {
        window.draw(scoreText);
        window.draw(roundText);

        window.draw(ball);
        window.draw(can);
        window.draw(ball2);
        
        playerSP.flip();
        enemySP.draw(window);
        playerSP.draw(window);

        window.draw(aim);
        window.draw(move);
        window.draw(drink);

        window.draw(drinkBar);
        window.draw(enemyBar);

        window.draw(visBar);
        window.draw(visEnemyBar);

        window.draw(levelDisplay);

        drawBars(game, ball, window);
    }

}

void drawBars(GameStart &game, sf::CircleShape &ball, sf::RenderWindow &window)
{
    if (!game.isFlying && (game.up > 0.0f || game.left > 0.0f))
    {
        if (game.up > 0.0f)
        {
            sf::RectangleShape powerBar(sf::Vector2f(10.f, -game.up / 10.f));
            powerBar.setFillColor(sf::Color::Red);
            powerBar.setPosition(ball.getPosition());
            window.draw(powerBar);
        }
        if (game.left > 0.0f)
        {
            sf::RectangleShape powerBar(sf::Vector2f(-game.left / 10.f, 10.f));
            powerBar.setFillColor(sf::Color::Red);
            powerBar.setPosition(ball.getPosition());
            window.draw(powerBar);
        }
    }
}


