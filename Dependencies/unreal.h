#pragma once

#include "globaloffsets.h"
#include "unreal_structs.h"

static UObject* StaticFindObject(UObject* ObjectClass, UObject* ObjectPackage, const wchar_t* OrigInName, bool bExactClass) {
	return reinterpret_cast<UObject * (__fastcall*)(UObject*, UObject*, const wchar_t*, bool)>(Offsets::StaticFindObject)(ObjectClass, ObjectPackage, OrigInName, bExactClass);
}
