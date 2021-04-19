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
	m_connected(false),
	m_nextValidID(0)
{

}

Server::~Server()
{
	for (auto& connectedClient : m_connectedClients)
	{
		delete connectedClient;
	}
}

bool Server::Initialise(const unsigned short port)
{
	// Listen to the given port for incoming connections
	if (m_listener.listen(port, sf::IpAddress::getLocalAddress()) != sf::Socket::Done)
	{
		return false;
	}

	// Add the listener to the selector
	m_socketSelector.add(m_listener);
	
	std::cout << "Server is listening to port " << port << ", waiting for connections... " << std::endl;
	return true;
}

void Server::Update(unsigned short port)
{
	if (m_socketSelector.wait())
	{
		// std::cout << "Waiting for data on any socket..." << std::endl;
		if (m_socketSelector.isReady(m_listener))
		{
			// std::cout << "The listener is ready, testing to see if there's a connection..." << std::endl;
			
			// Create a new connection
			auto* client = new sf::TcpSocket();
			if (m_listener.accept(*client) == sf::Socket::Done)
			{

				sf::Packet packet;
				std::string id;

				if (client->receive(packet) == sf::Socket::Done)
				{
					packet >> id;
				}

				std::cout << id << " has connected to the server" << std::endl;
				m_connectedClients.push_back(client);

				// Add the new client to the selector - this means we can update all clients
				m_socketSelector.add(*client);
			}else
			{
				std::cout << "A client had an error connecting..." << std::endl;
				delete client;
			}

			// Wait for a connection
			/*if (m_listener.accept(m_socketSelector) != sf::Socket::Done)
				return;

			std::cout << "Client connected: " << m_socketSelector.getRemoteAddress() << std::endl;
			m_connected = true;*/
		} else
		{
			ReceiveMessage();

			SendMessage();
		}
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
	// https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1SocketSelector.php for multiple clients

	// Loop through each client and use our new, fancy, socket selector
	for (auto& connectedClient : m_connectedClients)
	{
		if (m_socketSelector.isReady(*connectedClient))
		{
			sf::Packet packet;
			if (connectedClient->receive(packet) == sf::Socket::Done)
			{
				float x, y;
				packet >> x >> y;

				const sf::Vector2f playerPos(x, y);

				/*if (playerPos != m_prevPlayerPosition)
				{
					std::cout << "Data received from the client measuring " << packet.getDataSize() << " bytes" << std::endl;

					std::cout << "The shape is at: " << x << ", " << y << std::endl;
				}*/

				// Now we have all the data from the packet, we need to send it to the other
				// clients so they're up to date
			}
		}
	}

	/*sf::Packet p;

	if (m_socketSelector.receive(p) != sf::Socket::Done)
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

	m_prevPlayerPosition = playerPos;*/

	return true;
}
