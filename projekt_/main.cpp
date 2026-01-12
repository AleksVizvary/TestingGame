#include "game_runtime.hpp"

int main() {
    GameConfig config;
    config.player = CharacterConfig{
        "Gracz",
        Vec2{750.0f, 215.0f},
        {1, 2, 4, {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 0, 255}},
        false};
    config.bot = CharacterConfig{
        "Bot",
        Vec2{50.0f, 215.0f},
        {2, 1, 3, {0, 200, 255, 255}, {255, 80, 80, 255}, {200, 200, 200, 255}, {80, 255, 120, 255}},
        false};
    config.startDifficulty = 1;
    config.enableRender = true;

    Game game = Game::init(config);

    while (game.isRunning()) {
        game.handleInput();
        game.update();
        game.draw();
    }

    return 0;
}
