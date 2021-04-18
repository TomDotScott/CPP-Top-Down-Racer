#pragma once
#include <SFML/Network.hpp>

// TODO: READ UP ON THE FACTORY PATTERN
class Server
{
public:
	// TODO:
	// static Server* createServer(unsigned short port);
	// TODO: Potentially swap to Unique_Ptr

	bool Initialise(unsigned short port);

	void Update(unsigned short port);

	bool SendMessage();

	bool ReceiveMessage();

private:
	// Factory pattern means that the constructor has to be private
	// Pass in the listener and the port
	// Server(sf::TcpListener listener) {}

	// Create a server socket to accept new connections
	sf::TcpListener m_listener;

	sf::TcpSocket m_socket;

	bool m_connected = false;
};
