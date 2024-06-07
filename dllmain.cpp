#include "hooks.hpp"

DWORD WINAPI MainThread(LPVOID param) {

	AllocConsole();
	FILE* Dummy;
	freopen_s(&Dummy, "CONOUT$", "w", stdout);
	freopen_s(&Dummy, "CONIN$", "r", stdin);

	uintptr_t BaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

	printf("Module Base Address: %p\n", (void*)BaseAddress);

	if (Offsets::Init()) {
		Offsets::ClientWasKicked = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"PlayerController.ClientWasKicked", false))->ExecFunction;
		Offsets::OnControlMappingsRebuilt = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"PilgrimGame.OnControlMappingsRebuilt", false))->ExecFunction;
		Offsets::UAC_SendClientHello = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"UACNetworkComponent.SendClientHello", false))->ExecFunction;
		Offsets::UAC_SendClientHello = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"UACNetworkComponent.SendClientHello", false))->ExecFunction;
		Offsets::UAC_SendPacketToClient = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"UACNetworkComponent.SendPacketToClient", false))->ExecFunction;
		Offsets::UAC_SendPacketToServer = (uintptr_t)static_cast<UFunction*>(StaticFindObject(nullptr, nullptr, L"UACNetworkComponent.SendPacketToServer", false))->ExecFunction;

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

