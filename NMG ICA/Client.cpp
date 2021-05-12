#include "Client.h"

#include <iostream>
#include <thread>
#include <utility>

#include "Globals.h"

std::unique_ptr<Client> Client::CreateClient(const std::string& username, const unsigned short port)
{
	// Create a new client object
	std::unique_ptr<Client> newClient(new Client(username));

	if (newClient->Initialise(port))
	{
		// abide to the factory pattern - only release an object if it was initialised successfully

		std::cout << "Player " << username << " created successfully" << std::endl;

		return newClient;
	}

	std::cout << "ERROR WHILST CREATING THE PLAYER " << username << std::endl;
	return nullptr;
}

bool Client::Initialise(const unsigned short port)
{
	// Load game files
	if (!m_carTexture.loadFromFile("images/car.png"))
	{
		return false;
	}

	// Connect to the server
	if (m_socket.connect(sf::IpAddress::getLocalAddress(), port) != sf::Socket::Done)
	{
		std::cout << "Unable to connect to the server at address: " << sf::IpAddress::getLocalAddress() << std::endl;
		return false;
	}

	std::cout << m_userName << " connected to server " << sf::IpAddress::getLocalAddress() << std::endl;

	// Now that we've connected, make first contact with the server
	sf::Packet outPacket;
	const TcpDataPacket firstConnectionDataPacket(eDataPacketType::e_FirstConnection, m_userName);
	outPacket << firstConnectionDataPacket;

	m_socket.send(outPacket);

	// Set the socket to false so it doesn't block the main thread
	m_socket.setBlocking(false);


	// See if the server has confirmed that the username is available...
	sf::Packet inPacket;

	sf::Clock clock{};
	float elapsedTime = 0.f;
	while (m_socket.receive(inPacket) != sf::Socket::Done)
	{
		elapsedTime += clock.restart().asSeconds();

		std::cout << " waiting for confirmation " << elapsedTime << std::endl;

		// So that we're not waiting forever, timeout after 10 seconds
		if (elapsedTime >= 10.f)
		{
			std::cout << "Timeout! " << std::endl;
			return false;
		}
	}


	// See what the server sent back to us
	TcpDataPacket inDataPacket;
	inPacket >> inDataPacket;

	switch (inDataPacket.m_type)
	{
		// Success...
	case eDataPacketType::e_UserNameConfirmation:
		std::cout << "Username confirmed, client connected" << std::endl;

		AddPlayer(m_userName);

		m_players[m_userName].SetColour(
			{
				static_cast<sf::Uint8>(inDataPacket.m_red),
				static_cast<sf::Uint8>(inDataPacket.m_green),
				static_cast<sf::Uint8>(inDataPacket.m_blue)
			}
		);

		m_players[m_userName].SetPosition(
			{
				inDataPacket.m_x,
				inDataPacket.m_y
			}
		);

		m_players[m_userName].SetAngle(inDataPacket.m_angle);

		break;
		// Failure...
	case eDataPacketType::e_UserNameRejection:
		std::cout << "The username is taken, try again" << std::endl;
		return false;
	}

	// If everything was set up okay, we have a complete client
	return true;
}

void Client::Update(const float deltaTime)
{
	// We only want to send messages to the server if the game is in action
	if (m_gameStarted && !m_completedRace)
	{
		m_players[m_userName].Update(deltaTime);
		m_background.CheckCollisions(m_players[m_userName]);

		m_packetTimer += deltaTime;

		// Don't send packets every frame as it puts a LOT of stress on the server
		if (m_packetTimer >= m_packetDelay)
		{
			SendMessage(eDataPacketType::e_UpdatePosition);
			m_packetTimer = 0.f;
		}
	}

	// But we want to receive all the time, incase a client connects or disconnects
	ReceiveMessage();
}

void Client::Render(sf::RenderWindow& window)
{
	// Reset the text colour in-case it changed
	m_text.setFillColor(sf::Color::White);

	if (m_gameStarted)
	{
		if (m_gameOver)
		{
			// Display the final order of the racers
			for (int i = 0; i < static_cast<int>(m_finalPlayerOrder.size()); ++i)
			{
				std::string stringToDisplay;
				switch (i)
				{
				case 0:
					stringToDisplay = "1st: ";
					break;
				case 1:
					stringToDisplay = "2nd: ";
					break;
				case 2:
					stringToDisplay = "3rd: ";
					break;
				case 3:
					stringToDisplay = "4th: ";
					break;
				default:
					break;

				}

				// Highlight the player with green text
				m_text.setFillColor(m_finalPlayerOrder[i] == m_userName ? sf::Color::Green : sf::Color::White);

				m_text.setString(stringToDisplay + m_finalPlayerOrder[i]);

				// Offset the text from each other
				m_text.setPosition(
					static_cast<float>(globals::game::k_screenWidth) / 2.f - m_text.getGlobalBounds().width / 2,
					static_cast<float>(globals::game::k_screenHeight) / 2.f + static_cast<float>(i) * m_text.getGlobalBounds().height + 20.f
				);

				window.draw(m_text);
			}
		} else
		{
			m_background.Render(window);

			// Structured binding for the players in the map because C++17 is cool

			for (auto& [username, player] : m_players)
			{
				// Render username above the player's car
				m_text.setString(username);

				m_text.setCharacterSize(15);

				const sf::Vector2f& playerPosition = player.GetPosition();

				m_text.setPosition(playerPosition.x - m_text.getGlobalBounds().width / 2, playerPosition.y - m_text.getGlobalBounds().height - 30);

				window.draw(m_text);

				player.Render(window);
			}

			// Draw the UI last

			m_text.setString("Laps: " + std::to_string(m_lapsCompleted + 1) + " / " + std::to_string(globals::game::k_totalLaps));

			m_text.setCharacterSize(30);
			m_text.setPosition(730.f, 25.f);
			window.draw(m_text);

			m_text.setString("Pos: " + std::to_string(m_positionInRace) + " / " + std::to_string(m_players.size()));
			m_text.setPosition(730.f, 75.f);
			window.draw(m_text);
		}
	} else
	{
		m_text.setPosition(
			static_cast<float>(globals::game::k_screenWidth) / 2.f - m_text.getGlobalBounds().width / 2,
			static_cast<float>(globals::game::k_screenHeight) / 2.f - m_text.getGlobalBounds().height
		);

		window.draw(m_text);
	}
}

void Client::Input(const float deltaTime)
{
	// Grab input from the keyboard and deal with it accordingly

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		m_players[m_userName].ChangeAngle(-3.14f * deltaTime);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		m_players[m_userName].ChangeAngle(3.14f * deltaTime);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		m_players[m_userName].ChangeVelocity(0, -m_players[m_userName].GetSpeed() * deltaTime);
	} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		m_players[m_userName].ChangeVelocity(0, m_players[m_userName].GetSpeed() * deltaTime);
	}
}

bool Client::AddPlayer(const std::string& username)
{
	// Ensure that the username that is being added isn't the reserved one from the username
	if (username != globals::k_reservedServerUsername)
	{
		// If the key isn't already in the map, add it
		if (!globals::is_key_in_map(m_players, username))
		{
			std::cout << "Adding: " << username << " to the map..." << std::endl;
			m_players.insert(std::make_pair(username, Player(m_carTexture)));

			std::cout << "The new map size is: " << m_players.size() << std::endl;
			return true;
		}
	}
	return false;
}

bool Client::RemovePlayer(const std::string& username)
{
	// Only remove the username if it is valid
	if (globals::is_key_in_map(m_players, username))
	{
		m_players.erase(username);
		return true;
	}

	return false;
}

bool Client::ReceiveMessage()
{
	sf::Packet inPacket;

	// Receive some data from the server
	if (m_socket.receive(inPacket) != sf::Socket::Done)
		return false;

	// Create a TcpDataPacket object to store the data received
	TcpDataPacket inData;

	inPacket >> inData;

	// See if the data is from a new client...
	if (AddPlayer(inData.m_userName))
	{
		std::cout << "A new client connected with the username: " << inData.m_userName << std::endl;

		m_players[inData.m_userName].SetColour({
				static_cast<sf::Uint8>(inData.m_red),
				static_cast<sf::Uint8>(inData.m_green),
				static_cast<sf::Uint8>(inData.m_blue)
			});

		m_players[inData.m_userName].SetPosition({
				inData.m_x,
				inData.m_y
			});

		m_players[inData.m_userName].SetAngle(inData.m_angle);
	}

	// Deal with the rest of the data received
	switch (inData.m_type)
	{
	case eDataPacketType::e_MaxPlayers:
		std::cout << "The server told me that there are the max amount of players in the game already..." << std::endl;
		break;

		
	case eDataPacketType::e_UpdatePosition:
		m_players[inData.m_userName].SetPosition({ inData.m_x, inData.m_y });
		m_players[inData.m_userName].SetAngle(inData.m_angle);
		break;

		
	case eDataPacketType::e_StartGame:
		std::cout << "The server told me that the game has started..." << std::endl;
		m_gameStarted = true;
		break;

		
	case eDataPacketType::e_ClientDisconnected:
		std::cout << "The server told me that the player: " << inData.m_userName << " disconnected..." << std::endl;
		if (RemovePlayer(inData.m_userName))
		{
			std::cout << "The disconnected player: " << inData.m_userName << " was removed successfully..." << std::endl;
		} else
		{
			std::cout << "There was an error trying to remove " << inData.m_userName << "\n\t They may have been removed already...." << std::endl;
		}
		break;

		
	case eDataPacketType::e_CollisionData:
		std::cout << "The server told me that player: " << inData.m_playerCollidedWith << " collided with me" << std::endl;

		m_players[inData.m_userName].SetPosition({ inData.m_x, inData.m_y });

		break;

		
	case eDataPacketType::e_LapCompleted:
		std::cout << "The server told me that I completed a lap!" << std::endl;

		m_lapsCompleted++;
		break;

		
	case eDataPacketType::e_Overtaken:
		std::cout << "The server updated me on my position in the race!" << std::endl;

		m_positionInRace = inData.m_positionInRace;
		break;

		
	case eDataPacketType::e_RaceCompleted:
		std::cout << "The server told me I have completed the race: " << std::endl;
		m_completedRace = true;
		break;

		
	case eDataPacketType::e_GameOver:
		std::cout << "The server told me that the game has finished and the final positions are: " << std::endl;

		for (int i = 0; i < static_cast<int>(inData.m_placementOrder.m_racePositions.size()); ++i)
		{
			std::cout << i + 1 << ": " << inData.m_placementOrder.m_racePositions[i] << std::endl;
		}

		m_finalPlayerOrder = inData.m_placementOrder.m_racePositions;

		m_gameOver = true;
		break;

		
	default:
		break;
	}

	return true;
}

bool Client::SendMessage(const eDataPacketType type)
{
	// Push some data to the packet
	const auto& playerPosition = m_players[m_userName].GetPosition();
	sf::Packet outPacket;

	const TcpDataPacket outDataPacket(
		type,
		m_userName,
		playerPosition.x,
		playerPosition.y,
		m_players[m_userName].GetAngle(),
		m_players[m_userName].GetColour()
	);

	outPacket << outDataPacket;

	// Send the packet to the server via the socket 
	if (m_socket.send(outPacket) != sf::Socket::Done)
	{
		return false;
	}

	return true;
}

bool Client::SendMessage(TcpDataPacket& dp)
{
	sf::Packet outPacket;

	outPacket << dp;

	// Send the packet to the server via the socket
	if (m_socket.send(outPacket) != sf::Socket::Done)
	{
		return false;
	}

	return true;
}

Client::Client(std::string username) :
	m_userName(std::move(username)),
	m_packetDelay(0.05f),
	m_packetTimer(0.f),
	m_gameStarted(false),
	m_completedRace(false),
	m_gameOver(false),
	m_lapsCompleted(0),
	m_positionInRace(0)
{
	m_text.setString("Waiting for other players\nto connect...");
	m_text.setCharacterSize(60);
}

void Client::SetGameFont(const sf::Font& font)
{
	m_text.setFont(font);
}
