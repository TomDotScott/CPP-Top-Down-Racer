#pragma once
#include <SFML/Graphics.hpp>

/**
 * \brief The Player class holds all the information needed to control and draw the race cars for the game
 */
class Player
{
public:
	Player(); // Default constructor needed to compile for the unordered_map<>'s
	Player(const sf::Texture& textureData);
	
	/**
	 * \brief Updates the player object
	 * \param deltaTime The time difference between frames for frame-rate independence
	 */
	void Update(float deltaTime);

	/**
	 * \brief Renders the player to the screen
	 * \param window The RenderWindow to draw the map to
	 */
	void Render(sf::RenderWindow& window);

	/**
	 * \return The current position of the player
	 */
	sf::Vector2f GetPosition() const;
	
	/**
	 * \param position The new position of the player
	 */
	void SetPosition(const sf::Vector2f& position);

	/**
	 * \return The colour of the player
	 */
	sf::Color GetColour() const;
	
	/**
	 * \param colour The new colour to set the player
	 */
	void SetColour(const sf::Color& colour);

	
	/**
	 * \brief Changes the velocity of the player
	 * \param dx The difference in the x axis for the velocity
	 * \param dy The difference in the y axis for the velocity
	 */
	void ChangeVelocity(const float dx, const float dy);

	/**
	 * \return The current angle of the player
	 */
	float GetAngle() const;
	
	/**
	 * \param angle The new angle, in Radians, to set the player
	 */
	void SetAngle(const float angle);
	
	/**
	 * \param deltaAngle The change in angle, in Radians, of the player
	 */
	void ChangeAngle(const float deltaAngle);

	/**
	 * \return The current speed of the player
	 */
	float GetSpeed() const;

	/**
	 * \param speed The new speed of the player
	 */
	void SetSpeed(const float speed);

private:
	// The sprite used to draw the player
	sf::Sprite m_sprite;

	// The current speed of the player
	float m_speed;

	// The current position of the player
	sf::Vector2f m_position;

	// The current velocity of the player
	sf::Vector2f m_velocity;

	// The current angle of the player
	float m_angle;
};
