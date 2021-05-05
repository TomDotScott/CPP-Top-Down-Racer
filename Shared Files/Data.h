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
	e_MaxPlayers,
	// Used to notify clients if the server is full
	e_UpdatePosition
};

inline const char* to_string(eDataPacketType e)
{
	switch (e)
	{
		case eDataPacketType::e_None: return "e_None";
		case eDataPacketType::e_FirstConnection: return "e_FirstConnection";
		case eDataPacketType::e_UserNameConfirmation: return "e_UserNameConfirmation";
		case eDataPacketType::e_UserNameRejection: return "e_UserNameRejection";
		case eDataPacketType::e_NewClient: return "e_NewClient";
		case eDataPacketType::e_MaxPlayers: return "e_MaxPlayers";
		case eDataPacketType::e_UpdatePosition: return "e_UpdatePosition";
		default: return "unknown";
	}
}

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
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0)
	{
	}

	DataPacket(const eDataPacketType type, std::string userName, const sf::Color& colour = sf::Color::Red, const float x = 0.f, const float y = 0.f, const float angle = 0.f) :
		m_type(type),
		m_userName(std::move(userName)),
		m_x(x),
		m_y(y),
		m_angle(angle),
		m_red(static_cast<uint8_t>(colour.r)),
		m_green(static_cast<uint8_t>(colour.g)),
		m_blue(static_cast<uint8_t>(colour.b))
	{
	}

	eDataPacketType m_type;
	std::string m_userName;
	float m_x;
	float m_y;
	float m_angle;
	uint8_t m_red;
	uint8_t m_green;
	uint8_t m_blue;
};

inline sf::Packet operator<<(sf::Packet& packet, const DataPacket& dp)
{
	return packet << dp.m_type << dp.m_userName << dp.m_x << dp.m_y << dp.m_angle << dp.m_red << dp.m_green << dp.m_blue;
}

inline sf::Packet operator>>(sf::Packet& packet, DataPacket& dp)
{
	return packet >> dp.m_type >> dp.m_userName >> dp.m_x >> dp.m_y >> dp.m_angle >> dp.m_red >> dp.m_green >> dp.m_blue;
}