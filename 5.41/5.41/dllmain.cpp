#include "pch.h"
#include "Hooks.h"

void Main()
{
    AllocConsole();
    
    FILE* File = nullptr;

    freopen_s(&File, "CONOUT$", "w+", stdout);
    freopen_s(&File, "CONIN$", "r", stdin);
    freopen_s(&File, "CONOUT$", "w+", stderr);

    SetConsoleTitleA("5.41: Setting Up | Made By Tezzy!");

    if (MH_Initialize() != MH_OK) { ExitThread(0); }

    Sleep(2500);

    SetConsoleTitleA("5.41: Patching | Made By Tezzy!");
    *reinterpret_cast<bool*>(InSDKUtils::GetImageBase() + 0x531F204) = false; // GIsClient
    *reinterpret_cast<bool*>(InSDKUtils::GetImageBase() + 0x531F205) = true; // GIsServer

    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x264BA00, Hooks::TickFlush, (void**)&Hooks::Originals::TickFlush);
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x291E920, Hooks::GetNetMode);
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x10169A0, Hooks::KickPlayer, (void**)&Hooks::Originals::KickPlayer);

    Utils::Get()->VTable(AFortPlayerControllerAthena::GetDefaultObj(), 0x105, Hooks::ServerAcknowledgePossession, (void**)&Hooks::Originals::ServerAcknowledgePossession);

    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x8F3B60, Hooks::DispatchRequest, (void**)&Hooks::Originals::DispatchRequest);
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0xC017A0, Hooks::SpawnDefaultPawnFor);
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0xBFB790, Hooks::ReadyToStartMatch, (void**)&Hooks::Originals::ReadyToStartMatch);

    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0xC62260, Utils::ReturnNull);
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x21B06F0, Utils::ReturnNull);
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x11E6180, Utils::ReturnNull);
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0xE8F790, Utils::ReturnNull);

    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x60F7E0, Utils::ReturnTrue); // CanActivateAbility

    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0x17C8960, Utils::ReturnFalse); // CollectGarbage
    Utils::Get()->Hook(InSDKUtils::GetImageBase() + 0xEE51A0, Utils::ReturnFalse); // NoMCP

    Sleep(1000);

    SetConsoleTitleA("5.41: Loading World | Made By Tezzy!");
    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
    UGameplayStatics::OpenLevel(UWorld::GetWorld(), UKismetStringLibrary::Conv_StringToName(L"Athena_Terrain"), false, FString());
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ReasonForCall, LPVOID lpReserved)
{
    switch (ReasonForCall)
    {
    case DLL_PROCESS_ATTACH:
        std::thread(Main).detach();
        break;
    default:
        break;
    }
    return TRUE;
}