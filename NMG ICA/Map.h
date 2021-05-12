#pragma once
#include "Player.h"

/**
 * \brief The map game object is the racetrack itself. It can detect if a player
 * is on the track or the grass and set their speed accordingly
 */
class Map
{
public:
	Map();

	/**
	 * \brief Checks a player's collision with the map and sets their speed accordingly
	 * \param player The player's collision to check
	 */
	void CheckCollisions(Player& player) const;

	/**
	 * \brief Renders the map to the screen
	 * \param window The RenderWindow to draw the map to
	 */
	void Render(sf::RenderWindow& window) const;

private:
	// Loads an image file from the images folder
	sf::Image m_image; 

	// A wrapper for the image, allows it to be drawn onscreen
	sf::Texture m_texture; 
};
