#include "ClientSnapshot.h"

#include <iostream>

ClientSnapshot::ClientSnapshot(const sf::Vector2f& position, const float angle) :
	socket(new sf::TcpSocket()),
	position(position),
	angle(angle),
	checkPointsPassed(),
	nextAICheckpoint(0),
	lapsCompleted(0),
	raceCompleted(false)
{
	// Start having completed no checkpoints
	for (int i = 0; i < globals::game::k_numCheckPoints; ++i)
	{
		checkPointsPassed[i] = false;
	}
}

ClientSnapshot::~ClientSnapshot()
{
	delete socket;
}

bool ClientSnapshot::AllCheckPointsPassed()
{
	int passedCheckPoints = 0;
	for (const auto& cp : checkPointsPassed)
	{
		if (cp)
		{
			passedCheckPoints++;
		}
	}
	return passedCheckPoints == globals::game::k_numCheckPoints;
}

void ClientSnapshot::ResetCheckPoints()
{
	std::cout << "Completed a lap, resetting the checkpoints" << std::endl;
	for (auto& point : checkPointsPassed)
	{
		point = false;
	}
}

int ClientSnapshot::HighestCheckPointPassed()
{
	for (int i = globals::game::k_numCheckPoints - 1; i >= 0; --i)
	{
		if (checkPointsPassed[i])
		{
			return i;
		}
	}
	return -1;
}

void ClientSnapshot::PrintCheckPoints()
{
	for (int i = 0; i < static_cast<int>(checkPointsPassed.size()); ++i)
	{
		std::cout << "Checkpoint: " << i << (checkPointsPassed[i] ? " passed" : " not passed") << std::endl;
	}
}