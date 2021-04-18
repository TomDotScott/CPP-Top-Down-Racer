#include "Server.h"

#include <iostream>

// TODO: Factory pattern creation should be the Initialise function below
// Return a heap allocated Server, or a nullptr if it fails :P
//Server* Server::createServer(unsigned short port)
//{
//	
//}

bool Server::Initialise(unsigned short port)
{
	// Listen to the given port for incoming connections
	if (m_listener.listen(port, sf::IpAddress::getLocalAddress()) != sf::Socket::Done)
	{
		return false;
	}

	std::cout << "Server is listening to port " << port << ", waiting for connections... " << std::endl;
	return true;
}

void Server::Update(unsigned short port)
{
	if (!m_connected)
	{
		// Wait for a connection
		if (m_listener.accept(m_socket) != sf::Socket::Done)
			return;

		std::cout << "Client connected: " << m_socket.getRemoteAddress() << std::endl;
		m_connected = true;
	} else
	{
		ReceiveMessage();

		SendMessage();
	}
}

bool Server::SendMessage()
{
	std::cout << "Sending a message to the connected clients..." << std::endl;
	
	// Send a message to the connected client
	const char out[] = "Hi, I'm the server";
	if (m_socket.send(out, sizeof(out)) != sf::Socket::Done)
		return false;

	std::cout << "Message sent to the client: \"" << out << "\"" << std::endl;
	return true;
}


bool Server::ReceiveMessage()
{
	std::cout << "Seeing if the server has received anything..." << std::endl;

	// TODO: SWITCH TO USING PACKETS
	// https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1SocketSelector.php for multiple clients

	// Receive a message back from the client
	char in[128];
	
	size_t received;

	if (m_socket.receive(in, sizeof(in), received) != sf::Socket::Done)
		return false;

	std::cout << "Answer received from the client: \"" << in << "\nMeasuring " << received << " Bytes" << std::endl;

	return true;
}
