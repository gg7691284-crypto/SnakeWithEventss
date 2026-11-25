#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <random>
#include <vector>

struct Point {
    int x;
    int y;

    bool operator==(Point other) const {
        return x == other.x && y == other.y;
    }
};

enum class Direction { up, left, down, right };
enum class State { playing, wall, body, rock, won };

class SnakeGame {
public:
    static constexpr int width = 20;
    static constexpr int height = 10;

    std::vector<Point> snake{{5, 4}, {4, 4}, {3, 4}};
    std::optional<Point> food;
    Direction direction = Direction::right;
    State state = State::playing;
    int score = 0;
    int turn = 0;

    explicit SnakeGame(unsigned seed = std::random_device{}()) : random(seed) {
        food = freeCell();
    }

    std::optional<Point> freeCell(const std::vector<Point>& blocked = {}) {
        std::vector<Point> cells;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const Point cell{x, y};
                if (std::find(snake.begin(), snake.end(), cell) == snake.end()
                    && std::find(blocked.begin(), blocked.end(), cell) == blocked.end()) {
                    cells.push_back(cell);
                }
            }
        }
        if (cells.empty()) {
            return std::nullopt;
        }
        const auto index = std::uniform_int_distribution<std::size_t>(0, cells.size() - 1)(random);
        return cells[index];
    }

    void step(Direction requested, const std::vector<Point>& rocks = {},
              std::optional<Point> bonus = std::nullopt) {
        if (state != State::playing) {
            return;
        }
        const std::array<Point, 4> offsets{{{0, -1}, {-1, 0}, {0, 1}, {1, 0}}};
        if ((static_cast<int>(requested) + 2) % 4 != static_cast<int>(direction)) {
            direction = requested;
        }
        const Point offset = offsets[static_cast<int>(direction)];
        const Point next{snake.front().x + offset.x, snake.front().y + offset.y};
        if (next.x < 0 || next.x >= width || next.y < 0 || next.y >= height) {
            state = State::wall;
            return;
        }
        if (std::find(rocks.begin(), rocks.end(), next) != rocks.end()) {
            state = State::rock;
            return;
        }
        const bool eats = food == next;
        // На обычном ходу хвост освобождает клетку.
        const auto occupiedEnd = eats ? snake.end() : snake.end() - 1;
        if (std::find(snake.begin(), occupiedEnd, next) != occupiedEnd) {
            state = State::body;
            return;
        }

        ++turn;
        snake.insert(snake.begin(), next);
        if (!eats) {
            snake.pop_back();
            return;
        }
        score += 10;
        auto blocked = rocks;
        if (bonus) {
            blocked.push_back(*bonus);
        }
        food = freeCell(blocked);
        if (snake.size() == width * height) {
            state = State::won;
        }
    }

private:
    std::mt19937 random;
};
