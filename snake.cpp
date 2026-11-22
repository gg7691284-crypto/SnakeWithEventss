#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <ctime>

// Структура для координат
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// Перечисление направлений
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

class SnakeGame {
private:
    bool gameOver;
    const int width = 40;
    const int height = 20;
    int score;
    int delayMs; // Задержка в миллисекундах (регулирует скорость)
    
    Point food;
    Direction dir;
    std::vector<Point> snake;

public:
    SnakeGame() {
        srand(time(0));
        Setup();
    }

    // Инициализация начального состояния
    void Setup() {
        gameOver = false;
        dir = RIGHT;
        score = 0;
        delayMs = 100; // Начальная скорость

        snake.clear();
        // Начальная длина змейки — 3 сегмента в центре поля
        snake.push_back({width / 2, height / 2});
        snake.push_back({width / 2 - 1, height / 2});
        snake.push_back({width / 2 - 2, height / 2});

        SpawnFood();
    }

    // Генерация еды в свободном месте
    void SpawnFood() {
        bool valid = false;
        while (!valid) {
            food.x = rand() % (width - 2) + 1;
            food.y = rand() % (height - 2) + 1;
            valid = true;
            // Проверяем, чтобы еда не появилась внутри змейки
            for (const auto& segment : snake) {
                if (segment == food) {
                    valid = false;
                    break;
                }
            }
        }
    }

    // Считывание нажатий клавиш
    void Input() {
        int ch = getch(); // Неблокирующий ввод благодаря timeout()
        switch (ch) {
            case 'a': case 'A': case KEY_LEFT:
                if (dir != RIGHT) dir = LEFT;
                break;
            case 'd': case 'D': case KEY_RIGHT:
                if (dir != LEFT) dir = RIGHT;
                break;
            case 'w': case 'W': case KEY_UP:
                if (dir != DOWN) dir = UP;
                break;
            case 's': case 'S': case KEY_DOWN:
                if (dir != UP) dir = DOWN;
                break;
            case 'x': case 'X': case 27: // Выход по X или ESC
                gameOver = true;
                break;
        }
    }

    // ==========================================================
    // ТОЧКА РАСШИРЕНИЯ ДЛЯ ВАШИХ СОБЫТИЙ
    // ==========================================================
    void ProcessEvents() {
        // Сюда можно добавлять любую логику, которая срабатывает 
        // по времени, при наборе очков или случайно:
        
        // Пример 1: Ускорение змейки каждые 50 очков
        // if (score > 0 && score % 50 == 0 && delayMs > 40) {
        //     delayMs -= 5;
        //     timeout(delayMs);
        // }

        // Пример 2: Событие при достижении определенного счета
        // if (score == 100) { /* например, спавн стены или золотого яблока */ }
    }

    // Основная игровая логика (физика и правила)
    void Logic() {
        if (dir == STOP) return;

        // Вычисляем новую позицию головы
        Point newHead = snake[0];
        switch (dir) {
            case LEFT:  newHead.x--; break;
            case RIGHT: newHead.x++; break;
            case UP:    newHead.y--; break;
            case DOWN:  newHead.y++; break;
            default: break;
        }

        // Проверка столкновения со стенами
        if (newHead.x <= 0 || newHead.x >= width - 1 || 
            newHead.y <= 0 || newHead.y >= height - 1) {
            gameOver = true;
            return;
        }

        // Проверка столкновения с самим собой
        for (size_t i = 0; i < snake.size(); i++) {
            if (snake[i] == newHead) {
                gameOver = true;
                return;
            }
        }

        // Добавляем новую голову
        snake.insert(snake.begin(), newHead);

        // Проверка поедания еды
        if (newHead == food) {
            score += 10;
            SpawnFood();
            ProcessEvents(); // Проверяем события после съедения
        } else {
            // Если не съели, удаляем хвост (движение вперед)
            snake.pop_back();
        }
    }

    // Отрисовка кадра в терминале
    void Draw() {
        erase(); // Очистка буфера экрана (без мерцания)

        // Отрисовка границ поля
        for (int x = 0; x < width; x++) {
            mvaddch(0, x, '#');
            mvaddch(height - 1, x, '#');
        }
        for (int y = 0; y < height; y++) {
            mvaddch(y, 0, '#');
            mvaddch(y, width - 1, '#');
        }

        // Отрисовка еды
        attron(COLOR_PAIR(2));
        mvaddch(food.y, food.x, '@');
        attroff(COLOR_PAIR(2));

        // Отрисовка змейки
        attron(COLOR_PAIR(1));
        for (size_t i = 0; i < snake.size(); i++) {
            char symbol = (i == 0) ? 'O' : 'o'; // Голова 'O', тело 'o'
            mvaddch(snake[i].y, snake[i].x, symbol);
        }
        attroff(COLOR_PAIR(1));

        // Отрисовка интерфейса
        mvprintw(height + 1, 0, "Score: %d | Controls: WASD / Arrows | Exit: X", score);

        refresh(); // Вывод буфера на экран
    }

    // Главный игровой цикл
    void Run() {
        // Настройка ncurses
        initscr();
        cbreak();             // Отключаем буферизацию строк
        noecho();             // Не выводить нажатые клавиши на экран
        curs_set(0);          // Скрываем курсор
        keypad(stdscr, TRUE); // Включаем поддержку стрелок
        timeout(delayMs);     // Задержка ожидания ввода (задает FPS игры)

        // Инициализация цветов
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_GREEN, COLOR_BLACK); // Змейка
            init_pair(2, COLOR_RED, COLOR_BLACK);   // Еда
        }

        while (!gameOver) {
            Draw();
            Input();
            Logic();
        }

        // Экран окончания игры
        timeout(-1); // Ждем нажатия любой клавиши бесконечно
        mvprintw(height / 2, width / 2 - 5, "GAME OVER!");
        mvprintw(height / 2 + 1, width / 2 - 8, "Final Score: %d", score);
        mvprintw(height / 2 + 2, width / 2 - 11, "Press any key to exit");
        refresh();
        getch();

        endwin(); // Корректное завершение работы ncurses
    }
};

int main() {
    SnakeGame game;
    game.Run();
    return 0;
}