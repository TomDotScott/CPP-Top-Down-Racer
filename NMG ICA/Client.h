﻿#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

class Client
{
public:
	static Client* CreateClient(unsigned short port);

	void Update(float deltaTime);
	
	void Render(sf::RenderWindow& window);

private:
	sf::IpAddress m_server;
	sf::TcpSocket m_socket;
	sf::RectangleShape m_shape;
	float m_speed;
	
	bool Initialise(unsigned short port);
	bool ReceiveMessage();
	bool SendMessage();
	Client();
};