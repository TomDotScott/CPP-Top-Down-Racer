#include <iostream>
#include "Client.h"
#include "Player.h"

int main()
{
	bool clientCreated = false;
	std::string username;

	std::unique_ptr<Client> client;

	// Keep looping until a complete client has been created
	do
	{
		username = "";
		std::cout << "Enter a username: ";

		std::cin >> username;

		client = Client::CreateClient(username, 25565);

		if (client) clientCreated = true;
	} while (!clientCreated);


	sf::Font gameFont;

	gameFont.loadFromFile("images/gamefont.ttf");

	client->SetGameFont(gameFont);

	sf::RenderWindow window(sf::VideoMode(globals::game::k_screenWidth, globals::game::k_screenHeight), "Racing Game: " + username);

	sf::Clock clock;

	// run the program as long as the window is open
	while (window.isOpen())
	{
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event e{};

		while (window.pollEvent(e))
		{
			// "close requested" event: we close the window
			if (e.type == sf::Event::Closed)
				window.close();
		}

		window.clear();

		sf::Time time = clock.restart();
		const float deltaTime = time.asSeconds();

		// Only take input if the window has been clicked on - very useful for testing!
		if (window.hasFocus())
		{
			client->Input(deltaTime);
		}

		client->Update(deltaTime);
		client->Render(window);

		window.display();
	}
}
