#include "Server.h"

#include <iostream>

Server* Server::CreateServer(const unsigned short port)
{
	auto* s = new Server();
	if (s->Initialise(port))
	{
		return s;
	} else
	{
		delete s;
		return nullptr;
	}
}

Server::Server() :
	m_connected(false)
{

}

bool Server::Initialise(const unsigned short port)
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
	//std::cout << "Sending a message to the connected clients..." << std::endl;
	//
	//// Send a message to the connected client
	//const char out[] = "Hi, I'm the server";
	//if (m_socket.send(out, sizeof(out)) != sf::Socket::Done)
	//	return false;

	//std::cout << "Message sent to the client: \"" << out << "\"" << std::endl;
	return true;
}


bool Server::ReceiveMessage()
{
	// TODO: SWITCH TO USING PACKETS
	// https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1SocketSelector.php for multiple clients

	//// Receive a message back from the client
	//char in[128];
	//
	//size_t received;

	//if (m_socket.receive(in, sizeof(in), received) != sf::Socket::Done)
	//	return false;

	//std::cout << "Answer received from the client: \"" << in << "\nMeasuring " << received << " Bytes" << std::endl;

	sf::Packet p;

	if (m_socket.receive(p) != sf::Socket::Done)
	{
		return false;
	}

	float x, y;

	p >> x >> y;

	const sf::Vector2f playerPos(x, y);

	if (playerPos != m_prevPlayerPosition)
	{
		std::cout << "Data received from the client measuring " << p.getDataSize() << " bytes" << std::endl;

		std::cout << "The shape is at: " << x << ", " << y << std::endl;
	}

	m_prevPlayerPosition = playerPos;

	return true;
}
