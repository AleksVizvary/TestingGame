#include <SFML/Graphics.hpp>
#include <optional>

// Klasa postaci
// ====================================================================
enum class GameState {
    Playing,
    GameOver,
    Pause
};

struct HealthBar {

    sf::RectangleShape frame;
    sf::RectangleShape fill;

    HealthBar()
    {   
        frame.setSize({124.f, 18.f});
        frame.setFillColor(sf::Color::White);

        fill.setSize({112.f, 12.f});
        fill.setFillColor(sf::Color::Magenta);
    }
    
};

struct Label {

    sf::Text label;

    Label(const sf::Font& font, std::string Name): 
    label(font, Name, 22)
    {
        label.setFillColor(sf::Color::White);
    }
};

struct GameObject {
    HealthBar health;
    Label label;
    sf::Sprite sprite;

    float maxHp = 100.f;
    float hp    = 100.f;
    
    float velocityY = 0.f;

    float groundY = 0.f;   
    bool onGround = true;

    GameObject(
        const sf::Texture& texture,
        const sf::Font& font,
        const std::string& name,
        float x,
        float y
        ): 
        health(), 
        label(font, name), 
        sprite(texture){
            sprite.setPosition({x, y});
            const sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.size / 2.f);
        }

    bool isDead() const {
        return hp <= 0.f;
    }

    void damage(float damageAmount) {
        hp -= damageAmount;
        if (hp < 0.f)    hp = 0.f;
        if (hp > maxHp) hp = maxHp;

        float ratio = hp / maxHp;
        health.fill.setSize({112.f * ratio, 12.f});
    }
    
    void healthBarColor(float isDamage) {
        if (isDamage == 1) 
            health.fill.setFillColor(sf::Color::Green);
        else if (isDamage == 2) 
            health.fill.setFillColor(sf::Color::Red);
        else 
            health.fill.setFillColor(sf::Color::Magenta);
    }
   
    void update() {
        sf::Vector2f p = sprite.getPosition();
        const sf::FloatRect b = sprite.getLocalBounds();

        label.label.setPosition({
        p.x - 30,
        p.y - b.size.y * 0.5f - 16.f   // 16px nad głową
        });

        health.frame.setPosition({
        p.x - health.frame.getSize().x * 0.5f,
        p.y + b.size.y * 0.5f + 6.f
        });

        health.fill.setPosition({
        health.frame.getPosition().x + 6.f,
        health.frame.getPosition().y + 3.f
        });

    }
    
    void draw(sf::RenderTarget& target) const {
        target.draw(sprite);
        target.draw(label.label);
        target.draw(health.frame);
        target.draw(health.fill);
    }

};
// ====================================================================

// Klasa świata
// ====================================================================

struct World {

    sf::Sprite floor;

    World(
        const sf::Texture& texture,
        float y
        ):
        floor(texture){
            floor.setPosition({0, y + 100});
            const sf::FloatRect bounds = floor.getLocalBounds();

        }
    void draw(sf::RenderTarget& target) const {
        auto basePos = floor.getPosition();
        auto width = floor.getLocalBounds().size.x;

        for (int i = 0; i < 80; i++) {
            sf::Sprite tile = floor;
            tile.setPosition({basePos.x + i * width, basePos.y});
            target.draw(tile);
        }
    }

};



// Prototypy
// ====================================================================
void movement(float speed, GameObject &hero, float dt);
void damage(GameObject &hero);
void move(sf::Vector2f &pos, float moveX, float speed, float dt, GameObject &hero);
void setToJump(GameObject & hero);
void jump(GameObject &hero, float dt);
sf::Texture loadTexture(const std::string& file);
sf::Font loadFont(const std::string& file);

// Stałe
// ====================================================================

const float gravity   = 1200.f;
const float jumpPower = -500.f;

// Main
// ====================================================================
int main()
{
    // --- Setup rzeczy --- 
    // ================================================================================
    // --- window ---
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "GU60 G4M3");
    window.setFramerateLimit(60);

    // --- timing ---
    sf::Clock clock;
    constexpr float speed = 250.f;

    // --- font ---
    auto font = loadFont("robo.ttf");

    // -- Inicjacja swiata --
    // ================================================================================
    float wysokosc = 300;
    auto floorTexture = loadTexture("floor.png");
    World Podłoga{floorTexture, wysokosc};

    // ================================================================================

    // -- Inicjacja postaci --
    // ================================================================================

    // 1.
    std::string characterName = "Mashroom";
    auto texture1 = loadTexture(characterName + ".png");
    GameObject hero(texture1, font, characterName, 200, wysokosc);

    hero.groundY   = wysokosc;
    // ================================================================================
    // 2.
    std::string badguyName = "Viking";
    auto texture2 = loadTexture(badguyName + ".png");
    GameObject badguy(texture2, font, badguyName, 1080, wysokosc);

    badguy.groundY = wysokosc;
    // ================================================================================


    // -- Inicjacja gry --
    // ================================================================================
    GameState state = GameState::Playing;
    float floorLevel = hero.sprite.getPosition().y;
    float jumpLevel = hero.sprite.getPosition().y;
    // ================================================================================

    // -- Pętla --
    // ================================================================================
    while (window.isOpen()) {
        
        float dt = clock.restart().asSeconds();

        // 1️⃣ events
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // 2️⃣ input 
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            window.close();

        // 3️⃣ logika i fizyka
        movement(speed, hero, dt);
        damage(hero);

        badguy.update();
        hero.update();

        if (hero.isDead()) 
            state = GameState::GameOver;
        

        // 4️⃣ rysowanie
        window.clear(sf::Color::Black);
        Podłoga.draw(window);

        hero.draw(window);
        badguy.draw(window);
        window.display();

    }

    return 0;
}

// -- Funckje --
// ================================================================================

sf::Texture loadTexture(const std::string& file)
{
    sf::Texture tex;
    if (!tex.loadFromFile(file))
        throw std::runtime_error("Texture load failed");
    return tex;
}

sf::Font loadFont(const std::string& file)
{
    sf::Font font;
    if (!font.openFromFile(file))
        throw std::runtime_error("Font load failed");
    return font;
}

// ==========================================================================================

void movement(float speed, GameObject &hero, float dt)
{  
    float moveX = 0.f;
    sf::Vector2f pos = hero.sprite.getPosition();

    // A < 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        moveX -= 1.f;

    // D >
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        moveX += 1.f;

    // faktyczny ruch
    move(pos, moveX, speed, dt, hero);

    // przyspieszenie grawitacyjne działające na postać
    hero.velocityY += gravity * dt;
    
    // SKOK
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && hero.onGround)
        setToJump(hero);
    
    jump(hero, dt);



    // ============================================================   // ============================================================
}

void move(sf::Vector2f &pos, float moveX, float speed, float dt, GameObject &hero)
{
    pos.x += moveX * speed * dt;
    hero.sprite.setPosition(pos);
}

void setToJump(GameObject & hero)
{
    hero.velocityY = jumpPower;
    hero.onGround = false;
}

void jump(GameObject &hero, float dt)
{
    sf::Vector2f pos = hero.sprite.getPosition();
    pos.y += hero.velocityY * dt;
    hero.sprite.setPosition(pos);

    if (pos.y >= hero.groundY)
    {
        pos.y = hero.groundY;
        hero.sprite.setPosition(pos);

        hero.velocityY = 0.f;
        hero.onGround = true;
    }
}

void damage(GameObject &hero){

    float kill = 0;
    float isDamage = 0;

    // kill
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
    {
        kill = 0.5f;
        isDamage = 2;
    }

    // heal
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L))
    {
        kill = -0.5f;
        isDamage = 1;
    }

    hero.damage(kill);
    hero.healthBarColor(isDamage);
}
