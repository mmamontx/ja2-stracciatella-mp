#ifndef __GAME_INIT_H
#define __GAME_INIT_H
#include "Types.h"

#include "Soldier_Control.h"

using namespace RakNet;

void InitNewGame();
BOOLEAN AnyMercsHired(void);

void InitStrategicLayer(void);
void ShutdownStrategicLayer(void);

void ReStartingGame(void);

void InitBloodCatSectors(void);

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

extern ReplicaManager3Sample gReplicaManager;
extern NetworkIDManager gNetworkIdManager;

#endif
