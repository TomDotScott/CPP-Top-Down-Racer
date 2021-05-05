#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

#include "Globals.h"

class Player
{
public:
	Player() = default; // Default constructor needed to compile for the unordered_map<>'s
	Player(const sf::Texture& textureData);
	
	void Update(float deltaTime);
	void Render(sf::RenderWindow& window);

	sf::Vector2f GetPosition() const;
	void SetPosition(const sf::Vector2f& position);

	sf::Color GetColour() const;
	void SetColour(const sf::Color& colour);
	
	void ChangeVelocity(const float dx, const float dy);

	float GetAngle() const;
	void  SetAngle(const float angle);
	void  ChangeAngle(const float deltaAngle);

private:
	sf::Sprite m_sprite;
	
	sf::Vector2f m_position;
	sf::Vector2f m_velocity;
	
	float m_angle;
};
