#include "Server.h"

#include <iostream>
#include <algorithm>
#include <SFML/Graphics/RectangleShape.hpp>
#include "../Shared Files/Data.h"

namespace
{
	const std::array<sf::Color, globals::k_playerAmount> CAR_COLOURS{
		sf::Color(255, 0, 0),
		//sf::Color(0, 0, 255),
		/*sf::Color(0, 255, 0),
		sf::Color(255, 255, 0)*/
	};

	const std::array<sf::Vector2f, globals::k_playerAmount> STARTING_POSITIONS{
		sf::Vector2f(779.f, 558.f),
		//sf::Vector2f(779.f, 616.f),
		/*sf::Vector2f(772.f, 558.f),
		sf::Vector2f(722.f, 616.f)*/
	};
}

Client::Client(const sf::Vector2f& position, const float angle) :
	m_socket(new sf::TcpSocket()),
	m_position(position),
	m_angle(angle),
	m_checkPointsPassed()
{
	for (int i = 0; i < globals::k_numCheckPoints; ++i)
	{
		m_checkPointsPassed[i] = false;
	}
}

Client::~Client()
{
	delete m_socket;
}

bool Client::AllCheckPointsPassed()
{
	int passedCheckPoints = 0;
	for (const auto& cp : m_checkPointsPassed)
	{
		if (cp)
		{
			passedCheckPoints++;
		}
	}

	return passedCheckPoints == globals::k_numCheckPoints;
}

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
	m_gameInProgress(false)
{
	const sf::Vector2f checkPointColliderSize = { globals::k_checkPointWidth, globals::k_checkPointHeight };

	m_levelCheckpoints[0] = { sf::FloatRect({ 803.f, 523.f }, checkPointColliderSize) };
	m_levelCheckpoints[1] = { sf::FloatRect({514.f, 11.f}, checkPointColliderSize) };
	m_levelCheckpoints[2] = { sf::FloatRect({ 382.f, 313.f }, checkPointColliderSize) };
	m_levelCheckpoints[3] = { sf::FloatRect({ 167.f, 37.f }, checkPointColliderSize) };
	m_levelCheckpoints[4] = { sf::FloatRect({ 132.f, 597.f }, checkPointColliderSize) };
	m_levelCheckpoints[5] = { sf::FloatRect({ 434.f, 489.f }, checkPointColliderSize) };
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
		// Find the next available colour for the players
		sf::Color colour = CAR_COLOURS[m_connectedClients.size()];

		// Find the next available starting position for the players
		sf::Vector2f startingPosition = STARTING_POSITIONS[m_connectedClients.size()];

		// Create a new connection

		auto* newClient = new Client(startingPosition, globals::k_carStartingRotation);
		if (m_listener.accept(*newClient->m_socket) == sf::Socket::Done)
		{
			sf::Packet inPacket;
			DataPacket inData;

			if (newClient->m_socket->receive(inPacket) == sf::Socket::Done)
			{
				inPacket >> inData;
			}

			if (m_connectedClients.size() < globals::k_playerAmount)
			{
				if (inData.m_type == eDataPacketType::e_FirstConnection)
				{
					if (!IsUsernameTaken(inData.m_userName) && inData.m_userName != globals::k_reservedServerUsername)
					{
						// To tell the client that they are successful
						sf::Packet outPacket;

						DataPacket outData(
							eDataPacketType::e_UserNameConfirmation,
							globals::k_reservedServerUsername,
							startingPosition.x,
							startingPosition.y,
							globals::k_carStartingRotation,
							colour
						);

						// Add the new client to the selector - this means we can update all clients
						m_socketSelector.add(*newClient->m_socket);

						std::cout << inData.m_userName << " has connected to the server" << std::endl;

						newClient->m_username = inData.m_userName;
						
						m_connectedClients.emplace_back(newClient);

						outPacket << outData;

						newClient->m_socket->send(outPacket);

						outPacket.clear();

						// Tell the other clients that a new client has connected
						const DataPacket updateClientDataPacket(
							eDataPacketType::e_NewClient,
							globals::k_reservedServerUsername,
							startingPosition.x,
							startingPosition.y,
							globals::k_carStartingRotation,
							colour
						);

						outPacket << updateClientDataPacket;

						BroadcastMessage(updateClientDataPacket, *newClient->m_socket);
					} else
					{
						std::cout << "A CLIENT WITH THE USERNAME: " << inData.m_userName << " ALREADY EXISTS..." << std::endl;

						sf::Packet usernameRejectionPkt;
						DataPacket usernameRejectionData(eDataPacketType::e_UserNameRejection, globals::k_reservedServerUsername);
						usernameRejectionPkt << usernameRejectionData;

						newClient->m_socket->send(usernameRejectionPkt);
						delete newClient;
					}
				}
			} else
			{
				std::cout << "MAXIMUM AMOUNT OF CLIENTS CONNECTED" << std::endl;

				sf::Packet maxClientMessagePkt;
				const DataPacket maximumClientMessage(eDataPacketType::e_MaxPlayers, "SERVER");
				maxClientMessagePkt << maximumClientMessage;

				newClient->m_socket->send(maxClientMessagePkt);
				delete newClient;
			}
		} else
		{
			std::cout << "A client had an error connecting..." << std::endl;
			delete newClient;
		}
	}
}

void Server::CheckCollisionsBetweenClients()
{
	for (auto& client : m_connectedClients)
	{
		for (auto& otherClient : m_connectedClients)
		{
			if (client->m_username != otherClient->m_username)
			{
				float dx = client->m_position.x - otherClient->m_position.x;
				float dy = client->m_position.y - otherClient->m_position.y;

				bool collisionOccurred = false;

				while (dx * dx + dy * dy < 8 * 10.f * 17.f)
				{
					collisionOccurred = true;

					client->m_position.x += dx / 10.f;
					client->m_position.x += dy / 10.f;
					otherClient->m_position.x -= dx / 10.f;
					otherClient->m_position.x -= dy / 10.f;
					dx = client->m_position.x - otherClient->m_position.x;
					dy = client->m_position.y - otherClient->m_position.y;

					if (dx == 0 && dy == 0)
						break;
				}

				// If a collision occurred and was resolved, update the clients
				if (collisionOccurred)
				{
					DataPacket playerOneCollisionData(eDataPacketType::e_CollisionData, client->m_username, client->m_position, otherClient->m_username);
					SendMessage(playerOneCollisionData, client->m_username);

					DataPacket playerTwoCollisionData(eDataPacketType::e_CollisionData, otherClient->m_username, otherClient->m_position, client->m_username);
					SendMessage(playerTwoCollisionData, otherClient->m_username);
				}
			}
		}
	}
}

int Server::FindClientIndex(const std::string& username) const
{
	int index{ 0 };
	for (const auto& client : m_connectedClients)
	{
		if(client->m_username == username)
		{
			return index;
		}
		index++;
	}

	return -1;
}

bool Server::IsUsernameTaken(const std::string& username) const
{
	for (const auto& client : m_connectedClients)
	{
		if (client->m_username == username)
		{
			return true;
		}
	}

	return false;
}

void Server::CheckIfClientHasPassedCheckPoint(Client& client)
{
	// Work out the bounding box of the client
	sf::RectangleShape clientRect({ globals::k_carSpriteWidth, globals::k_carSpriteHeight });
	clientRect.setOrigin(globals::k_carOriginX, globals::k_carOriginY);

	clientRect.setPosition(client.m_position);

	// See if the player has overlapped the checkpoints
	for (int i = 0; i < globals::k_numCheckPoints; ++i)
	{
		if (m_levelCheckpoints[i].intersects(clientRect.getGlobalBounds()))
		{
			client.m_checkPointsPassed[i] = true;

			std::cout << "Checkpoint number: " << i << " has been passed" << std::endl;

			// See if all the checkpoints have been passed
			if (client.AllCheckPointsPassed())
			{
				std::cout << client.m_username << " has completed a lap!" << std::endl;
			}
		}
	}

}

bool Server::SendMessage(const DataPacket& dataToSend, const std::string& receiver)
{
	sf::Packet outPacket;
	outPacket << dataToSend;

	m_connectedClients[FindClientIndex(receiver)]->m_socket->send(outPacket);

	return true;
}

void Server::Update(unsigned short port)
{
	if (m_socketSelector.wait())
	{
		CheckForNewClients();

		if (m_connectedClients.size() == globals::k_playerAmount)
		{
			if (!m_gameInProgress)
			{
				m_gameInProgress = true;
				std::cout << globals::k_playerAmount << " have connected, starting the game..." << std::endl;

				const DataPacket outDataPacket(eDataPacketType::e_StartGame, globals::k_reservedServerUsername);
				BroadcastMessage(outDataPacket);
			}
		}

		// Loop through each client and use our new, fancy, socket selector
		for (auto& client : m_connectedClients)
		{
			if (client->m_socket && m_socketSelector.isReady(*client->m_socket))
			{
				sf::Packet inPacket;
				const auto clientStatus = client->m_socket->receive(inPacket);

				if (clientStatus == sf::Socket::Done)
				{
					DataPacket inData;

					inPacket >> inData;

					switch (inData.m_type)
					{
					case eDataPacketType::e_UpdatePosition:
						client->m_position = { inData.m_x, inData.m_y };
						client->m_angle = inData.m_angle;

						BroadcastMessage(inData, *client->m_socket);

						CheckIfClientHasPassedCheckPoint(*client);
						break;
					default:
						break;
					}
				}

				if (clientStatus == sf::Socket::Disconnected)
				{
					std::string disconnectedClientUsername = client->m_username;

					std::cout << "PLAYER WITH THE USERNAME: " << disconnectedClientUsername << " DISCONNECTED FROM THE SERVER" << std::endl;

					m_socketSelector.remove(*client->m_socket);

					client->m_socket->disconnect();

					// Remove from the vector
					int clientIndex = FindClientIndex(client->m_username);
					
					m_connectedClients.erase(m_connectedClients.begin() + clientIndex);

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

bool Server::BroadcastMessage(const DataPacket& dataToSend, sf::TcpSocket& sender) const
{
	sf::Packet sendPacket;
	sendPacket << dataToSend;

	// update the other connected clients
	for (const auto& client : m_connectedClients)
	{
		// Make sure not to send the client their own data!
		if (client->m_username != dataToSend.m_userName)
		{
			if (dataToSend.m_userName != globals::k_reservedServerUsername)
			{
				client->m_socket->send(sendPacket);
			}
		}
	}

	return true;
}

bool Server::BroadcastMessage(const DataPacket& dataToSend) const
{
	sf::Packet sendPacket;
	sendPacket << dataToSend;

	// update the other connected clients
	for (const auto& client : m_connectedClients)
	{
		if(client->m_socket->send(sendPacket) != sf::Socket::Done)
		{
			std::cout << "Error sending message to " << client->m_username << std::endl;
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
