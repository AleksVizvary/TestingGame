#include "game_core.hpp"

#include <cmath>
#include <cstdlib>

namespace {

Vec2 operator+(const Vec2& a, const Vec2& b) {
    return {a.x + b.x, a.y + b.y};
}

Vec2 operator*(const Vec2& a, float scalar) {
    return {a.x * scalar, a.y * scalar};
}

} // namespace

GameCore::GameCore(const GameConfig& config)
    : config_(config), difficulty_(config.startDifficulty) {
    model_.positions.player = config.player.startPos;
    model_.positions.bot = config.bot.startPos;
    model_.positions.can = constants_.canStartPos;
    model_.positions.ball = constants_.playerBallStart;
    model_.positions.botBall = constants_.botBallStart;
    model_.run.playerFacingRight = false;
    if (config_.testMode) {
        model_.flowState = GameFlowState::Game;
    }
    setupText();
}

GameCore GameCore::init(const GameConfig& config,
                        std::unique_ptr<IController> player,
                        std::unique_ptr<IController> bot,
                        IRenderBackend* backend) {
    GameCore core(config);
    core.playerController_ = std::move(player);
    core.botController_ = std::move(bot);
    core.backend_ = backend;
    return core;
}

bool GameCore::isRunning() const {
    return isRunning_;
}

void GameCore::handleInput(const InputState& input) {
    input_ = input;
    if (input_.closeRequested) {
        isRunning_ = false;
    }
}

void GameCore::update(float dt) {
    if (!isRunning_) {
        return;
    }

    if (model_.flowState == GameFlowState::Menu) {
        if (input_.mouseLeftPressed) {
            if (constants_.playButtonRect.contains(input_.mouseUi)) {
                model_.flowState = GameFlowState::Game;
            } else if (constants_.exitButtonRect.contains(input_.mouseUi)) {
                isRunning_ = false;
            }
        }
        return;
    }

    ControllerContext context;
    context.isGameActive = true;
    context.isBotTurn = model_.turn.isBotTurn;
    context.isIdle = !model_.throwing.isFlying && !model_.run.playerRunning && !model_.run.botRunning &&
                     !model_.drinks.playerDrinking && !model_.drinks.botDrinking;

    if (!playerController_ || !botController_) {
        return;
    }

    ActionState playerActions = playerController_->sample(input_, context, difficulty_, dt);
    ActionState botActions = botController_->sample(input_, context, difficulty_, dt);

    if (playerActions.backPressed) {
        model_.flowState = GameFlowState::Menu;
        resetRound();
        return;
    }

    const float midX = model_.positions.can.x;
    const float midPlayerX = midX + constants_.midOffset;
    const float midBotX = midX - constants_.midOffset;

    const bool spacePressed = playerActions.spaceDown && !model_.turn.spaceHeld;
    model_.turn.spaceHeld = playerActions.spaceDown;

    if (context.isIdle) {
        if (!model_.turn.isBotTurn) {
            handlePlayerThrow(playerActions, 250.f * dt * 2);
        } else if (botActions.requestThrow) {
            handleBotThrow();
        }
    }

    if (model_.throwing.isFlying) {
        updatePhysics(dt);

        if (model_.throwing.hitEvent) {
            if (!model_.turn.isBotTurn) {
                model_.run.playerRunning = true;
                model_.run.playerReturning = false;

                model_.drinks.botDrinking = true;
                model_.drinks.playerDrinking = false;
                model_.drinks.botDrinkAcc = 0.f;
            } else {
                model_.run.botRunning = true;
                model_.run.botReturning = false;

                model_.run.botRunSpeed = 0.7f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.5f;

                model_.drinks.playerDrinking = true;
                model_.drinks.botDrinking = false;
            }

            model_.throwing.hitEvent = false;
        }
    }

    if (model_.run.playerRunning && spacePressed) {
        const float targetX = model_.run.playerReturning ? config_.player.startPos.x : midPlayerX;
        const float dir = (targetX > model_.positions.player.x) ? 1.f : -1.f;

        if (dir > 0.f) {
            model_.run.playerFacingRight = true;
        } else if (dir < 0.f) {
            model_.run.playerFacingRight = false;
        }

        model_.positions.player.x += dir * constants_.playerRunStep;

        const bool reached =
            (dir > 0.f && model_.positions.player.x >= targetX) ||
            (dir < 0.f && model_.positions.player.x <= targetX);

        if (reached) {
            model_.positions.player.x = targetX;

            if (!model_.run.playerReturning) {
                model_.run.playerReturning = true;
            } else {
                model_.run.playerReturning = false;
                model_.run.playerRunning = false;
                model_.drinks.botDrinking = false;
            }
        }
    }

    if (model_.run.botRunning) {
        const float targetX = model_.run.botReturning ? config_.bot.startPos.x : midBotX;
        const float dir = (targetX > model_.positions.bot.x) ? 1.f : -1.f;

        const float v = constants_.botSpeedBase * model_.run.botRunSpeed;
        model_.positions.bot.x += dir * v * dt;

        const bool reached =
            (dir > 0.f && model_.positions.bot.x >= targetX) ||
            (dir < 0.f && model_.positions.bot.x <= targetX);

        if (reached) {
            model_.positions.bot.x = targetX;

            if (!model_.run.botReturning) {
                model_.run.botReturning = true;
            } else {
                model_.run.botReturning = false;
                model_.run.botRunning = false;
                model_.drinks.playerDrinking = false;
            }
        }
    }

    if (model_.drinks.playerDrinking && spacePressed) {
        drinkCounter();
    }

    if (model_.drinks.botDrinking) {
        model_.drinks.botDrinkAcc += dt;
        const float interval = difficulty_.botDrinkInterval(constants_.botDrinkInterval);
        if (model_.drinks.botDrinkAcc >= interval) {
            drinkCounterEnemy();
            model_.drinks.botDrinkAcc -= interval;
        }
    }

    if (model_.drinks.bot >= constants_.drinkMax) {
        model_.score.bot++;
        model_.score.round++;
        difficulty_.advanceRound();
        resetRound();
    } else if (model_.drinks.player >= constants_.drinkMax) {
        model_.score.player++;
        model_.score.round++;
        difficulty_.advanceRound();
        resetRound();
    }

    updateHudText();
}

void GameCore::draw() {
    if (!config_.enableRender) {
        return;
    }
    if (backend_) {
        backend_->draw(model_, textSystem_);
    }
}

bool GameCore::consumeSoundEvent() {
    if (model_.soundEvent) {
        model_.soundEvent = false;
        return true;
    }
    return false;
}

void GameCore::setupText() {
    textSystem_.addLabel("score", {"", {20.f, 20.f}, 18, {255, 255, 255, 255}, TextStyle::Regular});
    textSystem_.addLabel("round", {"", {20.f, 45.f}, 18, {255, 255, 255, 255}, TextStyle::Regular});
    textSystem_.addLabel("aim", {"Celowanie - Strzalki(Gora/Lewo)", {400.f, 50.f}, 15, {255, 0, 0, 255}, TextStyle::Bold});
    textSystem_.addLabel("drink", {"Klikanie Spacji po trafieniu uzupelnia pasek", {400.f, 80.f}, 15, {255, 0, 0, 255}, TextStyle::Bold});
    textSystem_.addLabel("move", {"Jak trafi przeciwnik to ... (placeholder)", {400.f, 110.f}, 15, {255, 0, 0, 255}, TextStyle::Bold});
    textSystem_.addLabel("level", {std::to_string(difficulty_.level()), {300.f, 110.f}, 15, {255, 0, 0, 255}, TextStyle::Bold});

    if (config_.player.showName) {
        textSystem_.addLabel("playerName", {config_.player.name, {config_.player.startPos.x, config_.player.startPos.y - 20.f}, 14, {255, 255, 255, 255}, TextStyle::Bold});
    }
    if (config_.bot.showName) {
        textSystem_.addLabel("botName", {config_.bot.name, {config_.bot.startPos.x, config_.bot.startPos.y - 20.f}, 14, {255, 255, 255, 255}, TextStyle::Bold});
    }

    updateHudText();
}

void GameCore::updateHudText() {
    textSystem_.setText("score", "Gracz: " + std::to_string(model_.score.player) + "\tBot: " + std::to_string(model_.score.bot));
    textSystem_.setText("round", "Runda: " + std::to_string(model_.score.round));
    textSystem_.setText("level", std::to_string(difficulty_.level()));

    if (config_.player.showName) {
        textSystem_.setPosition("playerName", {model_.positions.player.x, model_.positions.player.y - 20.f});
    }
    if (config_.bot.showName) {
        textSystem_.setPosition("botName", {model_.positions.bot.x, model_.positions.bot.y - 20.f});
    }
}

void GameCore::handlePlayerThrow(const ActionState& actions, float rampUp) {
    if (actions.chargeUp || actions.chargeLeft) {
        model_.throwing.isCharging = true;
        if (actions.chargeUp) {
            model_.throwing.up += rampUp;
        }
        if (actions.chargeLeft) {
            model_.throwing.left += rampUp;
        }

        if (model_.throwing.up > constants_.maxCharge) {
            model_.throwing.up = 0.0f;
        }
        if (model_.throwing.left > constants_.maxCharge) {
            model_.throwing.left = 0.0f;
        }
    } else if (model_.throwing.isCharging) {
        model_.throwing.isCharging = false;
        model_.throwing.isFlying = true;
        model_.throwing.velocity = {-model_.throwing.left, -model_.throwing.up};
        model_.throwing.initialVelocity = model_.throwing.velocity;
        model_.throwing.hasHit = false;
        model_.throwing.up = 0.0f;
        model_.throwing.left = 0.0f;
    }
}

void GameCore::handleBotThrow() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(185.0f, 800.0f);

    float random_x = static_cast<float>(dis(gen));
    float margin = difficulty_.sampleAimMargin(gen);

    float distance = std::abs(model_.positions.can.x - model_.positions.botBall.x);
    float height = model_.positions.can.y - model_.positions.botBall.y;

    float physicsConstant = (constants_.gravity * distance) / 2.0f;
    float random_y = (physicsConstant / random_x) - ((height * random_x) / distance);

    model_.throwing.velocity = {random_x, -(margin + random_y)};
    model_.throwing.initialVelocity = model_.throwing.velocity;
    model_.throwing.hasHit = false;
    model_.throwing.isFlying = true;
}

void GameCore::updatePhysics(float dt) {
    model_.throwing.velocity.y += constants_.gravity * dt;

    Vec2& activeBall = model_.turn.isBotTurn ? model_.positions.botBall : model_.positions.ball;
    activeBall = activeBall + (model_.throwing.velocity * dt);

    const Vec2 diff = {
        activeBall.x - model_.positions.can.x,
        activeBall.y - model_.positions.can.y};
    const float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    const float collisionDistance = constants_.ballRadius + constants_.canRadius;

    if (distance <= collisionDistance && !model_.throwing.hasHit) {
        model_.throwing.velocity.x = -model_.throwing.velocity.x;
        model_.throwing.velocity.y = -model_.throwing.velocity.y;

        model_.visual.canColor = {255, 0, 255, 255};

        model_.throwing.hasHit = true;
        model_.throwing.hitEvent = true;
        model_.soundEvent = true;
    }

    if (activeBall.y > 600.f) {
        model_.throwing.isFlying = false;
        model_.throwing.velocity = {0.f, 0.f};
        if (model_.turn.isBotTurn) {
            model_.positions.botBall = constants_.botBallStart;
        } else {
            model_.positions.ball = constants_.playerBallStart;
        }
        model_.turn.isBotTurn = !model_.turn.isBotTurn;
    }
}

void GameCore::drinkCounter() {
    if (model_.drinks.player >= constants_.drinkMax) return;
    model_.drinks.player++;
}

void GameCore::drinkCounterEnemy() {
    if (model_.drinks.bot >= constants_.drinkMax) return;
    model_.drinks.bot++;
}

void GameCore::resetRound() {
    const GameFlowState keepFlow = model_.flowState;
    const int keepP = model_.score.player;
    const int keepB = model_.score.bot;
    const int keepR = model_.score.round;

    model_ = GameModel();
    model_.flowState = keepFlow;
    model_.score.player = keepP;
    model_.score.bot = keepB;
    model_.score.round = keepR;

    model_.positions.player = config_.player.startPos;
    model_.positions.bot = config_.bot.startPos;
    model_.positions.can = constants_.canStartPos;
    model_.positions.ball = constants_.playerBallStart;
    model_.positions.botBall = constants_.botBallStart;
    model_.visual.canColor = {255, 255, 0, 255};
    model_.run.playerFacingRight = false;
}
