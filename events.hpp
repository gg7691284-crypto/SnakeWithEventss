#pragma once

#include "game.hpp"

class Events {
public:
    enum class Kind { none, gold, rocks };

    Kind kind() const { return active; }
    int remaining(int turn) const {
        return active == Kind::none ? 0 : endTurn - turn;
    }
    const std::vector<Point>& obstacles() const { return rocks; }
    std::optional<Point> apple() const { return gold; }

    void play(SnakeGame& game, Direction direction) {
        game.step(direction, rocks, gold);
        if (game.state != State::playing) {
            return;
        }

        const bool collectedGold = gold == game.snake.front();
        if (collectedGold) {
            game.score += 30;
        }
        if (active != Kind::none && (collectedGold || game.turn >= endTurn)) {
            active = Kind::none;
            gold.reset();
            rocks.clear();
            if (!game.food) {
                game.food = game.freeCell();
            }
        }

        // Регулярное начало событий удобно показывать на защите.
        if (game.turn % interval == 0) {
            start(game, game.turn / interval % 2 == 1 ? Kind::gold : Kind::rocks);
        }
    }

private:
    static constexpr int interval = 6;
    static constexpr int duration = 4;
    Kind active = Kind::none;
    int endTurn = 0;
    std::optional<Point> gold;
    std::vector<Point> rocks;

    void start(SnakeGame& game, Kind event) {
        std::vector<Point> blocked;
        if (game.food) {
            blocked.push_back(*game.food);
        }

        if (event == Kind::gold) {
            gold = game.freeCell(blocked);
            if (!gold) {
                return;
            }
        } else {
            for (int i = 0; i < 3; ++i) {
                const auto cell = game.freeCell(blocked);
                if (!cell) {
                    break;
                }
                rocks.push_back(*cell);
                blocked.push_back(*cell);
            }
            if (rocks.empty()) {
                return;
            }
        }
        active = event;
        endTurn = game.turn + duration;
    }
};
