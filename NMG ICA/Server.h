#pragma once
#include <unordered_map>
#include <SFML/Network.hpp>
#include <SFML/Graphics/Color.hpp>
#include <array>

#include "Globals.h"
#include "../Shared Files/Data.h"

struct Client
{
	Client() :
		m_socket(nullptr),
		m_position(),
		m_angle(0.f)
	{
	};

	Client(sf::TcpSocket* socketPtr, const sf::Vector2f& position, const float angle) :
		m_socket(socketPtr),
		m_position(position),
		m_angle(angle)
	{
	}

	// TODO: Swap to unique_ptr - ups are 'Owning Handles'
	sf::TcpSocket* m_socket;
	sf::Vector2f m_position;
	float m_angle;
};

// TODO: Use UDP for connecting, TCP for everything else
class Server
{
public:
	static std::unique_ptr<Server> CreateServer(unsigned short port);

	void Update(unsigned short port);

	~Server();

	// Non-copyable and non-moveable 
	Server(const Server& other) = delete;
	Server(Server&& other) noexcept = delete;
	Server& operator=(const Server& other) = delete;
	Server& operator=(Server&& other) noexcept = delete;

private:
	sf::TcpListener m_listener;
	sf::SocketSelector m_socketSelector;

	std::unordered_map<std::string, Client> m_connectedClients;

	unsigned m_maxClients;

	const std::array<sf::Color, globals::k_playerAmount> m_carColours{
		sf::Color(255, 0, 0),
		sf::Color(0, 0, 255),
		/*sf::Color(0, 255, 0),
		sf::Color(255, 255, 0)*/
	};

	const std::array<sf::Vector2f, globals::k_playerAmount> m_carStartingPositions{
		sf::Vector2f(779.f, 558.f),
		sf::Vector2f(779.f, 616.f),
		/*sf::Vector2f(772.f, 558.f),
		sf::Vector2f(722.f, 616.f)*/
	};

	bool m_gameInProgress;

	Server();
	bool Initialise(unsigned short port);
	void CheckForNewClients();
	void CheckCollisionsBetweenClients();
	bool SendMessage(const DataPacket& dataToSend, const std::string& receiver);
	bool BroadcastMessage(const DataPacket& dataToSend, sf::TcpSocket& sender);
	bool BroadcastMessage(const DataPacket& dataToSend);
	bool ReceiveMessage();
};
