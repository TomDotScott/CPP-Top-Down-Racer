#pragma once
#include <unordered_map>
#include <SFML/Network.hpp>
#include <SFML/Graphics/Color.hpp>
#include <array>
#include <SFML/Graphics/Rect.hpp>


#include "Globals.h"
#include "../Shared Files/Data.h"

struct Client
{
	Client(const sf::Vector2f& position, const float angle);
	
	~Client();
	
	bool AllCheckPointsPassed();
	
	std::string m_username;
	// TODO: Swap to unique_ptr - ups are 'Owning Handles'
	sf::TcpSocket* m_socket;
	sf::Vector2f m_position;
	float m_angle;
	// Switched to an array of size 6 - map isn't cache friendly and it's heap allocated!
	std::array<bool, globals::k_numCheckPoints> m_checkPointsPassed;
};

// TODO: Use UDP for connecting, TCP for everything else
class Server
{
public:
	static std::unique_ptr<Server> CreateServer(unsigned short port);

	void Update(unsigned short port);

	// Non-copyable and non-moveable 
	Server(const Server& other) = delete;
	Server& operator=(const Server& other) = delete;

	Server(Server&& other) = delete;
	Server& operator=(Server&& other) = delete;

private:
	sf::TcpListener m_listener;
	sf::SocketSelector m_socketSelector;

	// Moved from Unordered_Map to vector... The intent is made clearer as I am iterating over it
	std::vector<std::unique_ptr<Client>> m_connectedClients;

	std::array<sf::FloatRect, globals::k_numCheckPoints> m_levelCheckpoints;

	bool m_gameInProgress;

	Server();
	bool Initialise(unsigned short port);
	void CheckForNewClients();
	void CheckCollisionsBetweenClients();
	int FindClientIndex(const std::string& username) const;
	bool IsUsernameTaken(const std::string& username) const;
	void CheckIfClientHasPassedCheckPoint(Client& client);
	bool SendMessage(const DataPacket& dataToSend, const std::string& receiver);
	bool BroadcastMessage(const DataPacket& dataToSend, sf::TcpSocket& sender) const;
	bool BroadcastMessage(const DataPacket& dataToSend) const;
	bool ReceiveMessage();
};
