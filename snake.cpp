#include "events.hpp"

#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

void draw(const SnakeGame& game, const Events& events) {
    std::vector<std::string> field(SnakeGame::height, std::string(SnakeGame::width, '.'));
    for (Point rock : events.obstacles()) {
        field[rock.y][rock.x] = 'X';
    }
    if (game.food) {
        field[game.food->y][game.food->x] = '@';
    }
    if (const auto apple = events.apple()) {
        field[apple->y][apple->x] = '*';
    }
    for (Point part : game.snake) {
        field[part.y][part.x] = 'o';
    }
    const Point head = game.snake.front();
    field[head.y][head.x] = 'O';

    std::cout << "\nSNAKE WITH EVENTS   Score: " << game.score << "   Turn: " << game.turn << '\n';
    std::cout << '+' << std::string(SnakeGame::width, '-') << "+\n";
    for (const auto& row : field) {
        std::cout << '|' << row << "|\n";
    }
    std::cout << '+' << std::string(SnakeGame::width, '-') << "+\n";
    if (events.kind() != Events::Kind::none) {
        std::cout << (events.kind() == Events::Kind::gold ? "Gold apple (*)" : "Falling rocks (X)")
                  << " | turns left: " << events.remaining(game.turn) << '\n';
    }
}

std::optional<Direction> readDirection(const std::string& command, Direction current) {
    if (command.empty()) {
        return current;
    }
    if (command.size() != 1) {
        return std::nullopt;
    }
    switch (std::tolower(static_cast<unsigned char>(command.front()))) {
        case 'w': return Direction::up;
        case 'a': return Direction::left;
        case 's': return Direction::down;
        case 'd': return Direction::right;
        default: return std::nullopt;
    }
}

void showResult(const SnakeGame& game) {
    switch (game.state) {
        case State::wall: std::cout << "Game over: you hit the wall.\n"; break;
        case State::body: std::cout << "Game over: you hit your body.\n"; break;
        case State::rock: std::cout << "Game over: you hit a rock.\n"; break;
        case State::won: std::cout << "You win: the board is full!\n"; break;
        case State::playing: break;
    }
}

void demo() {
    SnakeGame game(7);
    Events events;
    game.food = Point{6, 4};
    for (const char command : std::string_view("ddddddssaaaa")) {
        events.play(game, *readDirection(std::string(1, command), game.direction));
        if (game.turn == 1 || game.turn == 6 || game.turn == 10 || game.turn == 12) {
            draw(game, events);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string_view(argv[1]) == "--demo") {
        demo();
        return 0;
    }
    if (argc != 1) {
        std::cerr << "Usage: snake [--demo]\n";
        return 1;
    }

    std::cout << "W/A/S/D + Enter: move | Enter: straight | Q: quit\n"
              << "@: food (+10, grows) | *: gold (+30) | X: rock\n";
    while (true) {
        SnakeGame game;
        Events events;
        draw(game, events);
        std::string command;

        while (game.state == State::playing) {
            std::cout << "Move > ";
            if (!std::getline(std::cin, command) || command == "q" || command == "Q") {
                return 0;
            }
            const auto direction = readDirection(command, game.direction);
            if (!direction) {
                std::cout << "Use W, A, S, D, Enter or Q.\n";
                continue;
            }
            events.play(game, *direction);
            draw(game, events);
        }
        showResult(game);
        std::cout << "Final score: " << game.score << " | R: restart | other key: exit > ";
        if (!std::getline(std::cin, command) || (command != "r" && command != "R")) {
            return 0;
        }
    }
}
