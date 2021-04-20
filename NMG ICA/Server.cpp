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

Server::Server()
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
				uint8_t packetCode{};
				std::string id;

				if (client->receive(packet) == sf::Socket::Done)
				{
					packet >> packetCode >> id;
				}

				if (packetCode == 0)
				{
					std::cout << id << " has connected to the server" << std::endl;
					m_connectedClients.push_back(client);

					// Add the new client to the selector - this means we can update all clients
					m_socketSelector.add(*client);
				}
			} else
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
			// https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1SocketSelector.php for multiple clients

			// Loop through each client and use our new, fancy, socket selector
			for (int i = 0; i < m_connectedClients.size(); ++i)
			{
				if (m_socketSelector.isReady(*m_connectedClients[i]))
				{
					sf::Packet packet;
					if (m_connectedClients[i]->receive(packet) == sf::Socket::Done)
					{
						uint8_t packetCode{};
						std::string id{};
						float x, y;
						packet >> packetCode >> id >> x >> y;

						// TODO: Don't hard-code the codes... Try and use the enum in the server and the client
						if (packetCode == 1)
						{
							const sf::Vector2f playerPos(x, y);
							std::cout << id << " moved to position: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;

							sf::Packet sendPacket;

							sendPacket << static_cast<uint8_t>(1) << id << x << y;
							// update the other connected clients
							for (int j = 0; j < m_connectedClients.size(); ++j)
							{
								if (j != i)
								{
									m_connectedClients[j]->send(sendPacket);
								}
							}
						}


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
