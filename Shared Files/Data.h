#pragma once
#include <cstdint>
#include <SFML/Network/Packet.hpp>

enum class eCheckPoints
{
	e_One,
	e_Two,
	e_Three,
	e_Four,
	e_Five,
	e_Six,
	e_None
};

inline const char* to_string(eCheckPoints e)
{
	switch (e)
	{
	case eCheckPoints::e_One: return "e_One";
	case eCheckPoints::e_Two: return "e_Two";
	case eCheckPoints::e_Three: return "e_Three";
	case eCheckPoints::e_Four: return "e_Four";
	case eCheckPoints::e_Five: return "e_Five";
	case eCheckPoints::e_Six: return "e_Six";
	case eCheckPoints::e_None: return "e_End";
	default: return "unknown";
	}
}


enum class eDataPacketType : uint8_t
{
	e_None,
	e_FirstConnection,
	// Every user must have a unique username as it's how the sockets are stored
	// in the map
	e_UserNameConfirmation,
	e_UserNameRejection,
	e_NewClient,
	e_ClientDisconnected,
	// Used to notify clients if the server is full
	e_MaxPlayers,
	e_StartGame,
	e_UpdatePosition,
	e_CollisionData,
	e_CheckPointPassed,
	e_LapCompleted
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
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0),
		m_checkPointPassed(eCheckPoints::e_None)
	{
	}

	DataPacket(const eDataPacketType type, const std::string& userName, const float x = 0.f, const float y = 0.f, const float angle = 0.f, const sf::Color& colour = sf::Color::Red) :
		m_type(type),
		m_userName(userName),
		m_x(x),
		m_y(y),
		m_angle(angle),
		m_red(static_cast<uint8_t>(colour.r)),
		m_green(static_cast<uint8_t>(colour.g)),
		m_blue(static_cast<uint8_t>(colour.b)),
		m_checkPointPassed(eCheckPoints::e_None)
	{
	}

	DataPacket(const eDataPacketType type, const std::string& userName, const sf::Color& colour) :
		m_type(type),
		m_userName(userName),
		m_x(0.f),
		m_y(0.f),
		m_angle(0.f),
		m_red(static_cast<uint8_t>(colour.r)),
		m_green(static_cast<uint8_t>(colour.g)),
		m_blue(static_cast<uint8_t>(colour.b)),
		m_checkPointPassed(eCheckPoints::e_None)
	{
	}

	DataPacket(const eDataPacketType type, const std::string& username, const sf::Vector2f& position, const std::string& playerCollidedWith) :
		m_type(type),
		m_userName(username),
		m_x(position.x),
		m_y(position.y),
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0),
		m_playerCollidedWith(playerCollidedWith),
		m_checkPointPassed(eCheckPoints::e_None)
	{
	}

	DataPacket(const eDataPacketType type, const std::string& username, eCheckPoints checkPointPassed) :
		m_type(type),
		m_userName(username),
		m_x(0.f),
		m_y(0.f),
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0),
		m_checkPointPassed(checkPointPassed)
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
	std::string m_playerCollidedWith;
	eCheckPoints m_checkPointPassed;

};

inline sf::Packet operator<<(sf::Packet& packet, const DataPacket& dp)
{
	return packet << dp.m_type << dp.m_userName << dp.m_x << dp.m_y << dp.m_angle << dp.m_red << dp.m_green << dp.m_blue << dp.m_playerCollidedWith << dp.m_checkPointPassed;
}

inline sf::Packet operator>>(sf::Packet& packet, DataPacket& dp)
{
	return packet >> dp.m_type >> dp.m_userName >> dp.m_x >> dp.m_y >> dp.m_angle >> dp.m_red >> dp.m_green >> dp.m_blue >> dp.m_playerCollidedWith >> dp.m_checkPointPassed;
}
