#include "Client.h"

#include <iostream>
#include <thread>
#include <utility>

#include "../Shared Files/Data.h"

Client* Client::CreateClient(const std::string& username, const unsigned short port)
{
	auto* c = new Client(username);

	if (c->Initialise(port))
	{
		std::cout << "Player " << username << " created successfully" << std::endl;
		return c;
	}

	std::cout << "ERROR WHILST CREATING THE PLAYER " << username << std::endl;
	delete c;
	return nullptr;
}

bool Client::Initialise(const unsigned short port)
{
	m_server = sf::IpAddress::getLocalAddress();

	// Connect to the server
	if (m_socket.connect(m_server, port) != sf::Socket::Done)
	{
		std::cout << "Unable to connect to the server at address: " << m_server << std::endl;
		return false;
	}

	sf::Packet packet;

	const DataPacket dp(FIRST_CONNECTION, m_userName);

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

	std::cout << "The server told me I am player number: " << m_playerNumber << std::endl;

	return true;
}

void Client::Update(const float deltaTime, const bool windowInFocus)
{
	bool playerMoved = false;

	if (windowInFocus)
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
	}

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

	if (dp.m_type == UPDATE_POSITION)
	{
		m_players[dp.m_playerNum].setPosition(dp.m_x, dp.m_y);
	}

	//if (code == static_cast<uint8_t>(ePacketType::e_UpdatePosition))
	//{
	//	std::cout << "Updating player two's position" << std::endl;

	//	// Update player 2's position based on the data received from the server
	//	const sf::Vector2f p2Pos(x, y);

	//	m_playerTwoShape.setPosition(p2Pos);
	//}

	return true;
}

bool Client::SendMessage()
{
	// Push some data to the packet
	const sf::Vector2f& position = m_players[m_playerNumber].getPosition();

	sf::Packet packet;
	const DataPacket dp(UPDATE_POSITION, m_userName, m_playerNumber, position.x, position.y);

	packet << dp;

	if (m_socket.send(packet) != sf::Socket::Done)
	{
		return false;
	}

	return true;
}

Client::Client(std::string username) :
	m_playerNumber(0),
	m_speed(100.f),
	m_userName(std::move(username)),
	m_packetDelay(0.033f),
	m_packetTimer(0.f)
{
	sf::RectangleShape shape;
	shape.setFillColor(sf::Color::Red);
	m_players.push_back(shape);

	shape.setFillColor(sf::Color::Blue);
	m_players.push_back(shape);

	shape.setFillColor(sf::Color::Yellow);
	m_players.push_back(shape);

	shape.setFillColor(sf::Color::Magenta);
	m_players.push_back(shape);

	shape.setFillColor(sf::Color::Cyan);
	m_players.push_back(shape);

	shape.setFillColor(sf::Color::White);
	m_players.push_back(shape);

	shape.setFillColor(sf::Color::Green);
	m_players.push_back(shape);

	shape.setFillColor({ 255, 165, 0 });
	m_players.push_back(shape);

	for (int i = 0; i < 8; ++i)
	{
		m_players[i].setSize({ 50.f, 50.f });
		m_players[i].setPosition(i * 50.f, 300.f);
	}
}
