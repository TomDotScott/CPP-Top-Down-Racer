#include "Background.h"

#include <iostream>

Background::Background()
{
	if(!m_image.loadFromFile("images/map.png"))
	{
		std::cout << "Unable to load the background image!" << std::endl;
	}

	m_texture.loadFromImage(m_image);
	
}

void Background::CheckCollisions(Player& player)
{
	
}

void Background::Render(sf::RenderWindow& window)
{
	const sf::Sprite sprite(m_texture);
	window.draw(sprite);
}

