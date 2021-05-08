#include "Client.h"

#include <iostream>
#include <thread>
#include <utility>

std::unique_ptr<Client> Client::CreateClient(const std::string& username, const unsigned short port)
{
	std::unique_ptr<Client> newClient(new Client(username));

	if (newClient->Initialise(port))
	{
		std::cout << "Player " << username << " created successfully" << std::endl;
		return newClient;
	}

	std::cout << "ERROR WHILST CREATING THE PLAYER " << username << std::endl;
	return nullptr;
}

bool Client::Initialise(const unsigned short port)
{
	if (!m_carTexture.loadFromFile("images/car.png"))
	{
		return false;
	}
	
	m_server = sf::IpAddress::getLocalAddress();

	// Connect to the server
	if (m_socket.connect(m_server, port) != sf::Socket::Done)
	{
		std::cout << "Unable to connect to the server at address: " << m_server << std::endl;
		return false;
	}

	sf::Packet outPacket;

	const DataPacket firstConnectionDataPacket(eDataPacketType::e_FirstConnection, m_userName);

	outPacket << firstConnectionDataPacket;

	m_socket.send(outPacket);
	m_socket.setBlocking(false);

	std::cout << m_userName << " connected to server " << m_server << std::endl;

	sf::Packet inPacket;

	sf::Clock clock{};
	float elapsedTime = 0.f;
	while (m_socket.receive(inPacket) != sf::Socket::Done)
	{
		elapsedTime += clock.restart().asSeconds();

		std::cout << " waiting for confirmation " << elapsedTime << std::endl;

		if (elapsedTime >= 10.f)
		{
			std::cout << "Timeout! " << std::endl;
			return false;
		}
	}

	DataPacket inDataPacket;
	inPacket >> inDataPacket;

	switch (inDataPacket.m_type)
	{
	case eDataPacketType::e_UserNameConfirmation:
		std::cout << "Username confirmed, client connected" << std::endl;
		
		std::cout << "The server told me I am: " << static_cast<int>(inDataPacket.m_red) << " " << static_cast<int>(inDataPacket.m_green) << " " << static_cast<int>(inDataPacket.m_blue) << std::endl;

		AddPlayer(m_userName);

		m_players[m_userName].SetColour(
			{
				static_cast<sf::Uint8>(inDataPacket.m_red),
				static_cast<sf::Uint8>(inDataPacket.m_green),
				static_cast<sf::Uint8>(inDataPacket.m_blue)
			}
		);
		break;
	case eDataPacketType::e_UserNameRejection:
		std::cout << "The username is taken, try again" << std::endl;
		return false;
	}
	return true;
}

void Client::CheckCollisions()
{
	// Loop through clients and see if any of them overlap
	for(auto& player : m_players)
	{
		// Don't check collisions with themselves...
		if(player.first != m_userName)
		{
			auto playerGB = m_players[m_userName].GetGlobalBounds();
			auto otherPlayerGB = player.second.GetGlobalBounds();
			
			if(playerGB.intersects(otherPlayerGB))
			{
				// Find how much they overlap
				
			}
		}
	}
}

void Client::Update(const float deltaTime)
{
	if (m_gameStarted)
	{
		m_players[m_userName].Update(deltaTime);

		for(auto& player : m_players)
		{
			m_background.CheckCollisions(player.second);
		}
		
		// TODO: Make the packet timer responsive, so that different internet speeds are accounted for
		m_packetTimer += deltaTime;

		if (m_packetTimer >= m_packetDelay)
		{
			SendMessage(eDataPacketType::e_UpdatePosition);
			m_packetTimer = 0.f;
		}
	}
	
	ReceiveMessage();
	m_playerMoved = false;
}

void Client::Render(sf::RenderWindow& window)
{
	if (!m_gameStarted)
	{
		m_text.setPosition(
			static_cast<float>(globals::k_screenWidth) / 2.f - m_text.getGlobalBounds().width / 2,
			static_cast<float>(globals::k_screenHeight) / 2.f - m_text.getGlobalBounds().height
		);
		
		window.draw(m_text);
	}else
	{
		m_background.Render(window);
		
		for (auto& player : m_players)
		{
			// Render username above the player's car
			m_text.setString(player.first);
			
			m_text.setCharacterSize(15);
			
			const sf::Vector2f& playerPosition = player.second.GetPosition();
			
			m_text.setPosition(playerPosition.x - m_text.getGlobalBounds().width / 2, playerPosition.y - m_text.getGlobalBounds().height - 30);

			window.draw(m_text);

			player.second.Render(window);
		}
	}
}

void Client::Input(const float deltaTime)
{
	m_players[m_userName].Input(deltaTime);
}

bool Client::AddPlayer(const std::string& username)
{
	if (username != "SERVER")
	{
		if (!globals::is_value_in_map(m_players, username))
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
	if(globals::is_value_in_map(m_players, username))
	{
		m_players.erase(username);
		return true;
	}

	return false;
}

bool Client::ReceiveMessage()
{
	sf::Packet inPacket;

	if (m_socket.receive(inPacket) != sf::Socket::Done)
		return false;

	DataPacket inData;

	inPacket >> inData;

	// std::cout << "Received a packet from " << inData.m_userName << std::endl;

	// See if the data is from a new client...
	if (AddPlayer(inData.m_userName))
	{
		std::cout << "A new client connected with the username: " << inData.m_userName << std::endl;

		m_players[inData.m_userName].SetColour(
			{
				static_cast<sf::Uint8>(inData.m_red),
				static_cast<sf::Uint8>(inData.m_green),
				static_cast<sf::Uint8>(inData.m_blue)
			}
		);

		std::cout << "The server told me the new clients are: " << static_cast<int>(inData.m_red) << " " << static_cast<int>(inData.m_green) << " " << static_cast<int>(inData.m_blue) << std::endl;
	}

	switch (inData.m_type)
	{
	case eDataPacketType::e_None: break;
	case eDataPacketType::e_FirstConnection: break;
	case eDataPacketType::e_UserNameConfirmation: break;
	case eDataPacketType::e_UserNameRejection: break;
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
		if(RemovePlayer(inData.m_userName))
		{
			std::cout << "The disconnected player: " << inData.m_userName << " was removed successfully..." << std::endl;
		}else
		{
			std::cout << "There was an error trying to remove " << inData.m_userName << "\n\t They may have been removed already...." << std::endl;
		}
		break;
	default:;
	}


	return true;
}

bool Client::SendMessage(const eDataPacketType type)
{
	// Push some data to the packet
	const auto& playerPosition = m_players[m_userName].GetPosition();
	sf::Packet outPacket;
	
	const DataPacket outDataPacket(
		type, 
		m_userName, 
		playerPosition.x, 
		playerPosition.y, 
		m_players[m_userName].GetAngle(),
		m_players[m_userName].GetColour()
	);

	outPacket << outDataPacket;

	if (m_socket.send(outPacket) != sf::Socket::Done)
	{
		return false;
	}

	return true;
}

Client::Client(const std::string& username) :
	m_userName(username),
	m_packetDelay(0.05f),
	m_packetTimer(0.f),
	m_playerMoved(false),
	m_gameStarted(false)
{
	m_text.setString("Waiting for other players\nto connect...");
	m_text.setCharacterSize(60);
}

bool Client::Ready() const
{
	return m_gameStarted;
}

void Client::SetGameFont(const sf::Font& font)
{
	m_text.setFont(font);
}
