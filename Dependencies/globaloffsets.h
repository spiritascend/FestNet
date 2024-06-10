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


	static uintptr_t FName_ToString;
	static uintptr_t NetConnection_Tick;
	static uintptr_t ReceivedPacket;
	static uintptr_t StaticFindObject;
	static uintptr_t ClearHitOffsetAverage;
	static uintptr_t StaticLoadObject;
	static uintptr_t CurlEasySetOpt;
	static uintptr_t BroadcastNetworkFailure;
	static uintptr_t ClientWasKickedNative;
	static uintptr_t GetObjectsOfClass;
	static uintptr_t ReturnToMainMenuError;


	static uintptr_t ClientWasKicked;
	static uintptr_t ClientReturnToMainMenu;
	static uintptr_t OnControlMappingsRebuilt;
	static uintptr_t UAC_SendClientHello;
	static uintptr_t UAC_SendPacketToClient;
	static uintptr_t UAC_SendPacketToServer;


	static int ProcessEvent_VTableIndex;


	static bool Init() {
		FName_ToString = Memcury::Scanner::FindStringRef(L"Loading Module %s").ScanFor({ Memcury::ASM::CALL }, false).RelativeOffset(1).Get();
		Check("FName_ToString", FName_ToString);

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

		StaticLoadObject = Memcury::Scanner::FindStringRef(L"/LauncherSocial/SocialActionButtonDefaultData.SocialActionButtonDefaultData").ScanFor({ Memcury::ASM::CALL }, true).RelativeOffset(1).ScanFor({ Memcury::ASM::CALL }, true).RelativeOffset(1).Get();
		Check("StaticLoadObject", StaticLoadObject);

		CurlEasySetOpt = Memcury::Scanner::FindPattern("89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9 75 08 8D 41 2B 48 83 C4 28 C3 4C").Get();
		Check("CurlEasySetOpt", CurlEasySetOpt);

		BroadcastNetworkFailure = Memcury::Scanner::FindStringRef(L"UIpNetConnection::HandleSocketSendResult: Socket->SendTo failed with error %i (%s). %s Connection will be closed during next Tick()!").ScanFor({ 0xE8 }, true, 2).RelativeOffset(1).Get();
		Check("BroadcastNetworkFailure", BroadcastNetworkFailure);

		ClientWasKickedNative = Memcury::Scanner::FindPattern("F3 0F 5F 05 ? ? ? ? F3 0F 59 C1 F3 0F 58 C3 C3").ScanFor({ 0x48, 0x89 ,0x5C ,0x24 }, false).Get();
		Check("ClientWasKickedNative", ClientWasKickedNative);

		GetObjectsOfClass = Memcury::Scanner::FindStringRef(L"STAT_Hash_GetObjectsOfClass").ScanFor({ 0x48, 0x8B, 0xC4 }, false).Get();
		Check("GetObjectsOfClass", GetObjectsOfClass);

		ReturnToMainMenuError = Memcury::Scanner::FindStringRef(L"ReturnToMenuError-%s-%s").ScanFor({ 0x48, 0x8B, 0xC4 }, false).Get();
		Check("ReturnToMainMenuError", ReturnToMainMenuError);
		
	}
}