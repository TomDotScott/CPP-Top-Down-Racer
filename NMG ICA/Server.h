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
	sf::TcpSocket m_socket;
	sf::Vector2f m_prevPlayerPosition;
	
	bool m_connected;
	
	Server();
	bool Initialise(unsigned short port);
	bool SendMessage();
	bool ReceiveMessage();
};
