#pragma once
#include <array>
#include "Player.h"
#include "../Shared Files/Data.h"

class Map
{
public:
	Map();

	void CheckCollisions(Player& player) const;
	
	eCheckPoints CheckPlayerCheckPoints(Player& player) const;

	void Render(sf::RenderWindow& window) const;

private:
	sf::Image m_image;
	sf::Texture m_texture;

	std::unordered_map<eCheckPoints, sf::FloatRect> m_levelCheckpoints;
};
