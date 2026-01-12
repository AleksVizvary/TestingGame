#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Color {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 255;
};

struct CharLook {
    int hatID = 0;
    int hairID = 0;
    int faceID = 0;
    Color topColor{};
    Color pantsColor{};
    Color shoeColor{};
    Color hairColor{};
};

struct CharacterConfig {
    std::string name;
    Vec2 startPos;
    CharLook look;
    bool showName = false;
};

struct GameConfig {
    CharacterConfig player;
    CharacterConfig bot;
    int startDifficulty = 1;
    bool enableRender = true;
    bool testMode = false;
};

struct InputState {
    bool closeRequested = false;
    bool mouseLeftPressed = false;
    Vec2 mouseUi{};
};

struct ControllerContext {
    bool isGameActive = false;
    bool isBotTurn = false;
    bool isIdle = false;
};

struct ActionState {
    bool chargeUp = false;
    bool chargeLeft = false;
    bool spaceDown = false;
    bool backPressed = false;
    bool requestThrow = false;
};

class DifficultyController {
public:
    explicit DifficultyController(int startLevel) : level_(startLevel) {}

    int level() const { return level_; }

    void advanceRound() { level_ += 1; }

    float botDrinkInterval(float baseInterval) const {
        const float scale = std::max(0.5f, 1.0f - 0.04f * static_cast<float>(level_ - 1));
        return baseInterval * scale;
    }

    float botDecisionDelay() const {
        return std::max(0.05f, 0.25f - 0.02f * static_cast<float>(level_ - 1));
    }

    float sampleAimMargin(std::mt19937& gen) const {
        if (level_ < 7) {
            std::uniform_real_distribution<> dis(-160.0f + (10 * level_), 160.0f - (10 * level_));
            return static_cast<float>(dis(gen));
        }
        std::uniform_real_distribution<> dis(-100.0f, 100.0f);
        return static_cast<float>(dis(gen));
    }

private:
    int level_ = 1;
};

class IController {
public:
    virtual ~IController() = default;
    virtual ActionState sample(const InputState& input, const ControllerContext& context, const DifficultyController& difficulty, float dt) = 0;
};

enum class TextStyle { Regular, Bold };

struct TextLabel {
    std::string text;
    Vec2 position;
    unsigned size = 12;
    Color color{255, 255, 255, 255};
    TextStyle style = TextStyle::Regular;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    bool contains(const Vec2& point) const {
        return point.x >= x && point.x <= (x + w) && point.y >= y && point.y <= (y + h);
    }
};

class TextSystem {
public:
    void addLabel(const std::string& id, const TextLabel& label) {
        labels_[id] = label;
    }

    void setText(const std::string& id, const std::string& text) {
        labels_.at(id).text = text;
    }

    void setPosition(const std::string& id, Vec2 position) {
        labels_.at(id).position = position;
    }

    std::size_t size() const {
        return labels_.size();
    }

    bool hasLabel(const std::string& id) const {
        return labels_.find(id) != labels_.end();
    }

    const std::unordered_map<std::string, TextLabel>& labels() const {
        return labels_;
    }

private:
    std::unordered_map<std::string, TextLabel> labels_;
};

struct GameConstants {
    Vec2 canStartPos{400.0f, 300.0f};
    Vec2 playerBallStart{700.0f, 200.0f};
    Vec2 botBallStart{100.0f, 200.0f};
    float gravity = 980.0f;
    float maxCharge = 800.0f;
    float playerRunStep = 10.0f;
    float botSpeedBase = 90.0f;
    float botDrinkInterval = 0.45f;
    float midOffset = 150.0f;
    int drinkMax = 30;
    float ballRadius = 10.0f;
    float canRadius = 15.0f;
    Rect playButtonRect{273.f, 260.f, 254.f, 104.f};
    Rect exitButtonRect{273.f, 400.f, 254.f, 104.f};
};

struct ThrowState {
    Vec2 velocity{};
    Vec2 initialVelocity{};
    float up = 0.0f;
    float left = 0.0f;
    bool isCharging = false;
    bool isFlying = false;
    bool hasHit = false;
    bool hitEvent = false;
};

struct PositionState {
    Vec2 player{750.f, 215.f};
    Vec2 bot{50.f, 215.f};
    Vec2 can{400.f, 300.f};
    Vec2 ball{700.f, 200.f};
    Vec2 botBall{100.f, 200.f};
};

struct RunState {
    bool playerRunning = false;
    bool botRunning = false;
    bool playerReturning = false;
    bool botReturning = false;
    float botRunSpeed = 1.0f;
    bool playerFacingRight = false;
};

struct DrinkState {
    int player = 0;
    int bot = 0;
    bool playerDrinking = false;
    bool botDrinking = false;
    float botDrinkAcc = 0.0f;
};

struct ScoreState {
    int player = 0;
    int bot = 0;
    int round = 1;
};

struct TurnState {
    bool isBotTurn = false;
    bool spaceHeld = false;
};

struct VisualState {
    Color canColor{255, 255, 0, 255};
};

enum class GameFlowState { Menu, Game };

struct GameModel {
    GameFlowState flowState = GameFlowState::Menu;
    ThrowState throwing;
    PositionState positions;
    RunState run;
    DrinkState drinks;
    ScoreState score;
    TurnState turn;
    VisualState visual;
    bool soundEvent = false;
};

class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;
    virtual void draw(const GameModel& model, const TextSystem& text) = 0;
};

class GameCore {
public:
    static GameCore init(const GameConfig& config,
                         std::unique_ptr<IController> player,
                         std::unique_ptr<IController> bot,
                         IRenderBackend* backend = nullptr);

    bool isRunning() const;
    void handleInput(const InputState& input);
    void update(float dt);
    void draw();
    bool consumeSoundEvent();

    const DifficultyController& difficulty() const { return difficulty_; }
    bool hasControllers() const { return playerController_ && botController_; }
    const TextSystem& textSystem() const { return textSystem_; }
    const GameModel& model() const { return model_; }

private:
    explicit GameCore(const GameConfig& config);

    void setupText();
    void handlePlayerThrow(const ActionState& actions, float rampUp);
    void handleBotThrow();
    void updatePhysics(float dt);
    void drinkCounter();
    void drinkCounterEnemy();
    void resetRound();
    void updateHudText();

    GameConfig config_{};
    GameConstants constants_{};
    GameModel model_{};
    DifficultyController difficulty_{1};
    std::unique_ptr<IController> playerController_;
    std::unique_ptr<IController> botController_;
    IRenderBackend* backend_ = nullptr;
    InputState input_{};
    bool isRunning_ = true;
    TextSystem textSystem_{};
};
