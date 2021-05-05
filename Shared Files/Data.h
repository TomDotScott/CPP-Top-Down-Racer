#pragma once
#include <cstdint>
#include <SFML/Network/Packet.hpp>

enum class eDataPacketType : uint8_t
{
	e_None,
	e_FirstConnection,
	// Every user must have a unique username as it's how the sockets are stored
	// in the map
	e_UserNameConfirmation,
	e_UserNameRejection,
	e_NewClient,
	e_MaxPlayers, // Used to notify clients if the server is full
	e_UpdatePosition
};

// Sending enums via sf::Packet https://en.sfml-dev.org/forums/index.php?topic=17075.0
template<typename T>
inline typename std::enable_if<std::is_enum<T>::value, sf::Packet&>::type
operator<<(sf::Packet& roPacket, const T& rkeMsgType)
{
	return roPacket << static_cast<typename std::underlying_type<T>::type>(rkeMsgType);
}

template<typename T>
inline typename std::enable_if<std::is_enum<T>::value, sf::Packet&>::type
operator>>(sf::Packet& roPacket, T& reMsgType)
{
	typename std::underlying_type<T>::type xMsgType{};
	roPacket >> xMsgType;
	reMsgType = static_cast<T>(xMsgType);

	return roPacket;
}

struct DataPacket
{
	DataPacket() :
		m_type(eDataPacketType::e_None),
		m_x(0.f),
		m_y(0.f),
		m_angle(0.f)
	{
	}

	DataPacket(const eDataPacketType type, std::string userName, const float x = 0.f, const float y = 0.f, const float angle = 0.f) :
		m_type(type),
		m_userName(std::move(userName)),
		m_x(x),
		m_y(y),
		m_angle(angle)
	{
	}

	eDataPacketType m_type;
	std::string m_userName;
	float m_x;
	float m_y;
	float m_angle;
};

inline sf::Packet operator<<(sf::Packet& packet, const DataPacket& dp)
{
	return packet << dp.m_type << dp.m_userName << dp.m_x << dp.m_y << dp.m_angle;
}

inline sf::Packet operator>>(sf::Packet& packet, DataPacket& dp)
{
	return packet >> dp.m_type >> dp.m_userName >> dp.m_x >> dp.m_y >> dp.m_angle;
}