#pragma once
#include <array>
#include <SFML/Graphics/Color.hpp>

namespace globals
{
	constexpr int k_playerAmount = 4;
	constexpr float k_carSpeed = 100.f;

	const std::array<sf::Color, k_playerAmount> k_carColours{
		sf::Color(255, 0, 0),
		sf::Color(0, 0, 255),
		sf::Color(0, 255, 0),
		sf::Color(255, 255, 0)
	};
}
