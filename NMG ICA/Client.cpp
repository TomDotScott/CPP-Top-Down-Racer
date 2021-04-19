#include "Client.h"

#include <iostream>
#include <thread>

Client* Client::CreateClient(const unsigned short port)
{
	auto* c = new Client();

	if (c->Initialise(port))
	{
		c->m_socket.setBlocking(false);
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

	std::cout << "Client connected to server " << m_server << std::endl;
	return true;
}

void Client::Update(const float deltaTime)
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


	SendMessage();

	ReceiveMessage();
}

void Client::Render(sf::RenderWindow& window)
{
	window.draw(m_shape);
}

bool Client::ReceiveMessage()
{
	//// Receive a message from the server
	char in[128];
	std::size_t received;
	if (m_socket.receive(in, sizeof(in), received) != sf::Socket::Done)
		return false;
	std::cout << "Message received from the server: \"" << in << "\"" << std::endl;
	return true;
}

bool Client::SendMessage()
{
	sf::Packet packet;

	// Push some data to the packet
	const sf::Vector2f& position = m_shape.getPosition();

	std::cout << "Shape is at: " << position.x << ", " << position.y << std::endl;
	
	packet << position.x << position.y;

	if(m_socket.send(packet) != sf::Socket::Done)
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
	
	std::cout << "Packet sent to the server measuring: " << packet.getDataSize() << " bytes... " << std::endl;
	return true;
}

Client::Client() :
	m_shape({ 50.f, 50.f }),
	m_speed(100.f)
{
	m_shape.setFillColor(sf::Color::Blue);
	m_shape.setPosition({ 100.f, 250.f });
}
