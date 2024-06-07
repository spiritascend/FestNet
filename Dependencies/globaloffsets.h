#pragma once
#include <cstdint>
#include "memcury.h"

namespace Offsets {
#define Check(str, offset) \
    if (offset == 0) { \
        char message[256]; \
        sprintf_s(message, sizeof(message), "Failed to find offset for %s", str); \
        MessageBoxA(NULL, message, "Error", MB_OK | MB_ICONERROR); \
		return false; \
    }

	static uintptr_t NetConnection_Tick;
	static uintptr_t ReceivedPacket;
	static uintptr_t StaticFindObject;
	static uintptr_t ClearHitOffsetAverage;
	static uintptr_t SetRuleSetForTrackType;

	static uintptr_t ClientWasKicked;
	static uintptr_t ClientReturnToMainMenu;
	static uintptr_t OnControlMappingsRebuilt;
	static uintptr_t UAC_SendClientHello;
	static uintptr_t UAC_SendPacketToClient;
	static uintptr_t UAC_SendPacketToServer;

	static bool Init() {
		StaticFindObject = Memcury::Scanner::FindStringRef(L"ServerStatReplicatorInst").ScanFor({ 0xE9 }, true).RelativeOffset(1).Get();
		Check("StaticFindObject", StaticFindObject);

		ReceivedPacket = Memcury::Scanner::FindPattern("48 8B 45 F0 48 8D 5D B0 48 85 C0").ScanFor({ 0x48, 0x8B, 0xC4 }, false).Get();
		Check("ReceivedPacket", ReceivedPacket);

		NetConnection_Tick = Memcury::Scanner::FindStringRef(L"Stat_NetConnectionTick").ScanFor({ 0x48, 0x8B, 0xC4 }, false).Get();
		Check("NetConnection_Tick", NetConnection_Tick);

		ClearHitOffsetAverage = Memcury::Scanner::FindStringRef("UPilgrimGame::ClearHitOffsetAverage").ScanFor({ 0x40, 0x53 }, false).Get();
		Check("ClearHitOffsetAverage", ClearHitOffsetAverage);

		ClientReturnToMainMenu = Memcury::Scanner::FindStringRef(L"Return To Main Menu").ScanFor({ 0x48, 0x8B, 0xC4 }, false).Get();
		Check("ClientReturnToMainMenu", ClientReturnToMainMenu);
	}
}