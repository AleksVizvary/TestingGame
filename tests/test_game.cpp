#include "../projekt_/game_core.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

namespace {

struct TestCase {
    const char* name;
    void (*fn)();
};

void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

struct CountingController : public IController {
    explicit CountingController(int& counter, ActionState next) : counter_(counter), next_(next) {}

    ActionState sample(const InputState&, const ControllerContext&, const DifficultyController&, float) override {
        counter_++;
        return next_;
    }

private:
    int& counter_;
    ActionState next_;
};

void test_main_structure() {
    const auto content = readFile("projekt_/main.cpp");

    expect(content.find("Game::init") != std::string::npos, "main.cpp should call Game::init");
    expect(content.find("game.handleInput()") != std::string::npos, "main.cpp should call handleInput");
    expect(content.find("game.update()") != std::string::npos, "main.cpp should call update");
    expect(content.find("game.draw()") != std::string::npos, "main.cpp should call draw");

    expect(content.find("sf::RenderWindow") == std::string::npos, "main.cpp should not manage RenderWindow");
    expect(content.find("pollEvent") == std::string::npos, "main.cpp should not poll events");
    expect(content.find("updateViewViewport") == std::string::npos, "main.cpp should not manage views");
}

void test_game_init_no_assets() {
    GameConfig config;
    config.player = CharacterConfig{"Player", {10.f, 10.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.bot = CharacterConfig{"Bot", {20.f, 20.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.startDifficulty = 3;
    config.enableRender = false;
    config.testMode = true;

    int playerCalls = 0;
    int botCalls = 0;
    GameCore core = GameCore::init(
        config,
        std::make_unique<CountingController>(playerCalls, ActionState{}),
        std::make_unique<CountingController>(botCalls, ActionState{}),
        nullptr);
    expect(core.hasControllers(), "GameCore::init should create controllers");
    expect(core.difficulty().level() == 3, "GameCore::init should honor start difficulty");
}

void test_controller_swap() {
    GameConfig config;
    config.player = CharacterConfig{"Player", {10.f, 10.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.bot = CharacterConfig{"Bot", {20.f, 20.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.startDifficulty = 1;
    config.enableRender = false;
    config.testMode = true;

    int playerCalls = 0;
    int botCalls = 0;

    GameCore core = GameCore::init(
        config,
        std::make_unique<CountingController>(playerCalls, ActionState{}),
        std::make_unique<CountingController>(botCalls, ActionState{}),
        nullptr);

    core.handleInput(InputState{});
    core.update(0.016f);

    expect(playerCalls == 1, "Player controller should be invoked once per update");
    expect(botCalls == 1, "Bot controller should be invoked once per update");
}

void test_difficulty_controller() {
    DifficultyController difficulty(2);
    expect(difficulty.level() == 2, "Difficulty should start at configured level");

    const float baseInterval = 1.0f;
    const float baseDelay = difficulty.botDecisionDelay();
    const float baseDrink = difficulty.botDrinkInterval(baseInterval);

    difficulty.advanceRound();
    expect(difficulty.level() == 3, "Difficulty should advance each round");
    expect(difficulty.botDecisionDelay() <= baseDelay, "Decision delay should not increase when difficulty rises");
    expect(difficulty.botDrinkInterval(baseInterval) <= baseDrink, "Drink interval should not increase when difficulty rises");
}

void test_draw_noop() {
    GameConfig config;
    config.player = CharacterConfig{"Player", {10.f, 10.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.bot = CharacterConfig{"Bot", {20.f, 20.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.startDifficulty = 1;
    config.enableRender = false;
    config.testMode = true;

    int playerCalls = 0;
    int botCalls = 0;
    GameCore core = GameCore::init(
        config,
        std::make_unique<CountingController>(playerCalls, ActionState{}),
        std::make_unique<CountingController>(botCalls, ActionState{}),
        nullptr);
    static_assert(std::is_same_v<decltype(&GameCore::draw), void (GameCore::*)()>, "GameCore::draw should take no arguments");

    core.draw();
}

void test_text_system() {
    GameConfig config;
    config.player = CharacterConfig{"Player", {10.f, 10.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.bot = CharacterConfig{"Bot", {20.f, 20.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.startDifficulty = 1;
    config.enableRender = false;
    config.testMode = true;

    int playerCalls = 0;
    int botCalls = 0;
    GameCore core = GameCore::init(
        config,
        std::make_unique<CountingController>(playerCalls, ActionState{}),
        std::make_unique<CountingController>(botCalls, ActionState{}),
        nullptr);
    expect(core.textSystem().size() >= 5, "Text system should register UI labels");
    expect(core.textSystem().hasLabel("score"), "Text system should include score label");
    expect(core.textSystem().hasLabel("round"), "Text system should include round label");
    expect(core.textSystem().hasLabel("aim"), "Text system should include aim label");
    expect(core.textSystem().hasLabel("drink"), "Text system should include drink label");
    expect(core.textSystem().hasLabel("move"), "Text system should include move label");
    expect(core.textSystem().hasLabel("level"), "Text system should include level label");
}

void test_asset_resilience() {
    GameConfig config;
    config.player = CharacterConfig{"Player", {10.f, 10.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.bot = CharacterConfig{"Bot", {20.f, 20.f}, {0, 0, 0, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}, false};
    config.startDifficulty = 1;
    config.enableRender = false;
    config.testMode = true;

    int playerCalls = 0;
    int botCalls = 0;
    GameCore core = GameCore::init(
        config,
        std::make_unique<CountingController>(playerCalls, ActionState{}),
        std::make_unique<CountingController>(botCalls, ActionState{}),
        nullptr);

    core.handleInput(InputState{});
    core.update(0.016f);
    core.draw();
}

} // namespace

int main() {
    const std::vector<TestCase> tests = {
        {"main_structure", test_main_structure},
        {"game_init_no_assets", test_game_init_no_assets},
        {"controller_swap", test_controller_swap},
        {"difficulty_controller", test_difficulty_controller},
        {"draw_noop", test_draw_noop},
        {"text_system", test_text_system},
        {"asset_resilience", test_asset_resilience},
    };

    int failures = 0;
    for (const auto& test : tests) {
        try {
            test.fn();
            std::cout << "[PASS] " << test.name << "\n";
        } catch (const std::exception& ex) {
            failures++;
            std::cerr << "[FAIL] " << test.name << ": " << ex.what() << "\n";
        }
    }

    if (failures > 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
