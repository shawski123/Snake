#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <random>

enum PlayerBody {
    head = 0
};

enum Directions {
    left = 0,
    right = 1,
    up = 2,
    down = 3
};

struct Player {
    static constexpr float speed = 250.f;
    bool directions[4] = { false, false, false, false };
};

void characterMovement(sf::Vector2f& movement, float speed, float deltaTime, std::vector<sf::Sprite>& character, sf::Vector2f& currentDirection, Player& moveDirec);
sf::Sprite createPlayeSprite(sf::Texture& texture, const sf::Vector2f position);
float getRandX(const sf::RenderWindow& window);
float getRandY(const sf::RenderWindow& window);
sf::FloatRect getMouthHitbox(const sf::Sprite& head, const sf::Vector2f& direction);
void resetGame(std::vector<sf::Sprite>& character, Player& moveDirec, sf::Sprite& entity, sf::Vector2f& currentDirection, int& count, bool& playerAlive, const sf::Texture& texture, const sf::RenderWindow& window);

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
    std::vector<sf::Sprite> character;
    std::vector<sf::Vector2f> positionHistory;
    Player moveDirec;

    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Snake Game", sf::Style::Titlebar | sf::Style::Close, sf::State::Windowed, settings);
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
    
    sf::Texture appleTexture;
    if (appleTexture.loadFromFile("resources/character/apple_alt_32-export.png")) {
        std::cout << "Texture loaded successfully!" << std::endl;
    }
    else {
        return -1;
    }

    //Font
    sf::Font font("resources/arial-font/arial.ttf");
    sf::Text score(font);
    score.Bold;
    score.setCharacterSize(24);
    score.setFillColor(sf::Color::White);

    //Apple
    sf::Sprite entity(appleTexture);
    entity.setPosition({ 200, window.getSize().y / 2.f });

    //Snake head
    sf::Vector2f playerDefaultPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
    sf::Sprite player = createPlayeSprite(texture, playerDefaultPosition);
    player.setTextureRect(sf::IntRect({ 0,0 }, { 42,42 }));
    player.setOrigin({ player.getLocalBounds().size.x / 2.f, player.getLocalBounds().size.y / 2.f });
    character.push_back(player);

    //Start with 1 body
    sf::Sprite newSegment(texture);
    newSegment.setTextureRect(sf::IntRect({ 84,84 }, { 42,42 }));
    newSegment.setOrigin({ newSegment.getLocalBounds().size.x / 2.f, newSegment.getLocalBounds().size.y });
    newSegment.setPosition(character.back().getPosition()); // Initial attach
    character.push_back(newSegment);
    sf::FloatRect boundBoxBody = newSegment.getGlobalBounds();
    //-----------------------------LOAD-----------------------------

    //GAME LOOP
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

        }

        std::vector<sf::Vector2f> previousPositions;
        for (const auto& segment : character) {
            previousPositions.push_back(segment.getPosition());
        }

        score.setString("Score: " + std::to_string(count) + "\nClick enter to reset");

        sf::Vector2f movement(0.f, 0.f);
        characterMovement(movement, Player::speed, deltaTime, character, currentDirection, moveDirec);

        for (size_t i = 1; i < character.size(); ++i) {
            character[i].setPosition(previousPositions[i - 1]);
            character[i].setRotation(character[i - 1].getRotation());
        }

        positionHistory.push_back(character[head].getPosition());

        // Limit position history to avoid infinite growth
        const std::size_t delayPerSegment = 10.5;
        const std::size_t maxHistory = delayPerSegment * character.size();
        if (positionHistory.size() > maxHistory) {
            positionHistory.erase(positionHistory.begin(), positionHistory.begin() + (positionHistory.size() - maxHistory));
        }

        // Move body parts using position history
        for (std::size_t i = 1; i < character.size(); ++i) {
            std::size_t index = positionHistory.size() - i * delayPerSegment;
            if (index < positionHistory.size()) {
                character[i].setPosition(positionHistory[index]);
            }
        }

        sf::FloatRect boundingBox = getMouthHitbox(character[head], currentDirection);
;
        sf::FloatRect apple = entity.getGlobalBounds();

        // Collision with apple
        if (const std::optional intersection = boundingBox.findIntersection(apple)) {
            sf::Vector2f randPos{ getRandX(window), getRandY(window) };
            entity.setPosition(randPos);

            sf::Sprite newSegment(texture);
            newSegment.setTextureRect(sf::IntRect({ 84,84 }, { 42,42 }));
            newSegment.setOrigin({ newSegment.getLocalBounds().size.x / 2.f, newSegment.getLocalBounds().size.y });
            newSegment.setPosition(character.back().getPosition()); // Initial attach
            character.push_back(newSegment);
            sf::FloatRect boundBoxBody = newSegment.getGlobalBounds();

            count++;
            std::cout << "Collision - segments: " << character.size() << "\n";
        }

        for (std::size_t i = 4; i < character.size(); ++i) {
            sf::FloatRect bodyPartBounds = character[i].getGlobalBounds();
            if (const std::optional intersection = boundingBox.findIntersection(bodyPartBounds)) {
                playerAlive = false;
                std::cout << "Player dead!\n";
                break;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
            resetGame(character, moveDirec, entity, currentDirection, count, playerAlive, texture, window);
        }

        // Draw
        window.clear();
        if (playerAlive == true) {
            for (const auto& part : character)
                window.draw(part);
        }
        window.draw(entity);
        window.draw(score);
        window.display();
    }
}

void characterMovement(sf::Vector2f& movement, float speed, float deltaTime, std::vector<sf::Sprite>& character, sf::Vector2f& currentDirection, Player& moveDirec) {

    if (!moveDirec.directions[right] && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        currentDirection = { -1.f, 0.f };
        character[head].setRotation(sf::degrees(90));
        moveDirec.directions[left] = true;
        moveDirec.directions[up] = false;
        moveDirec.directions[down] = false;
    }
    else if(!moveDirec.directions[left] && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        currentDirection = { 1.f, 0.f };
        character[head].setRotation(sf::degrees(-90));
        moveDirec.directions[right] = true;
        moveDirec.directions[up] = false;
        moveDirec.directions[down] = false;
    }
    else if (!moveDirec.directions[down] && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        currentDirection = { 0.f, -1.f };
        character[head].setRotation(sf::degrees(180));
        moveDirec.directions[up] = true;
        moveDirec.directions[left] = false;
        moveDirec.directions[right] = false;
    }
    else if (!moveDirec.directions[up] && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        currentDirection = { 0.f, 1.f };
        character[head].setRotation(sf::degrees(0));
        moveDirec.directions[down] = true;
        moveDirec.directions[left] = false;
        moveDirec.directions[right] = false;
    }

    movement = currentDirection * speed;
    character[head].move(movement * deltaTime);
}

sf::Sprite createPlayeSprite(sf::Texture& texture, const sf::Vector2f position) {
    sf::Sprite sprite(texture);
    sprite.setPosition(position);
    sprite.setScale({ 1.f,1.f });
    return sprite;
}

float getRandX(const sf::RenderWindow& window) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.f, static_cast<float>(window.getSize().x - 42));
    return dist(rng);
}

float getRandY(const sf::RenderWindow& window) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.f, static_cast<float>(window.getSize().y - 42));
    return dist(rng);
}

sf::FloatRect getMouthHitbox(const sf::Sprite& head, const sf::Vector2f& direction) {
    const float mouthWidth = 8.f;
    const float mouthHeight = 8.f;

    sf::Vector2f center = head.getPosition();

    sf::Vector2f mouthOffset;

    // Offset the hitbox to the tip of the mouth depending on direction
    if (direction.x == -1.f) {          // Left
        mouthOffset = { -21.f, 0.f };
    }
    else if (direction.x == 1.f) {    // Right
        mouthOffset = { 21.f, 0.f };
    }
    else if (direction.y == -1.f) {   // Up
        mouthOffset = { 0.f, -21.f };
    }
    else if (direction.y == 1.f) {    // Down
        mouthOffset = { 0.f, 21.f };
    }

    sf::Vector2f topLeft = center + mouthOffset - sf::Vector2f(mouthWidth / 2.f, mouthHeight / 2.f);
    return sf::FloatRect(topLeft, { mouthWidth, mouthHeight });
}

void resetGame(std::vector<sf::Sprite>& character, Player& moveDirec, sf::Sprite& entity, sf::Vector2f& currentDirection, int& count, bool& playerAlive, const sf::Texture& texture, const sf::RenderWindow& window) {
    character.clear();

    // Reset player head
    sf::Vector2f playerDefaultPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
    sf::Sprite player = createPlayeSprite(const_cast<sf::Texture&>(texture), playerDefaultPosition);
    player.setTextureRect(sf::IntRect({ 0,0 }, { 42,42 }));
    player.setOrigin({ player.getLocalBounds().size.x / 2.f, player.getLocalBounds().size.y / 2.f });
    character.push_back(player);

    // Reset body segment (1 segment)
    sf::Sprite newSegment(const_cast<sf::Texture&>(texture));
    newSegment.setTextureRect(sf::IntRect({ 84,84 }, { 42,42 }));
    newSegment.setOrigin({ newSegment.getLocalBounds().size.x / 2.f, newSegment.getLocalBounds().size.y });
    newSegment.setPosition(character.back().getPosition()); // attach to head
    character.push_back(newSegment);

    // Reset apple position
    entity.setPosition({ 200, window.getSize().y / 2.f });

    // Reset movement
    currentDirection = { 0.f, 0.f };
    for (int i = 0; i < 4; i++) {
        moveDirec.directions[i] = false;
    }

    // Reset score and player alive
    count = 0;
    playerAlive = true;
}