#pragma once
#include <SFML/Network.hpp>

class Server
{
public:
	static Server* CreateServer(unsigned short port);
	// TODO: Swap to Unique_Ptr

	void Update(unsigned short port);

private:
	sf::TcpListener m_listener;
	sf::SocketSelector m_socketSelector;
	std::vector<sf::TcpSocket*> m_connectedClients;
	
	Server();
	~Server();
	bool Initialise(unsigned short port);
	bool SendMessage();
	bool ReceiveMessage();
};
