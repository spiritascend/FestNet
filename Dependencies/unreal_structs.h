#pragma once


#include "unreal_enums.h"
#include "globaloffsets.h"



template<class T>
struct TArray
{

	friend struct FString;
public:
	T* Data;
	int32_t Count;
	int32_t Max;

	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline int Num() const
	{
		return Count;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline const T& operator[](int i) const
	{
		return Data[i];
	};

	inline bool IsValidIndex(int i) const
	{
		return i < Num();
	}


	inline void Add(T InputData)
	{
		Data = (T*)realloc(Data, sizeof(T) * (Count + 1));
		Data[Count++] = InputData;
		Max = Count;
	};

	inline void Remove(int32_t Index)
	{
		TArray<T> NewArray;
		for (size_t i = 0; i < this->Count; ++i)
		{
			if (i == Index)
				continue;

			NewArray.Add(this->operator[](i));
		}
		this->Data = (T*)realloc(NewArray.Data, sizeof(T) * (NewArray.Count));
		this->Count = NewArray.Count;
		this->Max = NewArray.Count;
	}
};

struct FString : private TArray<wchar_t>
{
	FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	}

	bool IsValid() const
	{
		return Data != nullptr;
	}

	const wchar_t* ToWString() const
	{
		return Data;
	}

	std::string ToString() const
	{
		auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};


class FName
{
public:
	int32_t ComparisonIndex;
	int32_t Number;

	inline std::string ToString() const {
		FString outname;
		reinterpret_cast<__int64(*)(const FName*, FString*)>(Offsets::FName_ToString)(this,&outname);
		return outname.ToString();
	}
};


class UObject
{
public:
	void* Vft;
	int32_t Flags;
	int32_t Index;
	UObject* Class;
	FName    Name;
	class UObject* Outer;

	std::string GetName()
	{
		return Name.ToString();
	}

	std::string GetFullName()
	{
		std::string temp;

		for (auto outer = Outer; outer; outer = outer->Outer)
		{
			temp = outer->GetName() + "." + temp;
		}

		temp = reinterpret_cast<UObject*>(Class)->GetName() + " " + temp + this->GetName();
		return temp;
	}

	inline bool ProcessEvent(UObject* pFunction, void* pParams)
	{
		auto vtable = *reinterpret_cast<void***>(this);
		auto ProcesseventVtable = static_cast<void(*)(void*, void*, void*)>(vtable[Offsets::ProcessEvent_VTableIndex]); if (!ProcesseventVtable) return false;
		ProcesseventVtable(this, pFunction, pParams);
		return true;
	}
};


class FField;


class FFieldVariant
{
public:
	union FFieldObjectUnion
	{
		FField* Field;
		UObject* Object;
	} Container;
};

class FField
{
public:
	void* VTableObject;
	UObject* ClassPrivate;
	FFieldVariant Owner;
	FField* Next;
	FName NamePrivate;
};


template <class TEnum>
class TEnumAsByte
{
public:
	TEnumAsByte()
	{
	}

	TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	operator TEnum() const
	{
		return static_cast<TEnum>(value);
	}

	TEnum GetValue() const
	{
		return static_cast<TEnum>(value);
	}

private:
	uint8_t value;
};


struct FProperty : FField
{
	int32_t ArrayDim;
	int32_t ElementSize;
	EPropertyFlags PropertyFlags;
	uint16_t RepIndex;
	TEnumAsByte<ELifetimeCondition> BlueprintReplicationCondition;
	int32_t Offset_Internal;
	FName RepNotifyFunc;
	FProperty* PropertyLinkNext;
	FProperty* NextRef;
	FProperty* DestructorLinkNext;
	FProperty* PostConstructLinkNext;
};

class UField : public UObject
{
public:
	UField* Next;
};

class UStruct : public UField
{
public:
	uint8_t Pad_38[0x10];
	class UStruct* Super;
	class UField* Children;
	class FField* ChildProperties;
	int32_t Size;
	int32_t MinAlignemnt;
	uint8_t Pad_39[0x50];
};


using FNativeFuncPtr = void (*)(void* Context, void* TheStack, void* Result);

class UFunction : public UStruct
{
public:
	uint32_t FunctionFlags;
	uint8_t  Pad_3F[0x20];
	FNativeFuncPtr ExecFunction;
};

struct FNativeFunctionLookup
{
	FName Name;
	void* Pointer;
};

class UClass : public UStruct
{
public:
	void* ClassConstructor;
	void* ClassVTableHelperCtorCaller;
	void* ClassAddReferencedObjects;
	mutable uint32_t ClassUnique : 31;
	uint32_t bCooked : 1;
	EClassFlags ClassFlags;
	EClassCastFlags ClassCastFlags;
	UClass* ClassWithin;
	UObject* ClassGeneratedBy;
	FName ClassConfigName;
	TArray<uint8_t> ClassReps;
	TArray<UField*> NetFields;
	int32_t FirstOwnedClassRep = 0;
	UObject* ClassDefaultObject;
	void* SparseClassData;
	void* SparseClassDataStruct;
	char FuncMap[0x50];
	char SuperFuncMap[0x50];
	void* SuperFuncMapLock;
	TArray<uint8_t> Interfaces;
	TArray<uint32_t> ReferenceTokenStream;
	// TArray<FName> TokenDebugInfo;
	char ReferenceTokenStreamCritical[0x28];
	TArray<FNativeFunctionLookup> NativeFunctionLookupTable;
};




struct FNameNativePtrPair
{
	const char* NameUTF8;
	FNativeFuncPtr Pointer;
};


struct FPilgrimHistogramSample final {
	int32_t OffsetMs;
	int32_t NumSamples;
};


struct FPilgrimQuickplayPlayerPerformanceData {
	float Accuracy;
	float AverageOffset;
	float StandardDeviation;
	bool FullCombo;
	uint8_t Pad_9605[0x3];
	int32_t NotesHit;
	int32_t NotesPassed;
	int32_t NotesMissed;
	int32_t TotalNotes;
	int32_t LongestStreak;
	float TimeInOverdriveMs;
	TArray <FPilgrimHistogramSample> HistogramSamples;
	TArray <int32_t> AccuracyTierCounts;
};