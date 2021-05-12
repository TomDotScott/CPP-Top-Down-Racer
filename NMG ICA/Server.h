#pragma once
#include <SFML/Network.hpp>


#include "ClientSnapshot.h"
#include "../Shared Files/Data.h"

/**
 * \brief The Server of the racing game
 */
class Server
{
public:
	/**
	 * \brief Returns a heap allocated Server object if initialisation was successful
	 * \param port The port to host the server on via TCP
	 * \return A unique_ptr if initialisation was successful, nullptr if not
	 */
	static std::unique_ptr<Server> CreateServer(unsigned short port);

	/**
	 * \brief Updates the AI drivers in the game
	 * \param deltaTime The time difference between update loops,
	 * for frame-rate independence
	 */
	void Update(const float deltaTime);
	
	// Non-copyable and non-moveable
	Server(const Server& other) = delete;
	Server& operator=(const Server& other) = delete;

	Server(Server&& other) = delete;
	Server& operator=(Server&& other) = delete;

	~Server() = default;

private:
	// The listener for the TCP sockets, to establish and maintain a connection
	sf::TcpListener m_listener;

	// The socket selector object allows for multiple clients to connect
	// to the server
	sf::SocketSelector m_socketSelector;

	// A vector of all of the connected clients in the game
	std::vector<std::unique_ptr<ClientSnapshot>> m_connectedClients;

	// A flag for whether the race has started or not
	bool m_gameInProgress;

	Server();

	/**
	 * \brief Attempts to initialise a Server object
	 * \param port The port to bind the TCP Listener to 
	 * \return True if the Server was successfully initialised
	 */
	bool Initialise(unsigned short port);

	/**
	 * \brief Scans the socket selector in case a new client wants
	 * to connect to the game. Handles rejection if the server is full
	 * or if a race is already in progress
	 */
	void CheckForNewClients();
	
	/**
	 * \brief Checks to see if all of the racers have completed all the laps
	 * \return True if the race has finished
	 */
	bool CheckGameOver();
	
	/**
	 * \brief Handles collision between clients. Updates their position across all
	 * clients when the collision is resolved
	 */
	void CheckCollisionsBetweenClients();

	/**
	 * \brief Calculates the current order of players in the race. I.e. who is First all the
	 * way to who is last
	 */
	void WorkOutTrackPlacements();
	
	/**
	 * \brief Updates the AI controlled client
	 * \param deltaTime The time between updates, for frame-rate independence
	 * \param client The client that should be moved by the AI 
	 */
	void AIMovement(float deltaTime, ClientSnapshot& client) const;
	
	/**
	 * \brief Finds the index of the client in the connectedClients vector
	 * via their username
	 * \param username The username to look for
	 * \return The index of the username to look for, -1 if the username can't
	 * be found
	 */
	[[nodiscard]] int FindClientIndex(const std::string& username) const;
	
	/**
	 * \brief Finds out if the username is taken in the connectedClients vector
	 * \param username The username to look for
	 * \return True if the username is taken
	 */
	[[nodiscard]] bool IsUsernameTaken(const std::string& username) const;
	
	/**
	 * \brief Handles collision between the clients and the checkpoints around the map
	 * \param client The client to check collisions of
	 */
	void CheckIfClientHasPassedCheckPoint(ClientSnapshot& client);
	
	/**
	 * \brief Sends a packet to a connected client via TCP
	 * \param dataToSend The TcpDataPacket to send to the client
	 * \param receiver The username of the recipient of the message
	 * \return True if the message was sent correctly
	 */
	[[nodiscard]] bool SendMessage(const TcpDataPacket& dataToSend, const std::string& receiver);
	
	/**
	 * \brief Broadcasts a packet to every connected client
	 * \param dataToSend The TcpDataPacket to be sent to every connected client
	 * \return True if the broadcast was sent correctly
	 */
	[[nodiscard]] bool BroadcastMessage(const TcpDataPacket& dataToSend) const;
};
