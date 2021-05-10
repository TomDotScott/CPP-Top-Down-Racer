#include "Server.h"

int main()
{
	auto s = Server::CreateServer(25565);

	sf::Clock clock;
	
	while(true)
	{
		sf::Time time = clock.restart();
		const float deltaTime = time.asSeconds();
		
		s->Update(deltaTime);
	}
}
