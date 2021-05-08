#include "Map.h"

#include <iostream>

Map::Map()
{
	if (!m_image.loadFromFile("images/map.png"))
	{
		std::cout << "Unable to load the background image!" << std::endl;
	}

	m_texture.loadFromImage(m_image);

	const sf::Vector2f checkPointColliderSize{ globals::k_checkPointWidth, globals::k_checkPointHeight };

	m_levelCheckpoints.insert({ eCheckPoints::e_One, sf::FloatRect({ 803.f, 523.f }, checkPointColliderSize) });
	m_levelCheckpoints.insert({ eCheckPoints::e_Two, sf::FloatRect({514.f, 11.f}, checkPointColliderSize) });
	m_levelCheckpoints.insert({ eCheckPoints::e_Three, sf::FloatRect({ 382.f, 313.f }, checkPointColliderSize) });
	m_levelCheckpoints.insert({ eCheckPoints::e_Four, sf::FloatRect({ 167.f, 37.f }, checkPointColliderSize) });
	m_levelCheckpoints.insert({ eCheckPoints::e_Five, sf::FloatRect({ 132.f, 597.f }, checkPointColliderSize) });
	m_levelCheckpoints.insert({ eCheckPoints::e_Six, sf::FloatRect({ 434.f, 489.f }, checkPointColliderSize) });
}

void Map::CheckCollisions(Player& player) const
{
	const sf::Vector2f playerPosition = player.GetPosition();

	const auto playerPixel = m_image.getPixel(
		static_cast<unsigned>(playerPosition.x),
		static_cast<unsigned>(playerPosition.y)
	);

	if (playerPixel == sf::Color::Black || playerPixel == sf::Color::White)
	{
		player.SetSpeed(globals::k_carTrackSpeed);
	} else
	{
		player.SetSpeed(globals::k_carGrassSpeed);
	}
}

eCheckPoints Map::CheckPlayerCheckPoints(Player& player) const
{
	// See if the player has overlapped the checkpoints
	for (const auto& checkpoint : m_levelCheckpoints)
	{
		if (checkpoint.second.intersects(player.GetGlobalBounds()))
		{
			return checkpoint.first;
		}
	}

	return eCheckPoints::e_None;
}

void Map::Render(sf::RenderWindow& window) const
{
	const sf::Sprite sprite(m_texture);
	window.draw(sprite);

	for (const auto& checkpoint : m_levelCheckpoints)
	{
		sf::RectangleShape rect({ checkpoint.second.width, checkpoint.second.height });
		rect.setFillColor(sf::Color::Red);
		rect.setPosition({ checkpoint.second.left, checkpoint.second.top });
		window.draw(rect);
	}
}

