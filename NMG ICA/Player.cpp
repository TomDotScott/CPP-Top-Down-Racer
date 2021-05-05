#include "Player.h"

#include <iostream>
#include <SFML/Window/Keyboard.hpp>

Player::Player(const sf::Texture& texture) :
	m_position(400.f, 300.f),
	m_acceleration(0.f, 0.f),
	m_angle(0.f)
{
	m_sprite.setTexture(texture);
	m_sprite.setColor(globals::k_carColours[0]);
	m_sprite.setOrigin(22.f, 22.f);
}

void Player::Update(const float deltaTime)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		m_angle -= 3.14f * deltaTime;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		m_angle += 3.14f * deltaTime;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		m_acceleration.y = -globals::k_carSpeed * deltaTime;
	} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		m_acceleration.y = globals::k_carSpeed * deltaTime;
	}

	m_acceleration = {
		m_acceleration.x * cosf(m_angle) - m_acceleration.y * sinf(m_angle),
		m_acceleration.x * sinf(m_angle) + m_acceleration.y * cosf(m_angle)
	};

	m_position += m_acceleration;

	m_acceleration = { 0.f, 0.f };
}

void Player::Render(sf::RenderWindow& window)
{
	m_sprite.setPosition(m_position);
	m_sprite.setRotation(m_angle * 180.f / 3.141593f);
	window.draw(m_sprite);
}

void Player::Limit(sf::Vector2f& vector, const float maxMag) const
{
	const float magSq = vector.x * vector.x + vector.y + vector.y;
	if (magSq > maxMag * maxMag)
	{
		const float mag = sqrtf(magSq);

		if (mag > 0)
		{
			vector /= mag;
		}

		vector *= maxMag;
	}
}
