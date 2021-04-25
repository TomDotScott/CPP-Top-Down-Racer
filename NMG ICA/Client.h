#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <utility>

class Client
{
public:
	static Client* CreateClient(const std::string& username, unsigned short port);

	void Update(float deltaTime, bool windowInFocus);
	
	void Render(sf::RenderWindow& window);

private:
	sf::IpAddress m_server;
	sf::TcpSocket m_socket;
	
	uint8_t m_playerNumber;
	std::vector<sf::RectangleShape> m_players;
	
	float m_speed;
	std::string m_userName;
	float m_packetDelay;
	float m_packetTimer;
	
	bool Initialise(unsigned short port);
	bool ReceiveMessage();
	bool SendMessage();
	Client(std::string username);
};