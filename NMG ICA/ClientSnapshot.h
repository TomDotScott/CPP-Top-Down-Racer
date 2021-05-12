#pragma once
#include <array>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Vector2.hpp>

#include "Globals.h"

/**
 * \brief The ClientSnapshot is a simplified version of the Client, it is used by the server to keep
 * track of the game state
 */
struct ClientSnapshot
{
	ClientSnapshot(const sf::Vector2f& position, const float angle);

	~ClientSnapshot();

	/**
	 * \brief Checks if all of the checkpoints have been passed by the client
	 * i.e. whether they have completed a lap
	 * \return True if they have passed all of the checkpoints in the game
	 */
	bool AllCheckPointsPassed();

	/**
	 * \brief Resets each checkpoint back to false if the client has lapped
	 */
	void ResetCheckPoints();

	/**
	 * \brief Finds the index of the highest checkpoint passed by the client
	 * \return The index of the highest checkpoint passed by the client, -1 if
	 * no checkpoints have been passed
	 */
	int HighestCheckPointPassed();

	/**
	 * \brief Prints the checkpoints visited by the client - useful for debugging!
	 */
	void PrintCheckPoints();

	// The unique identifier of the client
	std::string username;

	// Pointer to the socket used for communication to the client
	sf::TcpSocket* socket;

	// The current position of the client
	sf::Vector2f position;

	// The current angle of the client
	float angle;

	// Each of the checkpoints in the game and whether they have been passed by the
	// client or not
	std::array<bool, globals::game::k_numCheckPoints> checkPointsPassed;

	// The next checkpoint for the AI client to travel to 
	int nextAICheckpoint;

	// The amount of laps completed by the client
	int lapsCompleted;

	// Whether the client has finished the race
	bool raceCompleted;
};
