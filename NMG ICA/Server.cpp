#include "Server.h"

#include <iostream>
#include <algorithm>
#include <SFML/Graphics/RectangleShape.hpp>
#include "../Shared Files/Data.h"
#include <algorithm>

// constants that exist within Server.cpp and don't need to be defined in
// globals.h
namespace
{
	// The colours of the clients so that they are recognisable across instances
	const std::array<sf::Color, globals::game::k_playerAmount> CAR_COLOURS{
		sf::Color(255, 0, 0),
		sf::Color(0, 0, 255),
		sf::Color(0, 255, 0),
		sf::Color(255, 255, 0)
	};

	// The starting positions of the clients
	const std::array<sf::Vector2f, globals::game::k_playerAmount> STARTING_POSITIONS{
		sf::Vector2f(779.f, 558.f),
		sf::Vector2f(779.f, 616.f),
		sf::Vector2f(772.f, 558.f),
		sf::Vector2f(722.f, 616.f)
	};

	// The positions of the 11 checkpoints that are placed around the map to ensure that a
	// proper lap has been completed
	const std::array<sf::FloatRect, globals::game::k_numCheckPoints> LEVEL_CHECKPOINTS{
		sf::FloatRect({ 803.f, 523.f }, globals::game::k_checkPointColliderSize),
		sf::FloatRect({ 846.f, 373.f }, globals::game::k_checkPointColliderSize),
		sf::FloatRect({ 681.f, 313.f }, globals::game::k_checkPointColliderSize),
	sf::FloatRect({511.f, 11.f}, globals::game::k_checkPointColliderSize),
	sf::FloatRect({ 384.f, 313.f }, globals::game::k_checkPointColliderSize),
	sf::FloatRect({ 167.f, 37.f }, globals::game::k_checkPointColliderSize),
	sf::FloatRect({ 17.f, 400.f }, {globals::game::k_checkPointHeight, globals::game::k_checkPointWidth}),
	sf::FloatRect({ 132.f, 597.f }, globals::game::k_checkPointColliderSize),
	sf::FloatRect({ 282.f, 584.f }, globals::game::k_checkPointColliderSize),
	sf::FloatRect({ 434.f, 489.f }, globals::game::k_checkPointColliderSize),
	sf::FloatRect({ 642.f, 508.f }, globals::game::k_checkPointColliderSize),
	};

	/**
	 * \brief A helper function for Server::WorkOutTrackPlacements, compares two clients to see who
	 * has completed the most laps
	 * \param a The first client
	 * \param b The second client
	 * \return True if a has completed more laps than b
	 */
	static bool compare_by_lap(std::unique_ptr<ClientSnapshot>& a, std::unique_ptr<ClientSnapshot>& b)
	{
		return a->lapsCompleted > b->lapsCompleted;
	}

	/**
	 * \brief A helper function for Server::WorkOutTrackPlacements, compares two clients to see who
	 * has passed the most checkpoints if their laps are the same
	 * \param a The first client
	 * \param b The second client
	 * \return True if a has passed more checkpoints than b
	 */
	static bool compare_by_check_points(std::unique_ptr<ClientSnapshot>& a, std::unique_ptr<ClientSnapshot>& b)
	{
		// Only compare checkpoints if the laps are the same
		if (a->lapsCompleted == b->lapsCompleted)
		{
			// Find the index of the highest lap completed
			const int aHighestCheckPoint = a->HighestCheckPointPassed();
			const int bHighestCheckPoint = b->HighestCheckPointPassed();
			return aHighestCheckPoint > bHighestCheckPoint;
		}
		return false;
	}

	/**
	 * \brief A helper function for Server::WorkOutTrackPlacements, compares two clients to see who
	 * out of the two is closest to their checkpoints if the laps and highest checkpoint passed is the
	 * same
	 * \param a The first client
	 * \param b The second client
	 * \return True if a is closer to the next checkpoint than b
	 */
	static bool compare_by_distance(std::unique_ptr<ClientSnapshot>& a, std::unique_ptr<ClientSnapshot>& b)
	{
		if (a->lapsCompleted == b->lapsCompleted && a->HighestCheckPointPassed() == b->HighestCheckPointPassed())
		{
			// Calculate the Distance to the next checkpoint
			const int highestCheckPoint = a->HighestCheckPointPassed();

			sf::Vector2f nextCheckpointPosition;

			if (highestCheckPoint == 5)
				nextCheckpointPosition = sf::Vector2f(LEVEL_CHECKPOINTS[highestCheckPoint + 1].left, LEVEL_CHECKPOINTS[highestCheckPoint].top);
			else
				nextCheckpointPosition = sf::Vector2f(LEVEL_CHECKPOINTS[0].left, LEVEL_CHECKPOINTS[0].top);

			// Find the distance to the next checkpoint
			sf::Vector2f aC = nextCheckpointPosition - a->position;
			sf::Vector2f bC = nextCheckpointPosition - b->position;

			// Find the magnitude of the vectors
			const float magAC = globals::sqr_magnitude(aC);
			const float magBC = globals::sqr_magnitude(bC);

			return magAC > magBC;

		}
		return false;
	}
} // anonymous namespace

std::unique_ptr<Server> Server::CreateServer(const unsigned short port)
{
	std::unique_ptr<Server> newServer(new Server());

	// Abide by the factory pattern, only return a value if it was initialised correctly
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
	// Attempt to initialise the server
	
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
	// See if the socket selector is ready to accept a new TCP socket
	if (m_socketSelector.isReady(m_listener))
	{
		// Find the next available colour for the players
		sf::Color colour = CAR_COLOURS[m_connectedClients.size()];

		// Find the next available starting position for the players
		sf::Vector2f startingPosition = STARTING_POSITIONS[m_connectedClients.size()];

		// Create a new connection
		auto* newClient = new ClientSnapshot(startingPosition, globals::cars::k_carStartingRotation);
		if (m_listener.accept(*newClient->socket) == sf::Socket::Done)
		{
			sf::Packet inPacket;
			TcpDataPacket inData;

			if (newClient->socket->receive(inPacket) == sf::Socket::Done)
			{
				inPacket >> inData;
			}

			if (m_connectedClients.size() < globals::game::k_playerAmount)
			{
				if (inData.m_type == eDataPacketType::e_FirstConnection)
				{
					if (!IsUsernameTaken(inData.m_userName) && inData.m_userName != globals::k_reservedServerUsername)
					{
						// To tell the client that they are successful
						sf::Packet outPacket;

						TcpDataPacket outData{
							eDataPacketType::e_UserNameConfirmation,
							globals::k_reservedServerUsername,
							startingPosition.x,
							startingPosition.y,
							globals::cars::k_carStartingRotation,
							colour
						};

						// Add the new client to the selector - this means we can update all clients
						m_socketSelector.add(*newClient->socket);

						std::cout << inData.m_userName << " has connected to the server" << std::endl;

						newClient->username = inData.m_userName;

						m_connectedClients.emplace_back(newClient);

						outPacket << outData;

						newClient->socket->send(outPacket);

						if(!BroadcastMessage({
							eDataPacketType::e_NewClient,
							globals::k_reservedServerUsername,
							startingPosition.x,
							startingPosition.y,
							globals::cars::k_carStartingRotation,
							colour }))
						{
							std::cout << "Failed to broadcast the new client to the connected clients" << std::endl;
						}
					} else
					{
						std::cout << "A CLIENT WITH THE USERNAME: " << inData.m_userName << " ALREADY EXISTS..." << std::endl;

						sf::Packet usernameRejectionPkt;
						TcpDataPacket usernameRejectionData(eDataPacketType::e_UserNameRejection, globals::k_reservedServerUsername);
						usernameRejectionPkt << usernameRejectionData;

						newClient->socket->send(usernameRejectionPkt);
						delete newClient;
					}
				}
			} else
			{
				std::cout << "MAXIMUM AMOUNT OF CLIENTS CONNECTED" << std::endl;

				sf::Packet maxClientMessagePkt;
				const TcpDataPacket maximumClientMessage(eDataPacketType::e_MaxPlayers, "SERVER");
				maxClientMessagePkt << maximumClientMessage;

				newClient->socket->send(maxClientMessagePkt);
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
		if (!racer->raceCompleted)
		{
			gameOver = false;
		}
	}

	return gameOver;
}

void Server::CheckCollisionsBetweenClients()
{
	// Start two loops to compare the clients to each other
	for (auto& client : m_connectedClients)
	{
		for (auto& otherClient : m_connectedClients)
		{
			// Ensure the clients aren't the same by checking their unique identifier
			if (client->username != otherClient->username)
			{
				// Calculate their distance from each other
				float dx = client->position.x - otherClient->position.x;
				float dy = client->position.y - otherClient->position.y;

				bool collisionOccurred = false;

				// While the magnitude of their vector distance is greater than the threshold...
				while (dx * dx + dy * dy < 8 * 10.f * 17.f)
				{
					// Set the flag
					collisionOccurred = true;

					// Try to resolve the collision
					client->position.x += dx / 10.f;
					client->position.x += dy / 10.f;
					otherClient->position.x -= dx / 10.f;
					otherClient->position.x -= dy / 10.f;
					dx = client->position.x - otherClient->position.x;
					dy = client->position.y - otherClient->position.y;

					// See if it is resolved
					if (static_cast<int>(dx) == 0 && static_cast<int>(dy) == 0)
						break;
				}

				// If a collision occurred and was resolved, update the clients
				if (collisionOccurred)
				{
					if(!SendMessage({ eDataPacketType::e_CollisionData, client->username, client->position, otherClient->username }, client->username))
					{
						std::cout << "Failed to send collision data to " << client->username << std::endl;
					}

					if(!SendMessage({ eDataPacketType::e_CollisionData, otherClient->username, otherClient->position, client->username }, otherClient->username))
					{
						std::cout << "Failed to send collision data to " << client->username << std::endl;
					}
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
		previousPositions.emplace_back(client->username);
	}

	// STEP 1 - CHECK THE LAPS OF THE PLAYERS
	std::sort(m_connectedClients.begin(), m_connectedClients.end(), compare_by_lap);

	// STEP 2 - CHECK WHICH CHECKPOINTS THEY HAVE PASSED
	std::sort(m_connectedClients.begin(), m_connectedClients.end(), compare_by_check_points);

	// STEP 3 - CHECK DISTANCE BETWEEN THEM AND THE NEXT CHECKPOINT
	std::sort(m_connectedClients.begin(), m_connectedClients.end(), compare_by_distance);

	// STEP 4 - SEE IF THE POSITIONS HAVE CHANGED
	bool positionsChanged = false;
	for (int i = 0; i < static_cast<int>(previousPositions.size()); ++i)
	{
		if (m_connectedClients[i]->username != previousPositions[i])
		{
			positionsChanged = true;
		}
	}

	if (positionsChanged)
	{
		// STEP 5 - TELL THE CLIENTS WHICH PLACE THEY ARE IN
		for (int i = 0; i < static_cast<int>(m_connectedClients.size()); ++i)
		{
			std::cout << "\tPosition " << i + 1 << " : " << m_connectedClients[i]->username << std::endl;
			if(!SendMessage({ eDataPacketType::e_Overtaken, globals::k_reservedServerUsername, i + 1 }, m_connectedClients[i]->username))
			{
				std::cout << "Failed to send a message to " << m_connectedClients[1]->username << std::endl;
			}
		}
	}

}

void Server::AIMovement(const float deltaTime, ClientSnapshot& client) const
{
	// Choose the AI's next checkpoint
	const sf::Vector2f target(
		LEVEL_CHECKPOINTS[client.nextAICheckpoint].left + globals::game::k_checkPointWidth / 2.f,
		LEVEL_CHECKPOINTS[client.nextAICheckpoint].top + globals::game::k_checkPointHeight / 2.f
	);

	// Calculate their rotation to the target
	const float beta = client.angle - atan2(target.x - client.position.x, -target.y + client.position.y);

	if (sin(beta) < 0)
	{
		client.angle += 3.14f * deltaTime;
	} else
	{
		client.angle -= 3.14f * deltaTime;
	}

	// Move toward the target
	client.position.x += sin(client.angle) * globals::cars::k_carTrackSpeed * deltaTime;
	client.position.y -= cos(client.angle) * globals::cars::k_carTrackSpeed * deltaTime;

	sf::Vector2f direction = target - client.position;

	// See if the client is close enought to the checkpoint
	if (globals::sqr_magnitude(direction) <
		globals::game::k_aiDistanceThreshold * globals::game::k_aiDistanceThreshold)
	{
		// If the threshold was met, increment the counter
		client.nextAICheckpoint++;
		if (client.nextAICheckpoint == globals::game::k_numCheckPoints)
		{
			// Reset if all checkpoints were reached
			client.nextAICheckpoint = 0;
		}
	}

	// Update the clients on the AI Move
	if(!BroadcastMessage({ eDataPacketType::e_UpdatePosition, client.username, client.position.x, client.position.y, client.angle }))
	{
		std::cout << "Failed to broadcast the message to all clients" << std::endl;
	}
}

int Server::FindClientIndex(const std::string& username) const
{
	for (int i = 0; i < static_cast<int>(m_connectedClients.size()); ++i)
	{
		if (m_connectedClients[i]->username == username)
		{
			return i;
		}
	}
	return -1;
}

bool Server::IsUsernameTaken(const std::string& username) const
{
	const bool isUserNameTaken = std::any_of(m_connectedClients.begin(), m_connectedClients.end(), [&username](const auto& client)->bool {
		return client->username == username;
		});

	return isUserNameTaken;
}

void Server::CheckIfClientHasPassedCheckPoint(ClientSnapshot& client)
{
	// Work out the bounding box of the client
	sf::RectangleShape clientRect({ globals::cars::k_carSpriteWidth, globals::cars::k_carSpriteHeight });
	clientRect.setOrigin(globals::cars::k_carOriginX, globals::cars::k_carOriginY);

	clientRect.setPosition(client.position);

	// See if the player has overlapped the checkpoints
	for (int i = 0; i < globals::game::k_numCheckPoints; ++i)
	{
		if (LEVEL_CHECKPOINTS[i].intersects(clientRect.getGlobalBounds()))
		{
			if (i == 0)
			{
				if (client.lapsCompleted == 0 && client.checkPointsPassed[globals::game::k_numCheckPoints - 1])
				{
					client.checkPointsPassed[0] = true;
				} else if (client.checkPointsPassed[globals::game::k_numCheckPoints - 1])
				{
					client.checkPointsPassed[0] = true;
				}
			} else
			{
				client.checkPointsPassed[i] = true;
			}

			// See if all the checkpoints have been passed
			if (client.AllCheckPointsPassed() && client.lapsCompleted != globals::game::k_totalLaps)
			{
				std::cout << client.username << " has completed a lap!" << std::endl;

				// Tell the client that they have completed a lap
				if(!SendMessage({ eDataPacketType::e_LapCompleted, globals::k_reservedServerUsername }, client.username))
				{
					std::cout << "Failed to tell: " << client.username << " that they completed a lap..." << std::endl;
				}

				client.lapsCompleted++;
				client.ResetCheckPoints();

				if (client.lapsCompleted == globals::game::k_totalLaps)
				{
					std::cout << client.username << " completed the race!" << std::endl;
					client.raceCompleted = true;

					// Tell the client that they completed the race
					if(!SendMessage({ eDataPacketType::e_RaceCompleted, globals::k_reservedServerUsername }, client.username))
					{
						std::cout << "Failed to tell: " << client.username << " that they completed the race..." << std::endl;
					}
				}
			}
		}
	}
}

bool Server::SendMessage(const TcpDataPacket& dataToSend, const std::string& receiver)
{
	sf::Packet outPacket;
	outPacket << dataToSend;

	m_connectedClients[FindClientIndex(receiver)]->socket->send(outPacket);

	return true;
}

void Server::Update(const float deltaTime)
{
	if (m_socketSelector.wait())
	{
		CheckForNewClients();

		if (m_connectedClients.size() == globals::game::k_playerAmount)
		{
			if (!m_gameInProgress)
			{
				m_gameInProgress = true;
				std::cout << globals::game::k_playerAmount << " have connected, starting the game..." << std::endl;
				if(!BroadcastMessage({ eDataPacketType::e_StartGame, globals::k_reservedServerUsername }))
				{
					std::cout << "Failed to tell all players the game is starting!" << std::endl;
				}
			}
		}

		// Loop through each client and use our new, fancy, socket selector
		for (auto& client : m_connectedClients)
		{
			if (client && client->socket && m_socketSelector.isReady(*client->socket))
			{
				sf::Packet inPacket;
				const auto clientStatus = client->socket->receive(inPacket);

				if (clientStatus == sf::Socket::Done)
				{
					TcpDataPacket inData;

					inPacket >> inData;

					switch (inData.m_type)
					{
					case eDataPacketType::e_UpdatePosition:

						client->position = { inData.m_x, inData.m_y };
						client->angle = inData.m_angle;

						if(BroadcastMessage(inData))
						{
							std::cout << "Error broadcasting messages to all clients" << std::endl;
						}
						
						CheckIfClientHasPassedCheckPoint(*client);
						
						WorkOutTrackPlacements();

						if (CheckGameOver())
						{
							// record the final race order
							std::vector<std::string> racePositions;
							for (const auto& racer : m_connectedClients)
							{
								std::cout << racer->username << std::endl;
								racePositions.emplace_back(racer->username);
							}

							if(!BroadcastMessage({ eDataPacketType::e_GameOver, globals::k_reservedServerUsername, racePositions }))
							{
								std::cout << "Failed to tell the players that the race has ended!" << std::endl;
							}
						}
						break;
					default:
						break;
					}
				}

				if (clientStatus == sf::Socket::Disconnected)
				{
					std::string disconnectedClientUsername = client->username;

					std::cout << "PLAYER WITH THE USERNAME: " << disconnectedClientUsername << " DISCONNECTED FROM THE SERVER" << std::endl;

					m_socketSelector.remove(*client->socket);

					client->socket->disconnect();

					// Remove from the vector
					const int clientIndex = FindClientIndex(client->username);

					m_connectedClients.erase(m_connectedClients.begin() + clientIndex);

					// See if it was the last person to leave
					if (m_connectedClients.empty())
					{
						m_gameInProgress = false;
					}

					// Tell the other clients that a client disconnected
					if(!BroadcastMessage({ eDataPacketType::e_ClientDisconnected, disconnectedClientUsername }))
					{
						std::cout << "Failed to tell all clients that another client disconnected" << std::endl;
					}
					return;
				}
			}

			if (client->raceCompleted)
			{
				AIMovement(deltaTime, *client);
			}
		}

		// Check collisions and update the clients accordingly...
		CheckCollisionsBetweenClients();
	}
}

bool Server::BroadcastMessage(const TcpDataPacket& dataToSend) const
{
	sf::Packet sendPacket;
	sendPacket << dataToSend;

	// update the other connected clients
	for (const auto& client : m_connectedClients)
	{
		// Make sure not to send the client their own data!
		//if (client->m_username != dataToSend.m_userName && !client->m_raceCompleted)
		{
			if (client->socket->send(sendPacket) != sf::Socket::Done)
			{
				std::cout << "Error sending message to " << client->username << std::endl;
				return false;
			}
		}
	}
	return true;
}
