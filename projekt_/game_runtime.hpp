#pragma once

#include <memory>

#include "game_core.hpp"

class Game {
public:
    static Game init(const GameConfig& config);

    bool isRunning() const;
    void handleInput();
    void update();
    void draw();

private:
    struct Impl;
    explicit Game(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl_;
};
