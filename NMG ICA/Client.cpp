#include "Client.h"

#include <iostream>
#include <thread>
#include <utility>

#include "../Shared Files/Data.h"

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
	if (!m_texture.loadFromFile("images/car.png"))
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

	if (inDataPacket.m_type == eDataPacketType::e_UserNameConfirmation)
	{
		std::cout << "Username confirmed, client connected" << std::endl;
	}

	std::cout << "The server told me I am: " << static_cast<int>(inDataPacket.m_red) << " " << static_cast<int>(inDataPacket.m_green) << " " << static_cast<int>(inDataPacket.m_blue) << std::endl;

	AddPlayer(m_userName);
	
	m_players[m_userName].SetColour(
		{
			static_cast<sf::Uint8>(inDataPacket.m_red),
			static_cast<sf::Uint8>(inDataPacket.m_green),
			static_cast<sf::Uint8>(inDataPacket.m_blue)
		}
	);

	return true;
}


void Client::Update(const float deltaTime)
{
	m_players[m_userName].Update(deltaTime);

	// TODO: Make the packet timer responsive, so that different internet speeds are accounted for
	m_packetTimer += deltaTime;

	if (m_packetTimer >= m_packetDelay)
	{
		SendMessage();
		m_packetTimer = 0.f;
	}

	ReceiveMessage();
	m_playerMoved = false;
}

void Client::Render(sf::RenderWindow& window)
{
	for (auto& player : m_players)
	{
		player.second.Render(window);
	}
}

bool Client::AddPlayer(const std::string& username)
{
	if(!globals::is_value_in_map(m_players, username))
	{
		std::cout << "Adding: " << username << " to the map..." << std::endl;
		m_players.insert(std::make_pair(username, Player(m_texture)));

		std::cout << "The new map size is: " << m_players.size() << std::endl;
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
	default:;
	}


	return true;
}

bool Client::SendMessage()
{
	// Push some data to the packet
	const auto& playerPosition = m_players[m_userName].GetPosition();
	const auto playerAngle = m_players[m_userName].GetAngle();
	const auto playerColour = m_players[m_userName].GetColour();

	sf::Packet outPacket;
	const DataPacket outDataPacket(eDataPacketType::e_UpdatePosition, m_userName, playerColour, playerPosition.x, playerPosition.y, playerAngle);

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
	m_playerMoved(false)
{

}

void Client::Input(const float deltaTime)
{
	m_playerMoved = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		m_players[m_userName].ChangeAngle(-3.14f * deltaTime);
		m_playerMoved = true;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		m_players[m_userName].ChangeAngle(3.14f * deltaTime);
		m_playerMoved = true;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		m_players[m_userName].ChangeVelocity(0, -globals::k_carSpeed * deltaTime);
		m_playerMoved = true;
	} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		m_players[m_userName].ChangeVelocity(0, globals::k_carSpeed * deltaTime);
		m_playerMoved = true;
	}
}