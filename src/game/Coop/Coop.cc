#include "Animation_Control.h"
#include "Faces.h"
#include "Game_Clock.h"
#include "Map_Screen_Interface.h"
#include "MapScreen.h"
#include "Merc_Hiring.h"
#include "Message.h"
#include "Overhead.h"
#include "RakNet/MessageIdentifiers.h"
#include "Random.h"
#include "Soldier_Profile.h"
#include "Strategic.h"

#include <Game_Event_Hook.h>

BOOL gConnected = FALSE;
BOOL gEnemyEnabled = TRUE;
BOOL gNetworkCreated = FALSE;
DataStructures::List<Replica3*> gReplicaList;
NETWORK_OPTIONS gNetworkOptions;
NetworkIDManager gNetworkIdManager;
ReplicaManager3Sample gReplicaManager;
std::list<struct PLAYER> gPlayers;

DWORD WINAPI replicamgr(LPVOID lpParam)
{
	while (1)
		Sleep(1000); // NOTE: It doesn't affect object replication sync delay, but it may have some other effect - keep this in mind
	return 0;
}

// For debugging purposes (so we don't have to hire mercs manually everytime)
void HireRandomMercs(unsigned int n)
{
	struct MERC_HIRE_STRUCT h;
	int id_random;
	srand((unsigned)time(NULL));

	for (int i = 0; i < n; i++) {
		// Prevent repetitive mercs
		do {
			id_random = rand() % 40; // There should be at least 40 mercs (considering AIM only)
		} while (gMercProfiles[id_random].bMercStatus < 0);

		h = MERC_HIRE_STRUCT{};

		h.ubProfileID = id_random;
		h.sSector = g_merc_arrive_sector;
		h.fUseLandingZoneForArrival = TRUE;
		h.ubInsertionCode = INSERTION_CODE_ARRIVING_GAME;
		h.iTotalContractLength = 1;
		h.fCopyProfileItemsOver = true;

		gMercProfiles[id_random].ubMiscFlags |= PROFILE_MISC_FLAG_ALREADY_USED_ITEMS;

		HireMerc(h);
	}

	fDrawCharacterList = true;
	fTeamPanelDirty = true;
	ReBuildCharactersList();
}

unsigned char SGetPacketIdentifier(Packet* p)
{
	if (p == 0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char)p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	} else {
		return (unsigned char)p->data[0];
	}
}

DWORD WINAPI server_packet(LPVOID lpParam)
{
	RakNet::Packet* p;
	unsigned char SpacketIdentifier;

	while (TRUE) {
		p = gNetworkOptions.peer->Receive();

		while (p) {
			// We got a packet, get the identifier with our handy function
			SpacketIdentifier = SGetPacketIdentifier(p);

			// Check if this is a network message packet
			switch (SpacketIdentifier) {
			case ID_DISCONNECTION_NOTIFICATION:
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_DISCONNECTION_NOTIFICATION");
				break;
			case ID_ALREADY_CONNECTED:
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_ALREADY_CONNECTED");
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REMOTE_DISCONNECTION_NOTIFICATION");
				break;
			case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REMOTE_CONNECTION_LOST");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REMOT/MING_CONNECTION");
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_CONNECTION_ATTEMPT_FAILED");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				// Sorry, the server is full
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_NO_FREE_INCOMING_CONNECTIONS");
				break;
			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_CONNECTION_LOST");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				// This tells the client that it has connected
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_CONNECTION_REQUEST_ACCEPTED");
				break;
			case ID_NEW_INCOMING_CONNECTION:
				// This tells the server that a client has connected
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_NEW_INCOMING_CONNECTION");
				break;
			case ID_USER_PACKET_NAME: // This message and below are custom messages of JA2S Coop
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_USER_PACKET_NAME");
				struct USER_PACKET_NAME* up;
				up = (struct USER_PACKET_NAME*)p->data;

				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, (ST::string)up->name + " has connected.");

				// Registering the new player in the global struct
				struct PLAYER player;
				player.guid = p->guid;
				strcpy(player.name, up->name);
				gPlayers.push_back(player);

				break;
			}
			case ID_USER_PACKET_MESSAGE:
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_USER_PACKET_MESSAGE");
				struct USER_PACKET_MESSAGE* up;
				up = (struct USER_PACKET_MESSAGE*)p->data;

				for (std::list<struct PLAYER>::iterator it = gPlayers.begin(); it != gPlayers.end(); it++)
					if (it->guid == p->guid)
						ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, (ST::string)it->name + "> " + (ST::string)up->message);

				break;
			}
			case ID_REPLICA_MANAGER_SCOPE_CHANGE:
				// Changed scope of an object
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REPLICA_MANAGER_SCOPE_CHANGE");
				break;
			default:
				char unknown_id[3];
				itoa(SpacketIdentifier, unknown_id, 10);
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "SpacketIdentifier = " + (ST::string)unknown_id);
				break;
			}

			// We're done with the packet, get more :)
			gNetworkOptions.peer->DeallocatePacket(p);
			p = gNetworkOptions.peer->Receive();
		}

		Sleep(33); // NOTE: ~30 FPS, can be improved if needed
	}

	return 0;
}

DWORD WINAPI client_packet(LPVOID lpParam)
{
	RakNet::Packet* p;
	unsigned char SpacketIdentifier;

	while (TRUE) {
		p = gNetworkOptions.peer->Receive();

		while (p) {
			// We got a packet, get the identifier with our handy function
			SpacketIdentifier = SGetPacketIdentifier(p);

			// Check if this is a network message packet
			switch (SpacketIdentifier) {
			case ID_DISCONNECTION_NOTIFICATION:
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_DISCONNECTION_NOTIFICATION");
				break;
			case ID_ALREADY_CONNECTED:
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_ALREADY_CONNECTED");
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REMOTE_DISCONNECTION_NOTIFICATION");
				break;
			case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REMOTE_CONNECTION_LOST");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REMOT/MING_CONNECTION");
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_CONNECTION_ATTEMPT_FAILED");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				// Sorry, the server is full
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_NO_FREE_INCOMING_CONNECTIONS");
				break;
			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_CONNECTION_LOST");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				// This tells the client that it has connected
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_CONNECTION_REQUEST_ACCEPTED");
				break;
			case ID_NEW_INCOMING_CONNECTION:
				// This tells the server that a client has connected
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_NEW_INCOMING_CONNECTION");
				break;
			case ID_REPLICA_MANAGER_CONSTRUCTION:
				// Create an object
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REPLICA_MANAGER_CONSTRUCTION");
				break;
			case ID_REPLICA_MANAGER_SCOPE_CHANGE: // Not sure what does this one mean
				// Changed scope of an object
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REPLICA_MANAGER_SCOPE_CHANGE");
				break;
			case ID_REPLICA_MANAGER_SERIALIZE:
				// Serialized data of an object
				//ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REPLICA_MANAGER_SERIALIZE");
				break;
			case ID_REPLICA_MANAGER_DOWNLOAD_STARTED:
				// New connection, about to send all world objects
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REPLICA_MANAGER_DOWNLOAD_STARTED");
				break;
			case ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE:
				// Finished downloading all serialized objects
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE");

				gReplicaManager.GetReferencedReplicaList(gReplicaList);

				FOR_EACH_IN_TEAM(s, OUR_TEAM) { // Below are actions that were supposed to be done when hiring mercs through AIM
					gfAtLeastOneMercWasHired = true; // Ugly, but should handle the case when the server doesn't have any characters yet on client connection
					InitSoldierFace(*s); // 'face' has to be initialized locally since originally it points to the host memory and causes an exception
					AddStrategicEvent(EVENT_DELAYED_HIRING_OF_MERC, (STARTING_TIME + FIRST_ARRIVAL_DELAY) / NUM_SEC_IN_MIN, s->ubID); // Place to the helicopter
					EVENT_InitNewSoldierAnim(s, STANDING, Random(10), TRUE); // Initialize animation locally
					CreateSoldierPalettes(*s); // Initialize palette locally
				}

				// Update merc list in the left panel to show replicated characters
				fDrawCharacterList = true;
				fTeamPanelDirty = true;
				ReBuildCharactersList();

				break;
			case ID_USER_PACKET_MESSAGE:
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_USER_PACKET_MESSAGE");
				struct USER_PACKET_MESSAGE* up;
				up = (struct USER_PACKET_MESSAGE*)p->data;

				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "?> " + (ST::string)up->message); // FIXME: As there is no gPlayers list the name is unknown and replaced with '?'

				break;
			}
			default:
				char unknown_id[3];
				itoa(SpacketIdentifier, unknown_id, 10);
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "SpacketIdentifier = " + (ST::string)unknown_id);
				break;
			}

			// We're done with the packet, get more :)
			gNetworkOptions.peer->DeallocatePacket(p);
			p = gNetworkOptions.peer->Receive();
		}

		Sleep(33); // NOTE: ~30 FPS, can be improved if needed
	}

	return 0;
}
