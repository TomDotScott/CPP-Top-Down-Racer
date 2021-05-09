#pragma once
#include <array>
#include "Player.h"
#include "../Shared Files/Data.h"

// TODO: Move map to the client class - it's not needed as a separate object
class Map
{
public:
	Map();

	void CheckCollisions(Player& player) const;

	void Render(sf::RenderWindow& window) const;

private:
	sf::Image m_image;
	sf::Texture m_texture;
};
