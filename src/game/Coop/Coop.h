#ifndef COOP_H
#define COOP_H

#include "Handle_UI.h"
#define GROUP GROUP_JA2
#include "NetworkIDManager.h"
#include "RakNet/RPC4Plugin.h"
#include "RakPeerInterface.h"
#include "ReplicaManager3.h"
#undef GROUP
#include "MessageIdentifiers.h"
#include "MouseSystem.h"
#include "Soldier_Control.h"

using namespace RakNet;

#define ID_USER_PACKET_NAME    ID_USER_PACKET_ENUM
#define ID_USER_PACKET_MESSAGE (ID_USER_PACKET_ENUM + 1)
#define ID_USER_PACKET_READY   (ID_USER_PACKET_ENUM + 2)
#define ID_USER_PACKET_START   (ID_USER_PACKET_ENUM + 3)

#define MAX_NAME_LEN    16
#define MAX_MESSAGE_LEN 128

#define MAX_NUM_CLIENTS 2

#define KEY_RETURN 13

#define IS_CLIENT       (gGameOptions.fNetwork)
#define IS_VALID_CLIENT ((gGameOptions.fNetwork) && (gConnected) && (gReplicaList.Size() != 0))

#define RPC_READY ((gRPC_Events.empty() == FALSE) && gRPC_Exec)

struct NETWORK_OPTIONS {
	ST::string name;
	ST::string ip;
	UINT16 port;
	RakPeerInterface *peer;
};

struct USER_PACKET_NAME {
	unsigned char id;
	char name[MAX_NAME_LEN];
};

struct USER_PACKET_MESSAGE {
	unsigned char id;
	BOOL service; // Service message - don't show the name before the message in the chat
	char message[MAX_MESSAGE_LEN];
};

struct USER_PACKET_READY {
	unsigned char id;
	BOOL ready;
};

struct USER_PACKET_START {
	unsigned char id;
};

struct PLAYER {
	RakNetGUID guid;
	char name[MAX_NAME_LEN];
	BOOL ready;
};

struct RPC_DATA {
	BOOLEAN inv; // There are two types of RPC events: 0 - regular, which are placed to the back of the list and executed one per frame via gRPC_Exec;
	             //                                    1 - inventory, which are placed to the front of the list and executed right away.
	UIEventKind puiNewEvent;
	SoldierID id;
	SoldierID tgt_id;
	GridNo usMapPos;
	BOOLEAN fUIMovementFast;
	INT8 bNewStance;
	UINT8 ubHandPos;
	UINT8 ubKeyDown;
};

class SampleConnection : public Connection_RM3
{
public:
	SampleConnection(const SystemAddress& _systemAddress, RakNetGUID _guid) : Connection_RM3(_systemAddress, _guid) {}
	virtual ~SampleConnection() {}

	bool QueryGroupDownloadMessages(void) const { return true; }

	virtual Replica3* AllocReplica(BitStream* allocationId, ReplicaManager3* replicaManager3)
	{
		RakString typeName;
		allocationId->Read(typeName);
		if (typeName == "SOLDIERTYPE") return new SOLDIERTYPE;
		return 0;
	}
protected:
};

class ReplicaManager3Sample : public ReplicaManager3
{
	virtual Connection_RM3* AllocConnection(const SystemAddress& systemAddress, RakNetGUID rakNetGUID) const
	{
		return new SampleConnection(systemAddress, rakNetGUID);
	}
	virtual void DeallocConnection(Connection_RM3* connection) const
	{
		delete connection;
	}
};

extern BOOL gConnected;
extern BOOL gEnemyEnabled;
extern BOOL gNetworkCreated;
extern BOOL gReady;
extern BOOL gRPC_Exec;
extern BOOL gStarted;
extern DataStructures::List<Replica3*> gReplicaList;
extern NETWORK_OPTIONS gNetworkOptions;
extern NetworkIDManager gNetworkIdManager;
extern OBJECTTYPE* gpItemPointerRPC;
extern ReplicaManager3Sample gReplicaManager;
extern RPC4 gRPC;
extern std::list<RPC_DATA> gRPC_Events;
extern std::list<struct PLAYER> gPlayers;

extern DWORD WINAPI client_packet(LPVOID lpParam);
extern DWORD WINAPI replicamgr(LPVOID lpParam);
extern DWORD WINAPI server_packet(LPVOID lpParam);
extern unsigned char SGetPacketIdentifier(Packet* p);
extern void BeginItemPointerRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void BeginSoldierClimbDownRoofRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void BeginSoldierClimbFenceRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void BeginSoldierClimbUpRoofRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void ChangeWeaponModeRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void HandleEventRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void HandleItemPointerClickRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void HireRandomMercs(unsigned int n);
extern void BtnStealthModeCallbackRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void UIHandleItemPlacementRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);
extern void UIHandleSoldierStanceChangeRPC(RakNet::BitStream* bitStream, RakNet::Packet* packet);

#endif
