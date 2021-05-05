#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <cmath>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Globals.h"

class Player
{
public:
	Player(const sf::Texture& texture);
	void Update(float deltaTime);
	void Render(sf::RenderWindow& window);

private:
	sf::Sprite m_sprite;
	
	sf::Vector2f m_position;
	sf::Vector2f m_acceleration;
	
	float m_angle;

	void Limit(sf::Vector2f& vector, const float maxMag) const;
};
