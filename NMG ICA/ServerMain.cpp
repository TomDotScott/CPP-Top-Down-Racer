#include "Server.h"

int main()
{
	auto s = Server::CreateServer(25565);

	while(true)
	{
		s->Update(25565);
	}
}
