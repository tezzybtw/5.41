#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>
#include <iostream>
#include <thread>
#include "Includes/MinHook/Include/MinHook.h"
#include "Includes/SDK/SDK.hpp"
using namespace SDK;

class Utils
{
public:
	static Utils* Get()
	{
		static Utils* Instance = new Utils();
		return Instance;
	}
public:
	inline static void Hook(uint64_t Address, PVOID Hook, void** Original = nullptr)
	{
		MH_CreateHook(PVOID(Address), Hook, Original);
		MH_EnableHook(PVOID(Address));
	}

	inline static void VTable(void* Index, uintptr_t Target, void* Vft, void** Original = nullptr)
	{
		void** VTable = *(void***)Index;

		if (Original) *Original = VTable[Target];

		DWORD Protect;
		VirtualProtect(&VTable[Target], sizeof(void*), PAGE_EXECUTE_READWRITE, &Protect);

		VTable[Target] = Vft;

		DWORD Temp;
		VirtualProtect(&VTable[Target], sizeof(void*), Protect, &Temp);
	}

	template <class T>
	inline static T* SpawnActor(FVector Location = FVector(), FRotator Rotation = FRotator(0, 0, 0), UClass* InClass = T::StaticClass(), AActor* Owner = nullptr)
	{
		FTransform Transform = FTransform{};
		Transform.Rotation = UKismetMathLibrary::Conv_RotatorToTransform(Rotation).Rotation;
		Transform.Translation = Location;
		Transform.Scale3D = { 1, 1, 1 };

		AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), InClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
		if (Actor) UGameplayStatics::FinishSpawningActor(Actor, Transform);

		return (T*)Actor;
	}

	inline static void ReturnNull() {}
	inline static bool ReturnTrue() { return true; }
	inline static bool ReturnFalse() { return false; }
};