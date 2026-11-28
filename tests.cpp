#include "events.hpp"

#include <cstdlib>
#include <iostream>

void expect(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        std::exit(1);
    }
}

int main() {
    SnakeGame game(7);
    game.food = Point{6, 4};
    game.step(Direction::left);
    expect(game.snake.front() == Point{6, 4}, "reverse input is ignored");
    expect(game.score == 10 && game.snake.size() == 4, "normal apple grows the snake");

    game.snake = {{1, 1}, {1, 2}, {0, 2}, {0, 1}};
    game.food = Point{6, 4};
    game.direction = Direction::up;
    game.step(Direction::left);
    expect(game.state == State::playing && game.snake.front() == Point{0, 1},
           "the previous tail cell is available");
    game.step(Direction::left);
    expect(game.state == State::wall, "wall collision");

    SnakeGame self(7);
    self.snake = {{1, 1}, {1, 2}, {0, 2}, {0, 1}, {0, 0}};
    self.direction = Direction::up;
    self.food.reset();
    self.step(Direction::left);
    expect(self.state == State::body, "body collision");

    SnakeGame full(7);
    full.snake = {{1, 0}};
    for (int y = 0; y < SnakeGame::height; ++y) {
        for (int x = 0; x < SnakeGame::width; ++x) {
            if (y != 0 || x > 1) {
                full.snake.push_back({x, y});
            }
        }
    }
    full.direction = Direction::up;
    full.food = Point{0, 0};
    full.step(Direction::left);
    expect(full.state == State::won && !full.food, "a full board is a win");

    SnakeGame demo(7);
    demo.food = Point{6, 4};
    Events events;
    for (int i = 0; i < 6; ++i) {
        events.play(demo, Direction::right);
    }
    expect(events.kind() == Events::Kind::gold && events.apple()
           && !(events.apple() == demo.food) && events.remaining(demo.turn) == 4,
           "gold starts at turn six on a free cell");

    const auto apple = *events.apple();
    const auto length = demo.snake.size();
    const int nextRow = apple.y > 0 ? apple.y - 1 : 1;
    demo.snake = {{apple.x > 0 ? apple.x - 1 : 1, apple.y},
                  {0, nextRow}, {1, nextRow}, {2, nextRow}};
    demo.direction = Direction::up;
    events.play(demo, apple.x > 0 ? Direction::right : Direction::left);
    expect(demo.score == 40 && demo.snake.size() == length && !events.apple(),
           "gold gives 30 points without growth");
    expect(events.kind() == Events::Kind::none && events.remaining(demo.turn) == 0,
           "collecting gold ends the event immediately");

    SnakeGame timeline(7);
    timeline.food.reset();
    Events timelineEvents;
    for (int i = 0; i < 10; ++i) {
        timelineEvents.play(timeline, i < 6 ? Direction::right : Direction::down);
        if (i == 9) {
            expect(timelineEvents.kind() == Events::Kind::none && !timelineEvents.apple(),
                   "uncollected gold expires");
        }
    }
    timelineEvents.play(timeline, Direction::left);
    timelineEvents.play(timeline, Direction::left);
    expect(timelineEvents.kind() == Events::Kind::rocks && timelineEvents.obstacles().size() == 3,
           "rocks start at turn twelve");
    const auto rock = timelineEvents.obstacles().front();
    timeline.snake = {{rock.x > 0 ? rock.x - 1 : 1, rock.y}};
    timeline.direction = Direction::up;
    timelineEvents.play(timeline, rock.x > 0 ? Direction::right : Direction::left);
    expect(timeline.state == State::rock, "rock collision");
    std::cout << "All game checks passed.\n";
}
