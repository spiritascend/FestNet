#pragma once

#include <thread>
#include "Dependencies/memcury.h"
#include "Dependencies/unreal_structs.h"
#include "Dependencies/minhook/MinHook.h"
#include "Dependencies/globaloffsets.h"
#include "Dependencies/unreal.h"
#include "Dependencies/curl/curl.h"


#pragma comment(lib, "Dependencies/minhook/minhook.lib")

namespace Hooks {

#define CleanupMinHook() \
    MH_DisableHook(MH_ALL_HOOKS); \
    MH_Uninitialize()

    GSConnectionState ConnectionState = GSConnectionState::Connected;

    static void (*UEngine_BroadcastNetworkFailure)(UObject* Engine, UObject* World, UObject* NetDriver, unsigned int FailureType, const FString* ErrorString);
    static bool (*NetConnection_Tick)(UObject* Connection);
    static void (*UNetConnection_ReceivedPacket)(UObject* Connection, void* Reader, bool bIsReinjectedPacket, bool bDispatchPacket);
    static void (*UPilgrimGame_ClearHitOffsetAverage)(UObject* PilgrimGame);
    static __int64 (*ClientWasKickedNative)(__int64 a1, __int64 a2);
    static void (*ClientReturnToMainMenu)(UObject* PlayerController, __int64 Reason);
    static void (*ReturnToMainMenuError)(UObject* PlayerController, wchar_t** Reason);
    static CURLcode(*CurlEasySetOpt)(CURL* curl, CURLoption option, ...);



    static void (*APlayerController_execClientWasKicked)(UObject* Context, void* Stack, void* const Z_Param__Result);
    static void (*PilgrimGame_OnControlMappingsRebuilt)(UObject* Context, void* Stack, void* const Z_Param__Result);
    static void (*UACExec)(UObject* Context, void* Stack, void* const Z_Param__Result);


    void __fastcall UEngine_BroadcastNetworkFailure_Hook(UObject* Engine,UObject* World,UObject* NetDriver,unsigned int FailureType,const FString* ErrorString) {
        if (ConnectionState == GSConnectionState::Connected) {
            FString Message = L"took to long to select song.";
            return UEngine_BroadcastNetworkFailure(Engine, World, NetDriver, 3, &Message);
        }
        return;
    }


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

    __int64 __fastcall ClientWasKickedNative_Hook (__int64 a1, __int64 a2) {
        return NULL;
    }


    static void ClientReturnToMainMenu_Hook(UObject* PlayerController, __int64 Reason) {
        if (ConnectionState == GSConnectionState::Disconnected) {
            printf("Re-Connecting Client\n");
            ConnectionState = GSConnectionState::Connected;
        }
        return ClientReturnToMainMenu(PlayerController,Reason);
    }

    static void ReturnToMainMenuError_Hook(UObject* PlayerController, wchar_t** Reason) {
        return;
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

        return PilgrimGame_OnControlMappingsRebuilt(Context, Stack, Z_Param__Result);
    }


    static void APlayerController_execClientWasKicked_Hook(UObject* Context, void* Stack, void* const Z_Param__Result)
    {
        if (ConnectionState == GSConnectionState::Disconnected) return;
        return APlayerController_execClientWasKicked(Context, Stack, Z_Param__Result);
    }


   static void UACFunc_Hook(UObject* Context, void* Stack, void* const Z_Param__Result) 
   { 
       return;
   }

   inline CURLcode CurlEasySetOpt_Hook(CURL* curl, CURLoption option, ...)
   {
       va_list args;
       va_start(args, option);

       if (option == CURLOPT_URL)
       {
           const char* url = va_arg(args, const char*);

           if (strstr(url, "datarouter") != nullptr) {
               url = "http://0.0.0.0/";
           }

           return CurlEasySetOpt(curl, option, url);
       }

       CURLcode result = CurlEasySetOpt(curl, option, va_arg(args, void*));
       va_end(args);

       return result;
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

        MH_CreateHook((LPVOID)Offsets::ClientWasKickedNative, ClientWasKickedNative_Hook , (LPVOID*)&ClientWasKickedNative);
        MH_EnableHook((LPVOID)Offsets::ClientWasKickedNative);

        MH_CreateHook((LPVOID)Offsets::BroadcastNetworkFailure, UEngine_BroadcastNetworkFailure_Hook, (LPVOID*)&UEngine_BroadcastNetworkFailure);
        MH_EnableHook((LPVOID)Offsets::BroadcastNetworkFailure);

        MH_CreateHook((LPVOID)Offsets::ClientReturnToMainMenu, ClientReturnToMainMenu_Hook, (LPVOID*)&ClientReturnToMainMenu);
        MH_EnableHook((LPVOID)Offsets::ClientReturnToMainMenu);

        MH_CreateHook((LPVOID)Offsets::ReturnToMainMenuError, ReturnToMainMenuError_Hook, (LPVOID*)&ReturnToMainMenuError);
        MH_EnableHook((LPVOID)Offsets::ReturnToMainMenuError);

        MH_CreateHook((LPVOID)Offsets::OnControlMappingsRebuilt, PilgrimGame_OnControlMappingsRebuilt_Hook, (LPVOID*)&PilgrimGame_OnControlMappingsRebuilt);
        MH_EnableHook((LPVOID)Offsets::OnControlMappingsRebuilt);

        MH_CreateHook((LPVOID)Offsets::CurlEasySetOpt, CurlEasySetOpt_Hook, (LPVOID*)&CurlEasySetOpt);
        MH_EnableHook((LPVOID)Offsets::CurlEasySetOpt);

        MH_CreateHook((LPVOID)Offsets::UAC_SendClientHello, UACFunc_Hook, (LPVOID*)&UACExec);
        //MH_EnableHook((LPVOID)Offsets::UAC_SendClientHello);

        MH_CreateHook((LPVOID)Offsets::UAC_SendClientHello, UACFunc_Hook, (LPVOID*)&UACExec);
        //MH_EnableHook((LPVOID)Offsets::UAC_SendClientHello);

        MH_CreateHook((LPVOID)Offsets::UAC_SendPacketToClient, UACFunc_Hook, (LPVOID*)&UACExec);
        //MH_EnableHook((LPVOID)Offsets::UAC_SendPacketToClient);

        MH_CreateHook((LPVOID)Offsets::UAC_SendPacketToServer, UACFunc_Hook, (LPVOID*)&UACExec);
        // MH_EnableHook((LPVOID)Offsets::UAC_SendPacketToServer);

        return true;
    }
}
