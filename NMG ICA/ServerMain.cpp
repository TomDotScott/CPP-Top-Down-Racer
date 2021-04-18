
#include "Server.h"

int main()
{
	Server s;
	s.Initialise(25565);

	while(true)
	{
		s.Update(25565);
	}
}
