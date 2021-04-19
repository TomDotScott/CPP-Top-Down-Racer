
#include "Server.h"

int main()
{
	Server* s = Server::CreateServer(25565);

	while(true)
	{
		s->Update(25565);
	}
}
