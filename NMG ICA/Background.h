#pragma once
#include "Player.h"

class Background
{
public:
	Background();
	void CheckCollisions(Player& player) const;
	void Render(sf::RenderWindow& window) const;

private:
	sf::Image m_image;
	sf::Texture m_texture;
};
