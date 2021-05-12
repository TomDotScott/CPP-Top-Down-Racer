#pragma once
#include <SFML/Graphics.hpp>
#include "Globals.h"

/**
 * \brief The types of data sent via the TcpPacket struct
 */
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
	e_LapCompleted,
	e_Overtaken,
	e_RaceCompleted,
	e_GameOver
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

/**
 * \brief A holder for the final positions of the racers. The first element is first place,
 * the last element came last in the race
 */
struct PlacementOrder
{
	PlacementOrder()
	{
		m_racePositions.resize(globals::game::k_playerAmount);
	}
	
	PlacementOrder(const std::vector<std::string>& v) : m_racePositions(v){};
	
	std::vector<std::string> m_racePositions;
};

inline sf::Packet operator <<(sf::Packet& packet, const PlacementOrder& po)
{
	for (const auto& username : po.m_racePositions)
	{
		packet << username;
	}
	return packet;
}

inline sf::Packet operator >>(sf::Packet& packet, PlacementOrder& po)
{
	for (auto& username : po.m_racePositions)
	{
		packet >> username;
	}
	return packet;
}

/**
 * \brief Common struct between the Server and the Client, the TcpDataPacket is used as a container
 * for all of the data sent in the game
 */
struct TcpDataPacket
{
	TcpDataPacket() :
		m_type(eDataPacketType::e_None),
		m_x(0.f),
		m_y(0.f),
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0),
		m_positionInRace(0)
	{
	}

	TcpDataPacket(const eDataPacketType type, const std::string& userName, const float x = 0.f, const float y = 0.f, const float angle = 0.f, const sf::Color& colour = sf::Color::Red) :
		m_type(type),
		m_userName(userName),
		m_x(x),
		m_y(y),
		m_angle(angle),
		m_red(static_cast<uint8_t>(colour.r)),
		m_green(static_cast<uint8_t>(colour.g)),
		m_blue(static_cast<uint8_t>(colour.b)),
		m_positionInRace(0)
	{
	}

	TcpDataPacket(const eDataPacketType type, const std::string& userName, const sf::Color& colour) :
		m_type(type),
		m_userName(userName),
		m_x(0.f),
		m_y(0.f),
		m_angle(0.f),
		m_red(static_cast<uint8_t>(colour.r)),
		m_green(static_cast<uint8_t>(colour.g)),
		m_blue(static_cast<uint8_t>(colour.b)),
		m_positionInRace(0)
	{
	}

	TcpDataPacket(const eDataPacketType type, const std::string& username, const sf::Vector2f& position, const std::string& playerCollidedWith) :
		m_type(type),
		m_userName(username),
		m_x(position.x),
		m_y(position.y),
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0),
		m_positionInRace(0),
		m_playerCollidedWith(playerCollidedWith)
	{
	}

	TcpDataPacket(const eDataPacketType type, const std::string& username, const int positionInRace) :
		m_type(type),
		m_userName(username),
		m_x(0.f),
		m_y(0.f),
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0),
		m_positionInRace(positionInRace)
	{
	}

	TcpDataPacket(const eDataPacketType type, const std::string& username, const std::vector<std::string>& placementOrder) :
		m_type(type),
		m_userName(username),
		m_x(0.f),
		m_y(0.f),
		m_angle(0.f),
		m_red(0),
		m_green(0),
		m_blue(0),
		m_positionInRace(0),
		m_placementOrder(placementOrder)
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
	int m_positionInRace;
	std::string m_playerCollidedWith;
	PlacementOrder m_placementOrder; // TODO: add the default constructor to the initialiser lists 
};

inline sf::Packet operator<<(sf::Packet& packet, const TcpDataPacket& dp)
{
	return packet << dp.m_type << dp.m_userName << dp.m_x << dp.m_y << dp.m_angle <<
		dp.m_red << dp.m_green << dp.m_blue << dp.m_positionInRace << dp.m_playerCollidedWith << dp.m_placementOrder;
}

inline sf::Packet operator>>(sf::Packet& packet, TcpDataPacket& dp)
{
	return packet >> dp.m_type >> dp.m_userName >> dp.m_x >> dp.m_y >> dp.m_angle >>
		dp.m_red >> dp.m_green >> dp.m_blue >> dp.m_positionInRace >> dp.m_playerCollidedWith >> dp.m_placementOrder;
}