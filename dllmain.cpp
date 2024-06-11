#include "hooks.hpp"
#include <iostream>

DWORD WINAPI MainThread(LPVOID param) {
	// sleep 5 seconds so UE can prepare itself (if we load at process start rather than post-launch injection)
	Sleep(5000);

	// in Release builds check env variable FESTNET_DEBUG to see if we should enable the console
	// the console causes issues in gamescope
#ifdef NDEBUG
	if (GetEnvironmentVariableA("FESTNET_DEBUG", NULL, 0) != 0) {
#endif
		AllocConsole();
		FILE* Dummy;
		freopen_s(&Dummy, "CONOUT$", "w", stdout);
		freopen_s(&Dummy, "CONIN$", "r", stdin);
#ifdef NDEBUG
	}
#endif

	uintptr_t BaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

	printf("Module Base Address: %p\n", (void*)BaseAddress);

	if (Offsets::Init()) {
		Offsets::ClientWasKicked = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"PlayerController.ClientWasKicked", false))->ExecFunction;
		Offsets::OnControlMappingsRebuilt = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"PilgrimGame.OnControlMappingsRebuilt", false))->ExecFunction;
		Offsets::UAC_SendClientHello = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"UACNetworkComponent.SendClientHello", false))->ExecFunction;
		Offsets::UAC_SendPacketToClient = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"UACNetworkComponent.SendPacketToClient", false))->ExecFunction;
		Offsets::UAC_SendPacketToServer = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"UACNetworkComponent.SendPacketToServer", false))->ExecFunction;
		Offsets::ProcessEvent_VTableIndex = GetProcessEventIndexFromObject(StaticFindObject(nullptr, nullptr, L"CoreUObject.Object", false));
		
		if (Hooks::ApplyHooks())
		{
			printf("Applied Hooks\n");
		}
	}

	return 0;
}

volatile BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
		break;
	}

	return TRUE;
}

