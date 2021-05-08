#include "Server.h"

#include <iostream>

#include "../Shared Files/Data.h"

std::unique_ptr<Server> Server::CreateServer(const unsigned short port)
{
	std::unique_ptr<Server> newServer(new Server());
	if (newServer->Initialise(port))
	{
		return newServer;
	}

	return nullptr;
}

Server::Server() :
	m_maxClients(globals::k_playerAmount),
	m_gameInProgress(false)
{

}

Server::~Server()
{
	for (auto& connectedClient : m_connectedClients)
	{
		delete connectedClient.second.m_socket;
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
		auto* newTcpSocket = new sf::TcpSocket();
		if (m_listener.accept(*newTcpSocket) == sf::Socket::Done)
		{
			sf::Packet inPacket;
			DataPacket inData;

			if (newTcpSocket->receive(inPacket) == sf::Socket::Done)
			{
				inPacket >> inData;
			}

			if (m_connectedClients.size() < m_maxClients)
			{
				if (inData.m_type == eDataPacketType::e_FirstConnection)
				{
					if (!globals::is_value_in_map(m_connectedClients, inData.m_userName) && inData.m_userName != globals::k_reservedServerUsername)
					{
						// Find the next available colour for the players
						sf::Color colour = m_carColours[m_connectedClients.size()];

						// To tell the client that they are successful
						sf::Packet outPacket;

						DataPacket outData(eDataPacketType::e_UserNameConfirmation, globals::k_reservedServerUsername, 0.f, 0.f, 0.f, colour);

						std::cout << "The next available colour " << static_cast<int>(colour.r) << " " << static_cast<int>(colour.g) << " " << static_cast<int>(colour.b) << std::endl;

						// Add the new client to the selector - this means we can update all clients
						m_socketSelector.add(*newTcpSocket);

						std::cout << inData.m_userName << " has connected to the server" << std::endl;

						Client client(newTcpSocket);

						m_connectedClients.insert(std::make_pair(inData.m_userName, client));

						outPacket << outData;

						newTcpSocket->send(outPacket);

						outPacket.clear();

						// Tell the other clients that a new client has connected
						const DataPacket updateClientDataPacket(eDataPacketType::e_NewClient, globals::k_reservedServerUsername, colour);

						outPacket << updateClientDataPacket;

						BroadcastMessage(updateClientDataPacket, *newTcpSocket);
					} else
					{
						std::cout << "A CLIENT WITH THE USERNAME: " << inData.m_userName << " ALREADY EXISTS..." << std::endl;

						sf::Packet usernameRejectionPkt;
						DataPacket usernameRejectionData(eDataPacketType::e_UserNameRejection, globals::k_reservedServerUsername);
						usernameRejectionPkt << usernameRejectionData;

						newTcpSocket->send(usernameRejectionPkt);

						delete newTcpSocket;
					}
				}
			} else
			{
				std::cout << "MAXIMUM AMOUNT OF CLIENTS CONNECTED" << std::endl;

				sf::Packet maxClientMessagePkt;
				const DataPacket maximumClientMessage(eDataPacketType::e_MaxPlayers, "SERVER");
				maxClientMessagePkt << maximumClientMessage;

				newTcpSocket->send(maxClientMessagePkt);

				delete newTcpSocket;
			}
		} else
		{
			std::cout << "A client had an error connecting..." << std::endl;
			delete newTcpSocket;
		}
	}
}

void Server::CheckCollisionsBetweenClients()
{
	for (auto& client : m_connectedClients)
	{
		for (auto& otherClient : m_connectedClients)
		{
			if (client.first != otherClient.first)
			{
				float dx = client.second.m_position.x - otherClient.second.m_position.x;
				float dy = client.second.m_position.y - otherClient.second.m_position.y;
				
				if (dx * dx + dy * dy < 4 * 10.f * 17.f)
				{
					std::cout << client.first << " is colliding with " << otherClient.first << std::endl;
				}

				std::cout << "No collision is happening" << std::endl;
			}
		}
	}
}

bool Server::SendMessage(const DataPacket& dataToSend, const std::string& receiver)
{

}

void Server::Update(unsigned short port)
{
	if (m_socketSelector.wait())
	{
		CheckForNewClients();

		if (m_connectedClients.size() == m_maxClients)
		{
			if (!m_gameInProgress)
			{
				m_gameInProgress = true;
				std::cout << m_maxClients << " have connected, starting the game..." << std::endl;

				const DataPacket outDataPacket(eDataPacketType::e_StartGame, globals::k_reservedServerUsername);
				BroadcastMessage(outDataPacket);
			}
		}

		// Loop through each client and use our new, fancy, socket selector
		for (auto& client : m_connectedClients)
		{
			if (client.second.m_socket && m_socketSelector.isReady(*client.second.m_socket))
			{
				sf::Packet inPacket;
				const auto clientStatus = client.second.m_socket->receive(inPacket);

				if (clientStatus == sf::Socket::Done)
				{
					DataPacket inData;

					inPacket >> inData;

					if (inData.m_type == eDataPacketType::e_UpdatePosition)
					{
						client.second.m_position = { inData.m_x, inData.m_y };
						client.second.m_angle = inData.m_angle;

						BroadcastMessage(inData, *client.second.m_socket);
					}
				}

				if (clientStatus == sf::Socket::Disconnected)
				{
					std::string disconnectedClientUsername = client.first;

					std::cout << "PLAYER WITH THE USERNAME: " << disconnectedClientUsername << " DISCONNECTED FROM THE SERVER" << std::endl;

					m_socketSelector.remove(*client.second.m_socket);

					client.second.m_socket->disconnect();

					// Delete the allocated memory
					delete client.second.m_socket;

					// Remove from the map
					m_connectedClients.erase(client.first);

					// See if it was the last person to leave
					if (m_connectedClients.empty())
					{
						m_gameInProgress = false;
					}

					// Tell the other clients that a client disconnected
					sf::Packet clientDisconnectedPkt;

					DataPacket clientDisconnectedData(eDataPacketType::e_ClientDisconnected, disconnectedClientUsername);

					clientDisconnectedPkt << clientDisconnectedData;
					BroadcastMessage(clientDisconnectedData);
				}
			}
		}

		// Check collisions and update the clients accordingly...
		CheckCollisionsBetweenClients();
	}
}

bool Server::BroadcastMessage(const DataPacket& dataToSend, sf::TcpSocket& sender)
{
	sf::Packet sendPacket;
	sendPacket << dataToSend;

	// update the other connected clients
	for (auto& client : m_connectedClients)
	{
		// Make sure not to send the client their own data!
		if (client.first != dataToSend.m_userName)
		{
			if (dataToSend.m_userName != globals::k_reservedServerUsername)
			{
				client.second.m_socket->send(sendPacket);
			}
		}
	}

	return true;
}

bool Server::BroadcastMessage(const DataPacket& dataToSend)
{
	sf::Packet sendPacket;
	sendPacket << dataToSend;

	// update the other connected clients
	for (auto& client : m_connectedClients)
	{
		client.second.m_socket->send(sendPacket);
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
