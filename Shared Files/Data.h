#pragma once
#include <cstdint>
#include <SFML/Network/Packet.hpp>

constexpr uint8_t NONE = 0;
constexpr uint8_t FIRST_CONNECTION = 1;
constexpr uint8_t UPDATE_POSITION = 2;


struct DataPacket
{
	DataPacket() :
		m_type(NONE),
		m_userName(""),
		m_playerNum(0),
		m_x(0.f),
		m_y(0.f)
	{
	}

	DataPacket(const uint8_t type, std::string userName, const uint8_t playerNum = 0, const float x = 0.f, const float y = 0.f) :
		m_type(type),
		m_userName(std::move(userName)),
		m_playerNum(playerNum),
		m_x(x),
		m_y(y)
	{
	}

	uint8_t m_type;
	std::string m_userName;
	uint8_t m_playerNum;
	float m_x;
	float m_y;
};

inline sf::Packet operator<<(sf::Packet& packet, const DataPacket& dp)
{
	return packet << dp.m_type << dp.m_playerNum << dp.m_userName << dp.m_x << dp.m_y;
}

inline sf::Packet operator>>(sf::Packet& packet, DataPacket& dp)
{
	return packet >> dp.m_type >> dp.m_playerNum >> dp.m_userName >> dp.m_x >> dp.m_y;
}