#pragma once

#include "globaloffsets.h"
#include "unreal_structs.h"

static UObject* StaticFindObject(UObject* ObjectClass, UObject* ObjectPackage, const wchar_t* OrigInName, bool bExactClass) {
	return reinterpret_cast<UObject * (__fastcall*)(UObject*, UObject*, const wchar_t*, bool)>(Offsets::StaticFindObject)(ObjectClass, ObjectPackage, OrigInName, bExactClass);
}

#define StaticLoadObjectEasy(Path) StaticLoadObject((UObject*)-1, nullptr, Path);
static UObject* StaticLoadObject(UObject* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* FileName = nullptr, uint32_t LoadFlags = 0, void* Sandbox = nullptr, bool bAllowObjectReconciliation = false, void* InstancingContext = nullptr)
{
	return reinterpret_cast<UObject * (__fastcall*)(UObject*, UObject*, const TCHAR*, const TCHAR*, uint32_t, void*, bool, void*)>(Offsets::StaticLoadObject)(Class, InOuter, Name, FileName, LoadFlags, Sandbox, bAllowObjectReconciliation, InstancingContext);
}


static UObject* GetLocalPlayer() {
	return reinterpret_cast<UObject * (__fastcall*)()>(Offsets::GetLocalPlayer)();
}

FString* __fastcall UPlayer_ConsoleCommand(UObject* Player, FString* result, const FString* Cmd, bool bWriteToLog) {
	return reinterpret_cast<FString*(__fastcall*)(UObject*,FString*,const FString*,bool)>(Offsets::ExecuteConsoleCommand)(Player,result,Cmd,bWriteToLog);
}


static std::string QuickplayState_ToString(EPilgrimQuickplayStateMachine_NameState StateID) {
	switch (StateID) {
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_Loading:
		return "PilgrimState_Loading";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_PreGame:
		return "PilgrimState_PreGame";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_PreIntro:
		return "PilgrimState_PreIntro";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_Intro:
		return "PilgrimState_Intro";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_SongGameplay:
		return "PilgrimState_SongGameplay";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_Outro:
		return "PilgrimState_Outro";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_SongResult:
		return "PilgrimState_SongResult";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_SetResults:
		return "PilgrimState_SetResults";
	case EPilgrimQuickplayStateMachine_NameState::PilgrimState_Request_StateDone:
		return "PilgrimState_Request_StateDone";
	default:
		return "PilgrimState_Undefined";
	}
}


// Notes: EObjectFlags::RF_ClassDefaultObject to exclude default objects and also EInternalObjectFlags::GUnreachableObjectFlag for objects that are pending destruction
void __fastcall GetObjectsOfClass_Internal(
	UObject* ClassToLookFor,
	TArray<UObject*>* Results,
	bool bIncludeDerivedClasses = true,
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject,
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::Unreachable)
{
	return reinterpret_cast<void(__fastcall*)(UObject*, TArray<UObject*>*, bool, EObjectFlags, EInternalObjectFlags)>(Offsets::GetObjectsOfClass)(ClassToLookFor,Results,bIncludeDerivedClasses,ExclusionFlags, ExclusionInternalFlags);
}

TArray<UObject*> GetObjectsOfClass(UObject* Class) {
	TArray<UObject*> Results;
	GetObjectsOfClass_Internal(Class, &Results);
	return Results;
}


UObject* GetObjectOfClass(UObject* Class) {
	TArray<UObject*> Results;
	if (Class) {
		GetObjectsOfClass_Internal(Class, &Results);
		if (Results.Num() > 0) {
			return Results[0];
		}
	}
	return nullptr;
}

static int GetProcessEventIndexFromObject(UObject* pObject)
{
	auto vtable = *reinterpret_cast<void***>(pObject);

	for (int i = 0; i < 140; i++)
	{
		auto const bytes = (uint8_t*)vtable[i];

		if (bytes[0] == 0x40 && bytes[1] == 0x55)
		{
			if (reinterpret_cast<uint8_t*>(bytes + 2)[0] == 0x56)
			{
				return i;
			}
		}
	}
	return 0;
}