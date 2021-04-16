#pragma once
#include <SFML/Network.hpp>

class Client
{
public:
	bool Initialise(unsigned short port);
	void Update();

private:
	sf::IpAddress m_server;
	sf::TcpSocket m_socket;
	
	bool ReceiveMessage();
	bool SendMessage();
};