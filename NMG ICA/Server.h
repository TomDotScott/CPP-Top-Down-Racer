#pragma once
#include <list>
#include <SFML/Network.hpp>

#include "../Shared Files/Data.h"

class Server
{
public:
	static Server* CreateServer(unsigned short port);
	// TODO: Swap to Unique_Ptr

	void Update(unsigned short port);

private:
	sf::TcpListener m_listener;
	sf::SocketSelector m_socketSelector;
	
	// Swapped from vector to list because we add and remove clients on the fly
	std::list<sf::TcpSocket*> m_connectedClients;
	unsigned m_maxClients;
	
	Server();
	~Server();
	bool Initialise(unsigned short port);
	void CheckForNewClients();
	bool SendMessage(const DataPacket& dp, sf::TcpSocket* sender);
	bool ReceiveMessage();
};
