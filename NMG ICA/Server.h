#pragma once
#include <list>
#include <unordered_map>
#include <SFML/Network.hpp>
#include <SFML/Graphics/Color.hpp>

#include "Globals.h"
#include "../Shared Files/Data.h"

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
	
	// TODO: Swap to map of unique_ptr - ups are 'Owning Handles'
	std::unordered_map<std::string, sf::TcpSocket*> m_connectedClients;
	
	unsigned m_maxClients;

	const std::array<sf::Color, globals::k_playerAmount> m_carColours{
		sf::Color(255, 0, 0),
		sf::Color(0, 0, 255),
		/*sf::Color(0, 255, 0),
		sf::Color(255, 255, 0)*/
	};

	bool m_gameInProgress;
	
	Server();
	bool Initialise(unsigned short port);
	void CheckForNewClients();
	bool SendMessage(const DataPacket& dataToSend, sf::TcpSocket& sender);
	bool SendMessage(const DataPacket& dataToSend);
	bool ReceiveMessage();
};
