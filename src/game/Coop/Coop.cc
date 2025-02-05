#include "Animation_Control.h"
#include "Faces.h"
#include "Game_Clock.h"
#include "Interface_Items.h"
#include "Interface_Panels.h"
#include "Items.h"
#include "Map_Screen_Interface.h"
#include "MapScreen.h"
#include "Merc_Hiring.h"
#include "Message.h"
#include "Overhead.h"
#include "RakNet/MessageIdentifiers.h"
#define GROUP GROUP_JA2
#include "RakNet/RPC4Plugin.h"
#undef GROUP
#include "Random.h"
#include "Soldier_Profile.h"
#include "Strategic.h"
#include "Weapons.h"

#include <Game_Event_Hook.h>

BOOL gConnected = FALSE;
BOOL gEnemyEnabled = TRUE;
BOOL gNetworkCreated = FALSE;
BOOL gReady = FALSE;
BOOL gRPC_Exec = TRUE; // Enables remote events so that they don't overlap with local events
BOOL gStarted = FALSE;
DataStructures::List<Replica3*> gReplicaList;
NETWORK_OPTIONS gNetworkOptions;
NetworkIDManager gNetworkIdManager;
OBJECTTYPE* gpItemPointerRPC; // FIXME: Item dragged by the cursor - this single shared variable would cause conflicts for multiple clients
ReplicaManager3Sample gReplicaManager;
RPC4 gRPC;
std::list<RPC_DATA> gRPC_Events;
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
				player.ready = FALSE;
				gPlayers.push_back(player);

				break;
			}
			case ID_USER_PACKET_MESSAGE:
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_USER_PACKET_MESSAGE");
				struct USER_PACKET_MESSAGE* up;
				up = (struct USER_PACKET_MESSAGE*)p->data;

				for (std::list<struct PLAYER>::iterator it = gPlayers.begin(); it != gPlayers.end(); it++)
					if (it->guid == p->guid) {
						ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, up->service ? ((ST::string)up->message) : ((ST::string)it->name + "> " + (ST::string)up->message));
						break;
					}

				break;
			}
			case ID_USER_PACKET_READY:
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_USER_PACKET_READY");
				struct USER_PACKET_READY* up;
				struct USER_PACKET_MESSAGE up_broadcast;
				char str[256];
				int total_ready = gReady ? 1 : 0; // 1 more if the server is ready
				up = (struct USER_PACKET_READY*)p->data;

				for (std::list<struct PLAYER>::iterator it = gPlayers.begin(); it != gPlayers.end(); it++) {
					if (it->guid == p->guid) {
						it->ready = up->ready;

						// Broadcasting the name of the person that is ready
						sprintf(str, "%s is %s.", it->name, it->ready ? "ready" : "not ready");
						up_broadcast.id = ID_USER_PACKET_MESSAGE;
						up_broadcast.service = TRUE;
						strcpy(up_broadcast.message, str);
						gNetworkOptions.peer->Send((char*)&up_broadcast, sizeof(up_broadcast), MEDIUM_PRIORITY, RELIABLE, 0, UNASSIGNED_RAKNET_GUID, true);

						ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, str); // Duplicating to the server chat
					}

					if (it->ready)
						total_ready++;
				}

				// Broadcasting the cumulative ready status
				sprintf(str, "Ready: %d/%d", total_ready, (int)(gPlayers.size()) + 1); // 1 more for the server
				up_broadcast.id = ID_USER_PACKET_MESSAGE;
				up_broadcast.service = TRUE;
				strcpy(up_broadcast.message, str);
				gNetworkOptions.peer->Send((char*)&up_broadcast, sizeof(up_broadcast), MEDIUM_PRIORITY, RELIABLE, 0, UNASSIGNED_RAKNET_GUID, true);

				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, str); // Duplicating to the server chat

				break;
			}
			case ID_REPLICA_MANAGER_SCOPE_CHANGE:
				// Changed scope of an object
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_REPLICA_MANAGER_SCOPE_CHANGE");
				break;
			case ID_RPC_PLUGIN:
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_RPC_PLUGIN");
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

				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, up->service ? ((ST::string)up->message) : ("?> " + (ST::string)up->message)); // FIXME: As there is no gPlayers list the name is unknown and replaced with '?'

				break;
			}
			case ID_USER_PACKET_START:
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"ID_USER_PACKET_START");

				gStarted = TRUE;

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

void BeginSoldierClimbUpRoofRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	BeginSoldierClimbUpRoof(ID2Soldier(data.id));
}

void BeginSoldierClimbDownRoofRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	BeginSoldierClimbDownRoof(ID2Soldier(data.id));
}

void BeginSoldierClimbFenceRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	BeginSoldierClimbFence(ID2Soldier(data.id));
}

void ChangeWeaponModeRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	ChangeWeaponMode(ID2Soldier(data.id));
}

void UIHandleItemPlacementRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	BOOLEAN fDeductPoints = FALSE; // TODO: Find out if the item is being passed to another person

	if (data.ubKeyDown)
	{
		CleanUpStack(&(ID2Soldier(data.id)->inv[data.ubHandPos]), gpItemPointerRPC);
		return;
	}

	// Try to place here
	if (PlaceObject(ID2Soldier(data.id), data.ubHandPos, gpItemPointerRPC))
	{
		// TODO
		/*if (fDeductPoints)
		{
			// Deduct points
			if (gpItemPointerSoldier->bLife >= CONSCIOUSNESS)
			{
				DeductPoints(gpItemPointerSoldier, 2, 0);
			}
			if (gpSMCurrentMerc->bLife >= CONSCIOUSNESS)
			{
				DeductPoints(gpSMCurrentMerc, 2, 0);
			}
		}*/

		HandleTacticalEffectsOfEquipmentChange(ID2Soldier(data.id), data.ubHandPos, ID2Soldier(data.id)->inv[data.ubHandPos].usItem, gpItemPointerRPC->usItem);

		// Dirty
		fInterfacePanelDirty = DIRTYLEVEL2;

		// TODO: Enable the check below (based on fDeductPoints?), propage the message to the client
		/*if (gpItemPointerSoldier != gpSMCurrentMerc)
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(pMessageStrings[MSG_ITEM_PASSED_TO_MERC], GCM->getItem(usNewItemIndex)->getShortName(), gpSMCurrentMerc->name));
		}*/

		// TODO: Check if the call below is needed and, if so, enable it
		/*if (gpItemPointer != NULL)
		{
			ReevaluateItemHatches(gpSMCurrentMerc, FALSE);
		}*/
	}
}

void UIHandleSoldierStanceChangeRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	UIHandleSoldierStanceChange(ID2Soldier(data.id), data.bNewStance);
}

void HandleItemPointerClickRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	gRPC_Events.push_front(data);

	HandleItemPointerClick(data.usMapPos);
}

void BeginItemPointerRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	SOLDIERTYPE* const pSoldier = ID2Soldier(data.id);

	gpItemPointerRPC = new OBJECTTYPE{};

	if (data.ubKeyDown) {
		RemoveObjectFromSlot(pSoldier, data.ubHandPos, gpItemPointerRPC);
	} else {
		GetObjFrom(&(pSoldier->inv[data.ubHandPos]), 0, gpItemPointerRPC);
	}

	fInterfacePanelDirty = DIRTYLEVEL2; // Interface has to be redrawn for the server to show the released slot

	// The call below is not a part of BeginItemPointer(), but comes right after it
	HandleTacticalEffectsOfEquipmentChange(pSoldier, data.ubHandPos, pSoldier->inv[data.ubHandPos].usItem, NOTHING);
}

void HandleEventRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet)
{
	RPC_DATA data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	gRPC_Events.push_back(data);
}
