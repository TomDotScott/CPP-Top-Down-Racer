#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

enum class ePacketType : uint8_t
{
	e_FirstConnection,
	e_UpdatePosition
};

class Client
{
public:
	static Client* CreateClient(const std::string& username, unsigned short port);

	void Update(float deltaTime, bool windowInFocus);
	
	void Render(sf::RenderWindow& window);

private:
	sf::IpAddress m_server;
	sf::TcpSocket m_socket;
	sf::RectangleShape m_shape;
	sf::RectangleShape m_playerTwoShape;
	float m_speed;
	sf::Vector2f m_previousPosition;
	std::string m_id;
	float m_packetDelay;
	float m_packetTimer;
	
	bool Initialise(unsigned short port);
	bool ReceiveMessage();
	bool SendMessage();
	Client(std::string username);
};