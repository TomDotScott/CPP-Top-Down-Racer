#pragma once
#include <unordered_map>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

#include "Map.h"
#include "Player.h"
#include "../Shared Files/Data.h"


/**
 * \brief The Client class acts as the controller for the game, it holds the
 * player, map and communicates to the server
 */
class Client
{
public:
	/**
	 * \brief Returns a heap allocated Client object if initialisation was successful
	 * \param username The chosen username of the client
	 * \param port The port to connect to the server on via TCP
	 * \return A unique_ptr if initialisation was successful, nullptr if not
	 */
	static std::unique_ptr<Client> CreateClient(const std::string& username, unsigned short port);

	/**
	 * \brief Handles input for the game
	 * \param deltaTime The time difference between
	 * frames, for frame-rate independence
	 */
	void Input(float deltaTime);
	
	/**
	 * \brief Updates the objects in the game and
	 * communicates with the server
	 * \param deltaTime The time difference between
	 * frames, for frame-rate independence
	 */
	void Update(float deltaTime);
	
	/**
	 * \brief Renders the game to the screen
	 * \param window The RenderWindow to render to
	 */
	void Render(sf::RenderWindow& window);
	
	/**
	 * \brief Sets the font to be used in the game
	 * \param font The chosen font
	 */
	void SetGameFont(const sf::Font& font);

private:
	// The socket handles TCP communication between the client and the server
	sf::TcpSocket m_socket;

	// The username is a unique identifier for the client. The client will not
	// initialise if the username is taken
	std::string m_userName;

	// The time delay between packets sent
	float m_packetDelay;

	// The countdown between packets sent
	float m_packetTimer;

	// Flag for whether the game has started and the track and cars should be drawn
	// and updated
	bool m_gameStarted;

	// Flag for whether the race has been completed by the client
	bool m_completedRace;

	// Flag for whether the race has been completed by every client that is connected
	// to the server
	bool m_gameOver;

	// The amount of laps completed by the player
	int m_lapsCompleted;

	// The player's position in the race
	int m_positionInRace;

	// The container that holds all of the connected players in the game. As the username of
	// each client is their unique identifier, it was chosen to be the key of the unordered map
	std::unordered_map<std::string, Player> m_players;

	// This vector is accessed at the end of the race. It stores the order that the connected
	// clients placed at the end of the race and is used when drawing the end screen
	std::vector<std::string> m_finalPlayerOrder;

	// The texture used for drawing the player
	sf::Texture m_carTexture;

	// The track of the game
	Map m_background;

	// Text used to draw the UI in the game
	sf::Text m_text;

	/**
	 * \brief Initialises a client object if it is able to load the appropriate files
	 * and bind and connect to the server's IP and Port
	 * \param port The port number to connect to the server on
	 * \return True if the Client has been initialised correctly
	 */
	bool Initialise(unsigned short port);

	/**
	 * \brief Adds a player to the m_players map
	 * \param username The username of the player to add
	 * \return True if the player was added successfully
	 */
	bool AddPlayer(const std::string& username);
	
	/**
	 * \brief Removes a player from the m_players map
	 * \param username The username of the player to remove
	 * \return True if the player was removed successfully
	 */
	bool RemovePlayer(const std::string& username);
	
	/**
	 * \brief Receives a message from the server
	 * \return True if the message was received successfully
	 */
	bool ReceiveMessage();
	/**
	 * \brief Sends a generic TcpDataPacket to the server to communicate
	 * game-play to the server
	 * \param type The type of TcpDataPacket to send
	 * \return True if the message was sent successfully
	 */
	bool SendMessage(eDataPacketType type);
	
	/**
	 * \brief Sends a TcpDataPacket object to the server to communicate
	 * game-play
	 * \param dp The TcpDataPacket to send
	 * \return True if the message was sent successfully
	 */
	bool SendMessage(TcpDataPacket& dp);

	/**
	 * \brief Constructs a Client object
	 * \param username The chosen username of the client
	 */
	explicit Client(std::string username);
};
