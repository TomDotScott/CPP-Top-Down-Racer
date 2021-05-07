#pragma once
#include "Player.h"

class Background
{
public:
	Background();
	void CheckCollisions(Player& player);
	void Render(sf::RenderWindow& window);

private:
	sf::Image m_image;
	sf::Texture m_texture;
};
