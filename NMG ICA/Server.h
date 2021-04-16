#pragma once
#include <SFML/Network.hpp>

class Server
{
public:
	bool InitialiseServer(unsigned short port);


	void RunTcpServer(unsigned short port);

	bool SendMessage();

	bool ReceiveMessage();

private:
	
	// Create a server socket to accept new connections
	sf::TcpListener m_listener;

	sf::TcpSocket m_socket;

	bool m_connected = false;
};
