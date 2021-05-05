#include "Player.h"
#include <iostream>

Player::Player(const sf::Texture& textureData) :
	m_position(400.f, 300.f),
	m_velocity(0.f, 0.f),
	m_angle(0.f)
{	
	m_sprite.setTexture(textureData);
	m_sprite.setOrigin(22.f, 22.f);
}


void Player::Update(const float deltaTime)
{
	m_velocity = {
		m_velocity.x * cosf(m_angle) - m_velocity.y * sinf(m_angle),
		m_velocity.x * sinf(m_angle) + m_velocity.y * cosf(m_angle)
	};

	m_position += m_velocity;

	m_velocity = { 0.f, 0.f };
}

void Player::Render(sf::RenderWindow& window)
{
	m_sprite.setPosition(m_position);
	m_sprite.setRotation(m_angle * 180.f / 3.141593f);
	window.draw(m_sprite);
}

sf::Vector2f Player::GetPosition() const
{
	return m_position;
}

void Player::SetPosition(const sf::Vector2f& position)
{
	m_position = position;
}

sf::Color Player::GetColour() const
{
	return m_sprite.getColor();
}

void Player::SetColour(const sf::Color& colour)
{
	m_sprite.setColor(colour);
}

void Player::ChangeVelocity(const float dx, const float dy)
{
	m_velocity.x += dx;
	m_velocity.y += dy;
}

float Player::GetAngle() const
{
	return m_angle;
}

void Player::SetAngle(const float angle)
{
	m_angle = angle;
}

void Player::ChangeAngle(const float deltaAngle)
{
	m_angle += deltaAngle;
}
