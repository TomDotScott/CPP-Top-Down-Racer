#include "Client.h"

#include <iostream>
#include <thread>
#include <utility>

Client* Client::CreateClient(const std::string& username, const unsigned short port)
{
	auto* c = new Client(username);

	if (c->Initialise(port))
	{
		return c;
	} else
	{
		delete c;
		return nullptr;
	}
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

	sf::Packet id;
	id << static_cast<uint8_t>(ePacketType::e_FirstConnection) << m_id;

	m_socket.send(id);
	m_socket.setBlocking(false);

	std::cout << m_id << " connected to server " << m_server << std::endl;
	return true;
}

void Client::Update(const float deltaTime, const bool windowInFocus)
{
	if (windowInFocus)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			m_shape.move({ 0.f, -m_speed * deltaTime });
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			m_shape.move({ 0.f, m_speed * deltaTime });
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			m_shape.move({ -m_speed * deltaTime, 0.f });
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			m_shape.move({ m_speed * deltaTime, 0.f });
		}
	}

	// TODO: Make the packet timer responsive, so that different internet speeds are accounted for
	m_packetTimer += deltaTime;

	std::cout << m_packetTimer << std::endl;

	if (m_packetTimer >= m_packetDelay)
	{
		std::cout << "Sending packet to server" << std::endl;
		
		// If the position was the same as the last frame, don't send the packets
		if (m_shape.getPosition() != m_previousPosition)
		{
			SendMessage();
			m_previousPosition = m_shape.getPosition();
		}

		m_packetTimer = 0.f;
	}

	ReceiveMessage();

}

void Client::Render(sf::RenderWindow& window)
{
	window.draw(m_shape);
	window.draw(m_playerTwoShape);
}

bool Client::ReceiveMessage()
{
	sf::Packet inPacket;
	if (m_socket.receive(inPacket) != sf::Socket::Done)
		return false;

	uint8_t code{};
	std::string id;
	float x, y;

	inPacket >> code >> id >> x >> y;

	if (code == static_cast<uint8_t>(ePacketType::e_UpdatePosition))
	{
		std::cout << "Updating player two's position" << std::endl;

		// Update player 2's position based on the data received from the server
		const sf::Vector2f p2Pos(x, y);

		m_playerTwoShape.setPosition(p2Pos);
	}

	return true;

	//// Receive a message from the server
	//char in[128];
	//std::size_t received;
	//if (m_socket.receive(in, sizeof(in), received) != sf::Socket::Done)
	//	return false;
	//std::cout << "Message received from the server: \"" << in << "\"" << std::endl;
	//return true;
}

bool Client::SendMessage()
{
	sf::Packet packet;

	// Push some data to the packet
	const sf::Vector2f& position = m_shape.getPosition();

	//std::cout << "Shape is at: " << position.x << ", " << position.y << std::endl;

	packet << static_cast<uint8_t>(ePacketType::e_UpdatePosition) << m_id << position.x << position.y;

	if (m_socket.send(packet) != sf::Socket::Done)
	{
		return false;
	}

	/*
	// Send a message to the server
	std::cout << "Send a message: ";

	std::string message;
	std::cin >> message;

	if (m_socket.send(message.c_str(), sizeof(message.c_str())) != sf::Socket::Done)
		return false;

	std::cout << "Message sent to the server: '" << message << "'\tMeasuring: " << sizeof(message.c_str()) << " bytes... " << std::endl;*/

	//std::cout << "Packet sent to the server measuring: " << packet.getDataSize() << " bytes... " << std::endl;

	return true;
}

Client::Client(std::string username) :
	m_shape({ 50.f, 50.f }),
	m_playerTwoShape({ 50.f, 50.f }),
	m_speed(100.f),
	m_id(std::move(username)),
	m_packetDelay(0.033f),
	m_packetTimer(0.f)
{
	m_shape.setFillColor(sf::Color::Blue);
	m_shape.setPosition({ 100.f, 250.f });

	m_playerTwoShape.setFillColor(sf::Color::Red);
	m_playerTwoShape.setPosition({ 100.f, 250.f });
}
