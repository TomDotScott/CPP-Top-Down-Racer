#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

#include "Globals.h"

class Player
{
public:
	Player();
	
	void Update(float deltaTime);
	void Render(sf::RenderWindow& window);

	sf::Vector2f GetPosition() const;
	void SetPosition(const sf::Vector2f& position);
	
	void ChangeVelocity(const float dx, const float dy);

	float GetAngle() const;
	void  SetAngle(const float angle);
	void  ChangeAngle(const float deltaAngle);

private:
	sf::Sprite m_sprite;
	sf::Texture m_texture;
	
	sf::Vector2f m_position;
	sf::Vector2f m_velocity;
	
	float m_angle;
};
