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

	sf::Packet packet;

	const DataPacket dp(eDataPacketType::e_FirstConnection, m_userName);

	packet << dp;

	m_socket.send(packet);
	m_socket.setBlocking(false);

	std::cout << m_userName << " connected to server " << m_server << std::endl;

	sf::Packet p;

	sf::Clock clock{};
	float elapsedTime = 0.f;
	while (m_socket.receive(p) != sf::Socket::Done)
	{
		elapsedTime += clock.restart().asSeconds();

		std::cout << " waiting for the player number... " << elapsedTime << std::endl;

		if (elapsedTime >= 10.f)
		{
			std::cout << "Timeout! " << std::endl;
			return false;
		}
	}

	p >> m_playerNumber;

	std::cout << "The server told me I am player number: " << static_cast<int>(m_playerNumber) << std::endl;

	return true;
}

void Client::Update(const float deltaTime, const bool windowInFocus)
{
	bool playerMoved = false;

	/*if (windowInFocus)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			m_players[m_playerNumber].move({ 0.f, -m_speed * deltaTime });
			playerMoved = true;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			m_players[m_playerNumber].move({ 0.f, m_speed * deltaTime });
			playerMoved = true;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			m_players[m_playerNumber].move({ -m_speed * deltaTime, 0.f });
			playerMoved = true;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			m_players[m_playerNumber].move({ m_speed * deltaTime, 0.f });
			playerMoved = true;
		}
	}*/

	// TODO: Make the packet timer responsive, so that different internet speeds are accounted for
	m_packetTimer += deltaTime;

	if (m_packetTimer >= m_packetDelay)
	{
		// If the position was the same as the last frame, don't send the packets
		if (playerMoved)
		{
			SendMessage();
		}

		m_packetTimer = 0.f;
	}

	ReceiveMessage();
}

void Client::Render(sf::RenderWindow& window)
{
	for (const auto& shape : m_players)
	{
		
		window.draw(shape);
	}
}

bool Client::ReceiveMessage()
{
	sf::Packet inPacket;
	if (m_socket.receive(inPacket) != sf::Socket::Done)
		return false;

	DataPacket dp;

	inPacket >> dp;

	std::cout << "Received a packet from " << dp.m_userName << std::endl;

	if (dp.m_type == eDataPacketType::e_UpdatePosition)
	{
		m_players[dp.m_playerNum].setPosition(dp.m_x, dp.m_y);
	}

	
	return true;
}

bool Client::SendMessage()
{
	// Push some data to the packet
	const sf::Vector2f& position = m_players[m_playerNumber].getPosition();

	sf::Packet packet;
	const DataPacket dp(eDataPacketType::e_UpdatePosition, m_userName, m_playerNumber, position.x, position.y);

	packet << dp;

	if (m_socket.send(packet) != sf::Socket::Done)
	{
		return false;
	}

	return true;
}

Client::Client(const std::string& username) :
	m_userName(username),
	m_packetDelay(0.033f),
	m_packetTimer(0.f),
	m_playerNumber(0)
{
	for (int i = 0; i < 8; ++i)
	{
		m_players[i].setSize({ 50.f, 50.f });
		m_players[i].setPosition(i * 50.f, 300.f);
	}
}
