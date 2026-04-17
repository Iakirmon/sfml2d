#pragma once

#include <SFML/Graphics.hpp>
#include "Animator.h"

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
    void resetState();

    bool justJumped() const { return justJumped_; }

private:
    sf::Texture  texture_;
    sf::Sprite   sprite_;
    sf::Vector2u texSize_;
    Animator     animator_;

    sf::Vector2f velocity_{0.f, 0.f};
    bool isOnGround_{false};
    bool canDoubleJump_{false};
    bool canTripleJump_{false};
    bool spaceWasPressed_{false};
    bool facingRight_{true};
    bool justJumped_{false};
    int lives_{3};
    sf::Vector2f spawnPosition_{100.f, 300.f};
};
