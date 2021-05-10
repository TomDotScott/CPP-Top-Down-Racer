#include "Server.h"

#include <iostream>
#include <algorithm>
#include <SFML/Graphics/RectangleShape.hpp>
#include "../Shared Files/Data.h"

namespace
{
	const std::array<sf::Color, globals::k_playerAmount> CAR_COLOURS{
		sf::Color(255, 0, 0),
		sf::Color(0, 0, 255),
		/*sf::Color(0, 255, 0),
		sf::Color(255, 255, 0)*/
	};

	const std::array<sf::Vector2f, globals::k_playerAmount> STARTING_POSITIONS{
		sf::Vector2f(779.f, 558.f),
		sf::Vector2f(779.f, 616.f),
		/*sf::Vector2f(772.f, 558.f),
		sf::Vector2f(722.f, 616.f)*/
	};

	const std::array<sf::FloatRect, globals::k_numCheckPoints> LEVEL_CHECKPOINTS{
		sf::FloatRect({ 803.f, 523.f }, globals::k_checkPointColliderSize),
	sf::FloatRect({514.f, 11.f}, globals::k_checkPointColliderSize),
	sf::FloatRect({ 382.f, 313.f }, globals::k_checkPointColliderSize),
	sf::FloatRect({ 167.f, 37.f }, globals::k_checkPointColliderSize),
	sf::FloatRect({ 132.f, 597.f }, globals::k_checkPointColliderSize),
	sf::FloatRect({ 434.f, 489.f }, globals::k_checkPointColliderSize)
	};
}

Client::Client(const sf::Vector2f& position, const float angle) :
	m_socket(new sf::TcpSocket()),
	m_position(position),
	m_angle(angle),
	m_checkPointsPassed(),
	m_lapsCompleted(0),
	m_raceCompleted(false)
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

void Client::ResetCheckPoints()
{
	std::cout << "Completed a lap, resetting the checkpoints" << std::endl;
	for (auto& point : m_checkPointsPassed)
	{
		point = false;
	}
}

int Client::HighestCheckPointPassed()
{
	for (int i = globals::k_numCheckPoints - 1; i <= 0; ++i)
	{
		if (m_checkPointsPassed[i])
		{
			return i;
		}
	}
	return 0;
}

void Client::PrintCheckPoints()
{
	for (int i = 0; i < m_checkPointsPassed.size(); ++i)
	{
		std::cout << "Checkpoint: " << i << (m_checkPointsPassed[i] ? " passed" : " not passed") << std::endl;
	}
}

bool Client::CompareByLap(std::unique_ptr<Client>& a, std::unique_ptr<Client>& b)
{
	return a->m_lapsCompleted > b->m_lapsCompleted;
}

bool Client::CompareByCheckPoints(std::unique_ptr<Client>& a, std::unique_ptr<Client>& b)
{
	// Only compare checkpoints if the laps are the same
	if (a->m_lapsCompleted == b->m_lapsCompleted)
	{
		// Find the index of the highest lap completed
		const int aHighestCheckPoint = a->HighestCheckPointPassed();
		const int bHighestCheckPoint = b->HighestCheckPointPassed();
		return aHighestCheckPoint > bHighestCheckPoint;
	}
	return false;
}

bool Client::CompareByDistance(std::unique_ptr<Client>& a, std::unique_ptr<Client>& b)
{
	if (a->m_lapsCompleted == b->m_lapsCompleted && a->HighestCheckPointPassed() == b->HighestCheckPointPassed())
	{
		// Calculate the Distance to the next checkpoint
		const int highestCheckPoint = a->HighestCheckPointPassed();

		sf::Vector2f nextCheckpointPosition;

		if (highestCheckPoint == 5)
			nextCheckpointPosition = sf::Vector2f(LEVEL_CHECKPOINTS[highestCheckPoint + 1].left, LEVEL_CHECKPOINTS[highestCheckPoint].top);
		else
			nextCheckpointPosition = sf::Vector2f(LEVEL_CHECKPOINTS[0].left, LEVEL_CHECKPOINTS[0].top);

		// Find the distance to the next checkpoint
		sf::Vector2f aC = nextCheckpointPosition - a->m_position;
		sf::Vector2f bC = nextCheckpointPosition - b->m_position;

		// Find the magnitude of the vectors
		const float magAC = globals::sqr_magnitude(aC);
		const float magBC = globals::sqr_magnitude(bC);

		return magAC > magBC;

	}
	return false;
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

bool Server::CheckGameOver()
{
	// See if every racer has won
	bool gameOver = true;
	for (const auto& racer : m_connectedClients)
	{
		if (!racer->m_raceCompleted)
		{
			std::cout << racer->m_username << " still hasn't finished the race! " << std::endl;
			gameOver = false;
		}
	}

	return gameOver;
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

void Server::WorkOutTrackPlacements()
{
	// Find out the previous positions
	std::vector<std::string> previousPositions;

	for (const auto& client : m_connectedClients)
	{
		previousPositions.emplace_back(client->m_username);
	}

	// STEP 1 - CHECK THE LAPS OF THE PLAYERS
	std::sort(m_connectedClients.begin(), m_connectedClients.end(), Client::CompareByLap);

	// STEP 2 - CHECK WHICH CHECKPOINTS THEY HAVE PASSED
	std::sort(m_connectedClients.begin(), m_connectedClients.end(), Client::CompareByCheckPoints);

	// STEP 3 - CHECK DISTANCE BETWEEN THEM AND THE NEXT CHECKPOINT
	std::sort(m_connectedClients.begin(), m_connectedClients.end(), Client::CompareByDistance);

	// STEP 5 - SEE IF THE POSITIONS HAVE CHANGED
	bool positionsChanged = false;
	for (int i = 0; i < previousPositions.size(); ++i)
	{
		if (m_connectedClients[i]->m_username != previousPositions[i])
		{
			positionsChanged = true;
		}
	}

	if (positionsChanged)
	{
		// STEP 4 - TELL THE CLIENTS WHICH PLACE THEY ARE IN
		for (int i = 0; i < m_connectedClients.size(); ++i)
		{
			std::cout << "\tPosition " << i + 1 << " : " << m_connectedClients[i]->m_username << std::endl;
			DataPacket overtakenDataPacket(eDataPacketType::e_Overtaken, globals::k_reservedServerUsername, i + 1);
			SendMessage(overtakenDataPacket, m_connectedClients[i]->m_username);
		}
	}

}

int Server::FindClientIndex(const std::string& username) const
{
	int index{ 0 };
	for (const auto& client : m_connectedClients)
	{
		if (client->m_username == username)
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
		if (LEVEL_CHECKPOINTS[i].intersects(clientRect.getGlobalBounds()))
		{
			if (i == 0)
			{
				if (client.m_lapsCompleted == 0 && client.m_checkPointsPassed[globals::k_numCheckPoints - 1])
				{
					client.m_checkPointsPassed[0] = true;
				} else if (client.m_checkPointsPassed[globals::k_numCheckPoints - 1])
				{
					client.m_checkPointsPassed[0] = true;
				}
			} else
			{
				client.m_checkPointsPassed[i] = true;
			}

			// See if all the checkpoints have been passed
			if (client.AllCheckPointsPassed() && client.m_lapsCompleted != globals::k_totalLaps)
			{
				std::cout << client.m_username << " has completed a lap!" << std::endl;

				// Tell the client that they have completed a lap
				DataPacket lapCompletedDataPacket(eDataPacketType::e_LapCompleted, globals::k_reservedServerUsername);
				SendMessage(lapCompletedDataPacket, client.m_username);

				client.m_lapsCompleted++;
				client.ResetCheckPoints();

				if (client.m_lapsCompleted == globals::k_totalLaps)
				{
					std::cout << client.m_username << " completed the race!" << std::endl;
					client.m_raceCompleted = true;

					// Tell the client that they completed the race
					DataPacket raceCompletedDataPacket(eDataPacketType::e_RaceCompleted, globals::k_reservedServerUsername);
					SendMessage(raceCompletedDataPacket, client.m_username);
				}
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

						WorkOutTrackPlacements();

						if (CheckGameOver())
						{
							// record the final race order
							std::vector<std::string> racePositions;
							for (const auto& racer : m_connectedClients)
							{
								std::cout << racer->m_username << std::endl;
								racePositions.emplace_back(racer->m_username);
							}

							DataPacket endOfMatchDataPacket(eDataPacketType::e_GameOver, globals::k_reservedServerUsername, racePositions);
							BroadcastMessage(endOfMatchDataPacket);
						}


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
		if (client->m_socket->send(sendPacket) != sf::Socket::Done)
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
