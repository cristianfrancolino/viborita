#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#define WIDTH 450
#define HEIGHT 420
#define TILE_SIZE 15

enum directions { left, right, up, down };

sf::Vector2f generateApplePosition(std::vector<sf::Vector2i> snake);
bool existsInSnake(std::vector<sf::Vector2i> snake, int x, int y);
void showStartScreen(sf::RenderWindow *window, int *config);
void dibujarTextoDelMenu(sf::RenderWindow *window, int *config);
void dibujarScore(sf::RenderWindow *window, int score, sf::Font font) ;

int main() {
    // Configuración
    std::ifstream configFile("game.config");
    std::string configString;
    int config;
    getline(configFile, configString);
    config = stoi(configString);
    configFile.close();

    // Inicio
    sf::Font font;
    font.loadFromFile("ZenLoop-Regular.ttf");
    const int tiles_x = WIDTH / TILE_SIZE;
    const int tiles_y = HEIGHT / TILE_SIZE;
    sf::CircleShape apple(7.5f);
    apple.setFillColor(sf::Color::Red);
    apple.setPosition((rand() % tiles_x) * TILE_SIZE, (rand() % tiles_y) * TILE_SIZE);
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Viborita");
    // Creo víbora
    std::vector<sf::Vector2i> snake;
    snake.push_back(sf::Vector2i(5, 5));
    // snake.push_back(sf::Vector2i(4, 5));
    // snake.push_back(sf::Vector2i(3, 5));
    int grow = 2;
    // Seteo dirección inicial
    int direction = right;
    int lastMove = direction;
    // Creo clock
    sf::Clock clock;
    // Creo sonidos
    // Sonido de comer manzana
    sf::SoundBuffer eatSoundBuffer;
    eatSoundBuffer.loadFromFile("eat.ogg");
    sf::Sound eatSound;
    eatSound.setBuffer(eatSoundBuffer);
    // Sonido de muerte
    sf::SoundBuffer dieSoundBuffer;
    dieSoundBuffer.loadFromFile("die.ogg");
    sf::Sound dieSound;
    dieSound.setBuffer(dieSoundBuffer);
    // Música de fondo
    sf::Music backgroundMusic;
    backgroundMusic.openFromFile("background.ogg");
    backgroundMusic.setLoop(true);
    backgroundMusic.play();

    showStartScreen(&window, &config);

    // Teclas
    sf::Keyboard::Key upKey = sf::Keyboard::Key::Up;
    sf::Keyboard::Key downKey = sf::Keyboard::Key::Down;
    sf::Keyboard::Key leftKey = sf::Keyboard::Key::Left;
    sf::Keyboard::Key rightKey = sf::Keyboard::Key::Right;
    if (config == 1) {
        upKey = sf::Keyboard::Key::W;
        downKey = sf::Keyboard::Key::S;
        leftKey = sf::Keyboard::Key::A;
        rightKey = sf::Keyboard::Key::D;
    }

    while (window.isOpen()) {
        sf::Event event;
        float elapsedTime = clock.getElapsedTime().asMilliseconds();

        // Events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == rightKey && lastMove != left) {
                    direction = right;
                }
                if (event.key.code == leftKey && lastMove != right) {
                    direction = left;
                }
                if (event.key.code == upKey && lastMove != down) {
                    direction = up;
                }
                if (event.key.code == downKey && lastMove != up) {
                    direction = down;
                }
            }
        }

        // Move snake
        if (elapsedTime >= 100 - snake.size()) {
            clock.restart();
            sf::Vector2i newPos;
            switch (direction) {
                case left:
                    newPos.x = snake[0].x - 1;
                    newPos.y = snake[0].y;
                    break;
                case right:
                    newPos.x = snake[0].x + 1;
                    newPos.y = snake[0].y;
                    break;
                case up:
                    newPos.x = snake[0].x;
                    newPos.y = snake[0].y - 1;
                    break;
                case down:
                    newPos.x = snake[0].x;
                    newPos.y = snake[0].y + 1;
                    break;
            }
            // Reubico la nueva posición de la cabeza si se fue del mapa
            if (newPos.x >= tiles_x) {
                newPos.x = 0;
            }
            if (newPos.x < 0) {
                newPos.x = tiles_x - 1;
            }
            if (newPos.y >= tiles_y) {
                newPos.y = 0;
            }
            if (newPos.y < 0) {
                newPos.y = tiles_y - 1;
            }
            // Verifico si la cabeza chocó con el cuerpo
            bool perdi = false;
            for (int i = 0; i < snake.size(); i++) {
                if (snake[i].x == newPos.x && snake[i].y == newPos.y) {
                    // Perdí
                    perdi = true;
                    dieSound.play();
                    snake.clear();
                    snake.push_back(sf::Vector2i(5, 5));
                    direction = right;
                    lastMove = direction;
                    grow = 3;
                    apple.setPosition(generateApplePosition(snake));
                    window.clear();
                    showStartScreen(&window, &config);
                }
            }
            if (!perdi) {
                snake.insert(snake.begin(), newPos);
            }
            // Verifico si la manzana fue comida
            if (newPos.x == apple.getPosition().x / TILE_SIZE && newPos.y == apple.getPosition().y / TILE_SIZE) {
                eatSound.play();
                grow += 2;
                apple.setPosition(generateApplePosition(snake));
            }
            if (grow > 0) {
                grow--;
            } else {
                snake.pop_back();
            }
            lastMove = direction;
        }

        // Render
        window.clear();
        window.draw(apple);
        // Draw snake
        for (int i = 0; i < snake.size(); i++) {
            sf::RectangleShape snakePart(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            snakePart.setPosition(snake[i].x * TILE_SIZE, snake[i].y * TILE_SIZE);
            snakePart.setFillColor(sf::Color::Green);
            window.draw(snakePart);
        }
        dibujarScore(&window, snake.size() + grow, font);
        window.display();
    }

    return 0;
}

sf::Vector2f generateApplePosition(std::vector<sf::Vector2i> snake) {
    const int tiles_x = WIDTH / TILE_SIZE;
    const int tiles_y = HEIGHT / TILE_SIZE;
    int x = rand() % tiles_x;
    int y = rand() % tiles_y;
    while (existsInSnake(snake, x, y)) {
        x = rand() % tiles_x;
        y = rand() % tiles_y;
    }

    return sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
}

bool existsInSnake(std::vector<sf::Vector2i> snake, int x, int y) {
    for (int i = 0; i < snake.size(); i++)
        if (snake[i].x == x && snake[i].y == y)
            return true;
    return false;
}

void showStartScreen(sf::RenderWindow *window, int *config) {
    bool wait = true;
    dibujarTextoDelMenu(window, config);
    
    while (wait) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window->close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Key::Enter) {
                    wait = false;
                }
                if (event.key.code == sf::Keyboard::Key::Space) {
                    if (*config == 0) {
                        *config = 1;
                    } else {
                        *config = 0;
                    }
                }
            }
        }
        dibujarTextoDelMenu(window, config);
    }

    // Antes de empezar el juego, guardo la configuración seleccionada en el archivo de config
    std::ofstream configFile("game.config");
    configFile << *config;
    configFile.close();
}

void dibujarTextoDelMenu(sf::RenderWindow *window, int *config) {
    sf::Font font;
    font.loadFromFile("ZenLoop-Regular.ttf");
    // font.loadFromFile("Qahiri-Regular.ttf");
    sf::Text text;
    text.setFont(font);
    text.setString("Hello viborita!");
    text.setCharacterSize(68);
    text.setFillColor(sf::Color::Green);
    text.setPosition(WIDTH / 2 - text.getGlobalBounds().width / 2, HEIGHT / 3);
    window->clear();
    window->draw(text);
    text.setString("Teclas: Flechitas");
    if (*config == 1) {
        text.setString("Teclas: WASD");
    }
    text.move(0, 80);
    text.setCharacterSize(32);
    window->draw(text);
    window->display();
}

void dibujarScore(sf::RenderWindow *window, int score, sf::Font font) {
    // font.loadFromFile("Qahiri-Regular.ttf");
    sf::Text text;
    text.setFont(font);
    text.setString(std::to_string(score));
    text.setCharacterSize(34);
    text.setFillColor(sf::Color::White);
    text.setPosition(WIDTH / 2 - text.getGlobalBounds().width / 2, 10);
    window->draw(text);
}