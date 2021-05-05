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

	m_sprite.setTexture(m_texture);

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

	return true;
}

void Client::Update(const float deltaTime, const bool windowInFocus)
{
	m_players[m_userName].Update(deltaTime);

	// TODO: Make the packet timer responsive, so that different internet speeds are accounted for
	m_packetTimer += deltaTime;

	if (m_packetTimer >= m_packetDelay)
	{
		// If the position was the same as the last frame, don't send the packets
		if (m_playerMoved)
		{
			SendMessage();
		}

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

bool Client::ReceiveMessage()
{
	sf::Packet inPacket;

	if (m_socket.receive(inPacket) != sf::Socket::Done)
		return false;

	DataPacket inData;

	inPacket >> inData;

	std::cout << "Received a packet from " << inData.m_userName << std::endl;

	switch (inData.m_type)
	{
	case eDataPacketType::e_None: break;
	case eDataPacketType::e_FirstConnection: break;
	case eDataPacketType::e_UserNameConfirmation: break;
	case eDataPacketType::e_UserNameRejection: break;
	case eDataPacketType::e_NewClient:
		std::cout << "The server told me a new client connected with the username" << inData.m_userName << std::endl;
		m_players.insert(std::make_pair(inData.m_userName, Player()));
		std::cout << m_players.size() << std::endl;
		break;
	case eDataPacketType::e_MaxPlayers: break;
	case eDataPacketType::e_UpdatePosition:
		m_players[inData.m_userName].SetPosition({ inData.m_x, inData.m_y });
		break;
	default:;
	}


	return true;
}

bool Client::SendMessage()
{
	// Push some data to the packet
	const sf::Vector2f& playerPosition = m_players[m_userName].GetPosition();
	const float playerAngle = m_players[m_userName].GetAngle();

	sf::Packet outPacket;
	const DataPacket outDataPacket(eDataPacketType::e_UpdatePosition, m_userName, playerPosition.x, playerPosition.y, playerAngle);

	outPacket << outDataPacket;

	if (m_socket.send(outPacket) != sf::Socket::Done)
	{
		return false;
	}

	return true;
}

Client::Client(const std::string& username) :
	m_userName(username),
	m_packetDelay(0.033f),
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