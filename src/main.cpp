#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <random>

struct Player {
    static constexpr float speed = 300.f;
};

void characterMovement(sf::Vector2f& movement, float speed, float deltaTime, sf::Sprite& player, sf::Vector2f& currentDirection);
sf::Sprite createPlayeSprite(sf::Texture& texture, const sf::Vector2f position);
float getRandX(const sf::RenderWindow& window);
float getRandY(const sf::RenderWindow& window);

int main()
{
    //-----------------------INITIALIZE-----------------------
    sf::Vector2f currentDirection(0.f, 0.f);
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    bool playerAlive = true;
    sf::Clock clock;
    int count = 0;
    bool appleEaten = false;
    sf::Vector2f randNums;

    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Snake Game", sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(144);
    //-----------------------INITIALIZE-----------------------

    //-----------------------------LOAD-----------------------------
    sf::Texture texture;
    if (texture.loadFromFile("resources/character/Snake sprite sheet.png")) {
        std::cout << "Texture loaded successfully!" << std::endl;
    }
    else {
        return -1;
    }

    sf::Font font("resources/arial-font/arial.ttf");
    sf::Text score(font);
    score.Bold;
    score.setCharacterSize(24);
    score.setFillColor(sf::Color::White);

    sf::CircleShape entity(40);
    entity.setFillColor(sf::Color::Red);
    entity.setPosition({200, window.getSize().y / 2.f});

    sf::Vector2f playerDefaultPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
    sf::Sprite player = createPlayeSprite(texture, playerDefaultPosition);

    player.setTextureRect(sf::IntRect({0,0} , {42,42}));
    player.setOrigin({ player.getLocalBounds().size.x / 2.f, player.getLocalBounds().size.y / 2.f });
    //-----------------------------LOAD-----------------------------

    //GAME LOOP
    while (window.isOpen())
    {
        score.setString("Score: " + std::to_string(count));

        float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

        }

        sf::Vector2f movement(0.f, 0.f);

        characterMovement(movement, Player::speed, deltaTime, player, currentDirection);

        sf::FloatRect otherBox = entity.getGlobalBounds();
        sf::FloatRect boundingBox = player.getGlobalBounds();
        
        if (const std::optional intersection = boundingBox.findIntersection(otherBox))
        {
            randNums.x = getRandX(window);
            randNums.y = getRandY(window);

            std::cout << "Collision" << std::endl;
            count++;
            entity.setPosition(randNums);
        }

        window.clear();
        window.draw(player);
        window.draw(entity);
        window.draw(score);
        window.display();
    }
}

void characterMovement(sf::Vector2f& movement, float speed, float deltaTime, sf::Sprite& player, sf::Vector2f& currentDirection) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        currentDirection = { -1.f, 0.f };
        player.setRotation(sf::degrees(90));
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        currentDirection = { 1.f, 0.f };
        player.setRotation(sf::degrees(-90));
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        currentDirection = { 0.f, -1.f };
        player.setRotation(sf::degrees(180));
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        currentDirection = { 0.f, 1.f };
        player.setRotation(sf::degrees(0));
    }

    movement = currentDirection * speed;
    player.move(movement * deltaTime);
}

sf::Sprite createPlayeSprite(sf::Texture& texture, const sf::Vector2f position) {
    sf::Sprite sprite(texture);
    sprite.setPosition(position);
    sprite.setScale({ 1.f,1.f });
    return sprite;
}

float getRandX(const sf::RenderWindow& window) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.f, static_cast<float>(window.getSize().x - 80));
    return dist(rng);
}

float getRandY(const sf::RenderWindow& window) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.f, static_cast<float>(window.getSize().y - 80));
    return dist(rng);
}