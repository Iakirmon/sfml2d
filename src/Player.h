#pragma once

#include <SFML/Graphics.hpp>

class Player {
public:
    Player();

    void handleInput();
    void update(float dt);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getHitbox() const;

    void moveShape(sf::Vector2f offset);
    void setVelocityX(float vx);
    void setVelocityY(float vy);
    void setOnGround(bool val);

    int getLives() const;
    void respawn();

private:
    sf::RectangleShape shape_;
    sf::Vector2f velocity_{0.f, 0.f};
    bool isOnGround_{false};
    int lives_{3};
    sf::Vector2f spawnPosition_{100.f, 300.f};
};

