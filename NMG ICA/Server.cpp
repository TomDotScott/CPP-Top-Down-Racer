#include "Server.h"
#include "../Shared Files/Data.h"
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
	m_maxClients(8)
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

void Server::CheckForNewClients()
{
	if (m_socketSelector.isReady(m_listener))
	{
		// Create a new connection
		auto* client = new sf::TcpSocket();
		if (m_listener.accept(*client) == sf::Socket::Done)
		{
			sf::Packet packet;
			DataPacket dp;

			if (client->receive(packet) == sf::Socket::Done)
			{
				packet >> dp;
			}

			if (m_connectedClients.size() < m_maxClients)
			{
				if (dp.m_type == FIRST_CONNECTION)
				{
					std::cout << dp.m_userName << " has connected to the server" << std::endl;
					m_connectedClients.push_back(client);

					// Add the new client to the selector - this means we can update all clients
					m_socketSelector.add(*client);

					// Tell the client which player it is
					sf::Packet p;
					const uint8_t playerNum = static_cast<uint8_t>(m_connectedClients.size()) - static_cast<uint8_t>(1);
					
					p << static_cast<uint8_t>(playerNum);

					std::cout << "Telling " << dp.m_userName << " that they are player: " << playerNum << std::endl;

					client->send(p);
				}
			} else
			{
				std::cout << "MAXIMUM AMOUNT OF CLIENTS CONNECTED" << std::endl;
				delete client;
			}
		} else
		{
			std::cout << "A client had an error connecting..." << std::endl;
			delete client;
		}
	}
}

void Server::Update(unsigned short port)
{
	if (m_socketSelector.wait())
	{
		CheckForNewClients();

		// Loop through each client and use our new, fancy, socket selector
		for(auto& client : m_connectedClients)
		{
			if(m_socketSelector.isReady(*client))
			{
				sf::Packet packet;
				if(client->receive(packet) == sf::Socket::Done)
				{
					DataPacket dp;
					packet >> dp;

					std::cout << "Received a message from: " << dp.m_userName << std::endl;

					if (dp.m_type == UPDATE_POSITION)
					{
						SendMessage(dp, client);
					}
				}
			}
		}
	}
}

bool Server::SendMessage(const DataPacket& dp, sf::TcpSocket* sender)
{
	const sf::Vector2f playerPos(dp.m_x, dp.m_y);
	std::cout << dp.m_userName << " moved to position: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;

	sf::Packet sendPacket;

	sendPacket << dp;

	// update the other connected clients
	for (auto& client : m_connectedClients)
	{
		if (client != sender)
		{
			client->send(sendPacket);
		}
	}
	
	return true;
}


bool Server::ReceiveMessage()
{

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
