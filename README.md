# Multiplayer Racing Game
## Game Feature Breakdown
When the player first loads up client.exe, they are greeted with dialogue asking for them to select a username. When a username is chosen, the client attempts to communicate with the server. If after 10 seconds the server is not found or does not send a packet back, the client asks for another username and retries communication. 

If the client was able to connect to the server, the server checks to see if the username is taken. The username is used as a unique identifier for each of the connected client so to maintain the uniqueness. The Factory design pattern was used so that only a proper – in the sense that all the files were loaded and the username is unique – client can ever be constructed. If initialization of the client fails, a nullptr is returned by Client::CreateClient().
 
If the username is available, the server sends a response confirming the username, putting them into the lobby. 
 
There are 4 players required to play the game, and the clients wait until they receive the StartGame message from the server until they start the game. When this happens, the game plays.
 
The game is a standard race around the track. There are checkpoints placed periodically around the track that each client is checked against by the server. If the server notices that the client has passed all the checkpoints, it tells the client that it has completed a lap. If all three laps are completed, the car gets taken over by an AI until all the clients have finished the race. When all the clients finish, they are taken to an end lobby, showing their final placement in the race. 
 
Clients can connect and disconnect at any time and the server deals with it appropriately, sending messages to each client that a specific client connected or disconnected. 
## Known Bugs and Potential Fixes
The enemy AI jiggle about when driving. I think this may be due to them recalculating their rotation every time they move so they constantly move side to side. 

Collision is a bit dodgy too, proper Separating-Axis Theorem implementation rather than the hacky way that I have done should fix it.

Sometimes the server registers the finish line as being crossed twice, this means that occasionally someone will lap twice at once. Better verification of position and the order of packets might sort it.  
## Additions for the future
Now, there is only support for one protocol in the game: TCP. I would like to integrate UDP into the system, probably for server discovery. On top of this, the gameplay is basic, just being 3 laps and then finished. I think it would be fun to have power-ups, booster sections and more, making a top-down MarioKart clone.

On top of this, my Server isn’t threaded. Including separate threads would improve the scalability and speed of my server in the game. I would like to add multi-threading. 
