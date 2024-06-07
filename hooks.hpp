#pragma once

#include <thread>
#include "Dependencies/memcury.h"
#include "Dependencies/unreal_structs.h"
#include "Dependencies/minhook/MinHook.h"
#include "Dependencies/globaloffsets.h"
#include "Dependencies/unreal.h"

#pragma comment(lib, "Dependencies/minhook/minhook.lib")

namespace Hooks {

#define CleanupMinHook() \
    MH_DisableHook(MH_ALL_HOOKS); \
    MH_Uninitialize()

    GSConnectionState ConnectionState = GSConnectionState::Connected;

    static bool (*NetConnection_Tick)(UObject* Connection);
    static void (*UNetConnection_ReceivedPacket)(UObject* Connection, void* Reader, bool bIsReinjectedPacket, bool bDispatchPacket);
    static void (*UPilgrimGame_ClearHitOffsetAverage)(UObject* PilgrimGame);
    static void (*APlayerController_execClientWasKicked)(UObject* Context, void* Stack, void* const Z_Param__Result);
    static void (*ClientReturnToMainMenu)(UObject* PlayerController, __int64 Reason);
    static void (*PilgrimGame_OnControlMappingsRebuilt)(UObject* Context, void* Stack, void* const Z_Param__Result);
    static void (*UACExec)(UObject* Context, void* Stack, void* const Z_Param__Result);

    static bool NetConnection_Tick_Hook(UObject* Connection)
    {
        if (ConnectionState == GSConnectionState::Disconnected) return true;
        return NetConnection_Tick(Connection);
    }

    static void UNetConnection_ReceivedPacket_Hook(UObject* Connection, void* Reader, bool bIsReinjectedPacket, bool bDispatchPacket)
    {
        if (ConnectionState == GSConnectionState::Disconnected) return;
        return UNetConnection_ReceivedPacket(Connection, Reader, bIsReinjectedPacket, bDispatchPacket);
    }

   static void UPilgrimGame_ClearHitOffsetAverage_Hook(UObject* PilgrimGame)
    {
        if (ConnectionState == GSConnectionState::Disconnected) ConnectionState = GSConnectionState::Connected;
        return;
    }

    static void APlayerController_execClientWasKicked_Hook(UObject* Context, void* Stack, void* const Z_Param__Result)
    {
        if (ConnectionState == GSConnectionState::Disconnected) return;
        return APlayerController_execClientWasKicked(Context, Stack, Z_Param__Result);
    }

    static void PilgrimGame_OnControlMappingsRebuilt_Hook(UObject* Context, void* Stack, void* const Z_Param__Result)
    {
        auto DisconnectClient = []() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            printf("Disconnecting Client From Server\n");
            if (ConnectionState == GSConnectionState::Connected) {
                ConnectionState = GSConnectionState::Disconnected;
            }
            };

        std::thread(DisconnectClient).detach();

        return PilgrimGame_OnControlMappingsRebuilt(Context,Stack,Z_Param__Result);
    }

    static void ClientReturnToMainMenu_Hook(UObject* PlayerController, __int64 Reason) {
        if (ConnectionState == GSConnectionState::Disconnected) {
            printf("Re-Connecting Client\n");
            ConnectionState = GSConnectionState::Connected;
        }
        return ClientReturnToMainMenu(PlayerController,Reason);
    }

   static void UACFunc_Hook(UObject* Context, void* Stack, void* const Z_Param__Result) { 
       return;
   }

   static bool ApplyHooks()
    {
        if (MH_Initialize() != MH_OK) {
            printf("Failed to Initialize Minhook!\n");
            CleanupMinHook();
            return false;
        }

        MH_CreateHook((LPVOID)Offsets::NetConnection_Tick, NetConnection_Tick_Hook, (LPVOID*)&NetConnection_Tick);
        MH_EnableHook((LPVOID)Offsets::NetConnection_Tick);

        MH_CreateHook((LPVOID)Offsets::ReceivedPacket, UNetConnection_ReceivedPacket_Hook, (LPVOID*)&UNetConnection_ReceivedPacket);
        MH_EnableHook((LPVOID)Offsets::ReceivedPacket);

        MH_CreateHook((LPVOID)Offsets::ClearHitOffsetAverage, UPilgrimGame_ClearHitOffsetAverage_Hook, (LPVOID*)&UPilgrimGame_ClearHitOffsetAverage);
        MH_EnableHook((LPVOID)Offsets::ClearHitOffsetAverage);

        MH_CreateHook((LPVOID)Offsets::ClientWasKicked, APlayerController_execClientWasKicked_Hook, (LPVOID*)&APlayerController_execClientWasKicked);
        MH_EnableHook((LPVOID)Offsets::ClientWasKicked);

        MH_CreateHook((LPVOID)Offsets::ClientReturnToMainMenu, ClientReturnToMainMenu_Hook, (LPVOID*)&ClientReturnToMainMenu);
        MH_EnableHook((LPVOID)Offsets::ClientReturnToMainMenu);

        MH_CreateHook((LPVOID)Offsets::OnControlMappingsRebuilt, PilgrimGame_OnControlMappingsRebuilt_Hook, (LPVOID*)&PilgrimGame_OnControlMappingsRebuilt);
        MH_EnableHook((LPVOID)Offsets::OnControlMappingsRebuilt);

        MH_CreateHook((LPVOID)Offsets::UAC_SendClientHello, UACFunc_Hook, (LPVOID*)&UACExec);
        MH_EnableHook((LPVOID)Offsets::UAC_SendClientHello);

        MH_CreateHook((LPVOID)Offsets::UAC_SendClientHello, UACFunc_Hook, (LPVOID*)&UACExec);
        MH_EnableHook((LPVOID)Offsets::UAC_SendClientHello);

        MH_CreateHook((LPVOID)Offsets::UAC_SendPacketToClient, UACFunc_Hook, (LPVOID*)&UACExec);
        MH_EnableHook((LPVOID)Offsets::UAC_SendPacketToClient);

        MH_CreateHook((LPVOID)Offsets::UAC_SendPacketToServer, UACFunc_Hook, (LPVOID*)&UACExec);
        MH_EnableHook((LPVOID)Offsets::UAC_SendPacketToServer);

        return true;
    }
}
