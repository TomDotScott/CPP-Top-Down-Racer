#pragma once
#include <unordered_map>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <utility>

#include "Player.h"

class Client
{
public:
	static std::unique_ptr<Client> CreateClient(const std::string& username, unsigned short port);

	void Update(float deltaTime, bool windowInFocus);

	void Render(sf::RenderWindow& window);

	void Input(float deltaTime);

private:
	sf::IpAddress m_server;
	sf::TcpSocket m_socket;
	std::string m_userName;
	float m_packetDelay;
	float m_packetTimer;
	bool m_playerMoved;

	std::unordered_map<std::string, Player> m_players;
	sf::Texture m_texture;
	sf::Sprite m_sprite;

	bool Initialise(unsigned short port);
	bool ReceiveMessage();
	bool SendMessage();

	Client(const std::string& username);
};
