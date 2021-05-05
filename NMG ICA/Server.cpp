#include "Server.h"
#include <iostream>
#include "../Shared Files/Data.h"

std::unique_ptr<Server> Server::CreateServer(const unsigned short port)
{
	std::unique_ptr<Server> newServer(new Server());
	if (newServer->Initialise(port))
	{
		return newServer;
	}

	return nullptr;
}

Server::Server() :
	m_maxClients(8)
{

}

Server::~Server()
{
	for (auto& connectedClient : m_connectedClients)
	{
		delete connectedClient.second;
	}
}

bool Server::Initialise(const unsigned short port)
{
	// Listen to the given port for incoming connections
	if (m_listener.listen(port, sf::IpAddress::getLocalAddress()) != sf::Socket::Done)
	{
		return false;
	}

	// Add the listener to the selector
	m_socketSelector.add(m_listener);

	std::cout << "Server is listening to port " << port << ", waiting for connections... " << std::endl;
	return true;
}

void Server::CheckForNewClients()
{
	if (m_socketSelector.isReady(m_listener))
	{
		// Create a new connection
		auto* client = new sf::TcpSocket();
		if (m_listener.accept(*client) == sf::Socket::Done)
		{
			sf::Packet inPacket;
			DataPacket inData;

			if (client->receive(inPacket) == sf::Socket::Done)
			{
				inPacket >> inData;
			}

			if (m_connectedClients.size() < m_maxClients)
			{
				if (inData.m_type == eDataPacketType::e_FirstConnection)
				{
					// Add the new client to the selector - this means we can update all clients
					m_socketSelector.add(*client);
					
					std::cout << inData.m_userName << " has connected to the server" << std::endl;
					m_connectedClients.insert(std::make_pair(inData.m_userName, client));

					// Tell the client that they are successful
					sf::Packet outPacket;

					const DataPacket outData(eDataPacketType::e_UserNameConfirmation, "SERVER");

					outPacket << outData;
					
					client->send(outPacket);

					outPacket.clear();

					// Tell the other clients that a new client has connected
					const DataPacket updateClientDataPacket(eDataPacketType::e_NewClient, "SERVER");
					outPacket << updateClientDataPacket;

					SendMessage(updateClientDataPacket, *client);
				}
			} else
			{
				std::cout << "MAXIMUM AMOUNT OF CLIENTS CONNECTED" << std::endl;
				
				sf::Packet maxClientMessagePkt;
				const DataPacket maximumClientMessage(eDataPacketType::e_MaxPlayers, "SERVER");
				maxClientMessagePkt << maximumClientMessage;
				
				client->send(maxClientMessagePkt);
				
				delete client;
			}
		} else
		{
			std::cout << "A client had an error connecting..." << std::endl;
			delete client;
		}
	}
}

void Server::Update(unsigned short port)
{
	if (m_socketSelector.wait())
	{
		CheckForNewClients();

		// Loop through each client and use our new, fancy, socket selector
		for(auto& client : m_connectedClients)
		{
			if(client.second && m_socketSelector.isReady(*client.second))
			{
				sf::Packet inPacket;
				if(client.second->receive(inPacket) == sf::Socket::Done)
				{
					DataPacket inData;
					inPacket >> inData;

					std::cout << "Received a message from: " << inData.m_userName << std::endl;

					if (inData.m_type == eDataPacketType::e_UpdatePosition)
					{
						SendMessage(inData, *client.second);
					}
				}
			}
		}
	}
}

bool Server::SendMessage(const DataPacket& dataToSend, sf::TcpSocket& sender)
{
	const sf::Vector2f playerPos(dataToSend.m_x, dataToSend.m_y);
	std::cout << dataToSend.m_userName << " moved to position: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;

	//A reference stops this from happening...
	// sender = nullptr;
	sf::Packet sendPacket;

	sendPacket << dataToSend;

	// update the other connected clients
	for (auto& client : m_connectedClients)
	{
		// Make sure not to send the client their own data!
		if (client.first != dataToSend.m_userName)
		{
			if (dataToSend.m_userName != "SERVER")
			{
				client.second->send(sendPacket);
			}
		}
	}
	
	return true;
}


bool Server::ReceiveMessage()
{

	/*sf::Packet p;

	if (m_socketSelector.receive(p) != sf::Socket::Done)
	{
		return false;
	}

	float x, y;

	p >> x >> y;

	const sf::Vector2f playerPos(x, y);

	if (playerPos != m_prevPlayerPosition)
	{
		std::cout << "Data received from the client measuring " << p.getDataSize() << " bytes" << std::endl;

		std::cout << "The shape is at: " << x << ", " << y << std::endl;
	}

	m_prevPlayerPosition = playerPos;*/

	return true;
}
