#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#include <Windows.h>
#include <cstdint>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>



namespace GameAddresses {
	uintptr_t localPlayer = (uintptr_t)0x18AC00;

	uintptr_t entityListBase = (uintptr_t)0x18AC04;
	uintptr_t entityListSize = (uintptr_t)0x18AC0C;
}

struct Entity
{
	DWORD vTable; //0x0000
	float X; //0x0004
	float Y; //0x000C
	float Z; //0x0008
	char pad_0010[32]; //0x0010
	float footY; //0x0030
	float yaw; //0x0034
	float pitch; //0x0038
	char pad_003C[176]; //0x003C
	int8_t health; //0x00EC
	char pad_00ED[27]; //0x00ED
	int8_t secondaryAmmoReserve; //0x0108
	char pad_0109[7]; //0x0109
	int8_t primaryAmmoReserve; //0x0110
	char pad_0111[27]; //0x0111
	int8_t secondaryAmmo; //0x012C
	char pad_012D[7]; //0x012D
	int8_t primaryAmmo; //0x0134
	char pad_0135[208]; //0x0135
	char name[32]; //0x0205

}; //Size: 0x1063


bool isEntity(Entity* e) {
	if (e) {
		if (e->vTable == 5558396) {
			return true;
		}
	}
	return false;
}

std::vector<Entity*> GenerateEntityList(uintptr_t client) {
	uintptr_t* entityListBase = (uintptr_t*)(*(uintptr_t*)(client + GameAddresses::entityListBase));
	std::vector<Entity*> entityVector;

	for (int i = 1; i < *(int32_t*)(client + GameAddresses::entityListSize); i++)
	{
		Entity* currentPlayerBase = (Entity*)*(entityListBase + i);
		entityVector.push_back(currentPlayerBase);
		//std::cout << (char*)currentPlayerBase->name << "'s Health: " << (int32_t)currentPlayerBase->health << std::endl;
	}
	return entityVector;
}

Entity* GetClosestEntity(Entity* p, std::vector<Entity*> ev) {
	float smallestDistance = 10000;
	Entity* smallestDistanceEntity = ev[0];
	for (int i = 0; i < ev.size(); i++)
	{
		Entity* e = ev[i];
		if ((int32_t)e->health > 0) {
			float dx = p->X - e->X;
			float dy = p->Y - e->Y;
			float dz = p->Z - e->Z;
			float dist = (float)sqrt(dx * dx + dy * dy + dz * dz);
			if (dist < smallestDistance) {
				smallestDistance = dist;
				smallestDistanceEntity = e;
			}
		}
	}
	return smallestDistanceEntity;
}

void OutputEntityList(std::vector<Entity*> ev) {
	for (int i = 0; i < ev.size(); i++)
	{
		Entity* currentPlayerBase = ev[i];
		std::cout << (char*)currentPlayerBase->name << "'s Health: " << (int32_t)currentPlayerBase->health << std::endl;
	}
}


void AimAt(Entity* player, Entity* target) {

	//calculate horizontal angle between enemy and player (yaw)
	float dx = target->X - player->X;
	float dy = target->Y - player->Y;
	double angleYaw = atan2(dy, dx) * 180 / M_PI;

	//calculate verticle angle between enemy and player (pitch)
	float dz = target->Z - player->Z;

	double distance = sqrt(dx * dx + dy * dy);
	double anglePitch = atan2(dz, distance) * 180 / M_PI;

	//set self angles to calculated angles
	player->yaw = (float)angleYaw + 90;
	player->pitch = (float)anglePitch;
}


void hackMain(const HMODULE instance) noexcept {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	uintptr_t client = (uintptr_t)(GetModuleHandle("ac_client.exe"));

	std::vector<Entity*> entities = GenerateEntityList(client);
	OutputEntityList(entities);

	Entity* player;
	player = *reinterpret_cast<Entity**>(client + GameAddresses::localPlayer);
	std::cout << (char*)player->name << "'s Health: " << (int32_t)player->health << std::endl;

	
	// Player List
	// ----------------------------

	while (!GetAsyncKeyState(VK_END)) { // END KEY TO UNINJECT
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::cout << (char*)GetClosestEntity(player, entities)->name << std::endl;

		// Aimbot
		// ----------------------------
		AimAt(player, GetClosestEntity(player, entities));
	}
	FreeConsole();
	FreeLibraryAndExitThread(instance, 0);
}

int __stdcall DllMain(const HMODULE instance, const std::uintptr_t reason, const void* reserved) {
	if(reason == 1){
		DisableThreadLibraryCalls(instance);
		const auto thread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(hackMain), instance, 0, nullptr);
		if (thread) {
			CloseHandle(thread);
		}
	}
	return 1;
}