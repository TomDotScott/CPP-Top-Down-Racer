#include "Map.h"

#include <iostream>

#include "Globals.h"

Map::Map()
{
	// Load the image from the images folder
	if (!m_image.loadFromFile("images/map.png"))
	{
		std::cout << "Unable to load the background image!" << std::endl;
	}

	// Set the texture to be the data stored in the image
	m_texture.loadFromImage(m_image);

	const sf::Vector2f checkPointColliderSize{ globals::game::k_checkPointWidth, globals::game::k_checkPointHeight };
}

void Map::CheckCollisions(Player& player) const
{
	// See if the player has gone off the track

	const sf::Vector2f playerPosition = player.GetPosition();

	const auto playerPixel = m_image.getPixel(
		static_cast<unsigned>(playerPosition.x),
		static_cast<unsigned>(playerPosition.y)
	);

	// Work out the colour of the pixel that the player is on top of and set their speed accordingly
	if (playerPixel == sf::Color::Black || playerPixel == sf::Color::White)
	{
		player.SetSpeed(globals::cars::k_carTrackSpeed);
	} else
	{
		player.SetSpeed(globals::cars::k_carGrassSpeed);
	}
}

void Map::Render(sf::RenderWindow& window) const
{
	const sf::Sprite sprite(m_texture);
	window.draw(sprite);
}

