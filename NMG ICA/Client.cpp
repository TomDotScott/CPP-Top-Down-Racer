#include "Client.h"

#include <iostream>

bool Client::Initialise(const unsigned short port)
{
	m_server = sf::IpAddress::getLocalAddress();

	// Connect to the server
	if (m_socket.connect(m_server, port) != sf::Socket::Done)
		return false;

	std::cout << "Client connected to server " << m_server << std::endl;
	return true;
}

void Client::Update()
{
	SendMessage();
	ReceiveMessage();
}

bool Client::ReceiveMessage()
{
	// Receive a message from the server
	char in[128];
	std::size_t received;
	if (m_socket.receive(in, sizeof(in), received) != sf::Socket::Done)
		return false;
	std::cout << "Message received from the server: \"" << in << "\"" << std::endl;
	return true;
}

bool Client::SendMessage()
{
	// Send a message to the server
	std::cout << "Send a message: ";
	
	std::string message;
	std::cin >> message;
	
	if (m_socket.send(message.c_str(), sizeof(message.c_str())) != sf::Socket::Done)
		return false;

	std::cout << "Message sent to the server: '" << message << "'\tMeasuring: " << sizeof(message.c_str()) << " bytes... " << std::endl;
	return true;
}
