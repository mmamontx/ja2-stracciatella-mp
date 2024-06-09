#ifndef GAME_INIT_OPTIONS_SCREEN_H
#define GAME_INIT_OPTIONS_SCREEN_H

#include "ScreenIDs.h"
#include "RakPeerInterface.h"

using namespace RakNet;

ScreenID GameInitOptionsScreenHandle(void);

#define ID_USER_PACKET_NAME    ID_USER_PACKET_ENUM
#define ID_USER_PACKET_MESSAGE (ID_USER_PACKET_ENUM + 1)

#define MAX_NAME_LEN    16
#define MAX_MESSAGE_LEN 128

#define MAX_NUM_CLIENTS 2

#define KEY_RETURN 13

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
	char message[MAX_MESSAGE_LEN];
};

struct PLAYER {
	RakNetGUID guid;
	char name[MAX_NAME_LEN];
};

extern std::list<struct PLAYER> gPlayers;
extern NETWORK_OPTIONS gNetworkOptions;
extern BOOL gConnected;
extern BOOL gEnemyEnabled;

#endif
