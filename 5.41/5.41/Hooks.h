#pragma once

#include "pch.h"

enum ENetMode
{
	Standalone,
	DedicatedServer,
	ListenServer,
	Client,
	MAX
};

class Hooks
{
public:
	class Originals
	{
	public:
		inline static void (*TickFlush)(UNetDriver* _This, float DeltaSeconds);
		inline static void (*KickPlayer)(AGameSession* _This, AController* Controller);
		inline static void (*ServerAcknowledgePossession)(APlayerController* _This, APawn* Pawn);
		inline static void (*DispatchRequest)(__int64 a1, unsigned __int64* a2, int a3);
		inline static bool (*ReadyToStartMatch)(AFortGameModeAthena* _This);
	};
public:
	static void TickFlush(UNetDriver* _This, float DeltaSeconds)
	{
		if (_This->ReplicationDriver) { _This->ReplicationDriver->ServerReplicateActors(DeltaSeconds); }

		return Originals::TickFlush(_This, DeltaSeconds);
	}

	static ENetMode GetNetMode(UWorld* _This) { return ENetMode::DedicatedServer; }

	static void KickPlayer(AGameSession* _This, AController* Controller) { return; }

	static void ServerAcknowledgePossession(APlayerController* _This, APawn* Pawn)
	{
		_This->AcknowledgedPawn = Pawn;

		AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)_This;
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;

		PlayerState->ApplyCharacterCustomization(PlayerController->MyFortPawn);

		return Originals::ServerAcknowledgePossession(_This, Pawn);
	}

	static void DispatchRequest(__int64 a1, unsigned __int64* a2, int a3) { return Originals::DispatchRequest(a1, a2, 3); }

	static APawn* SpawnDefaultPawnFor(AGameModeBase* _This, AController* NewPlayer, AActor* StartSpot)
	{
		return _This->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
	}

	static bool ReadyToStartMatch(AFortGameModeAthena* _This)
	{
		if (!_This || !_This->GameState) return false;

		AFortGameStateAthena* GameState = (AFortGameStateAthena*)_This->GameState;

		float CurrentTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());
		float WarmupTime = 30.f;

		static bool Init = false;
		if (!Init)
		{
			Init = true;

			UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");

			GameState->CurrentPlaylistData = Playlist;
			GameState->OnRep_CurrentPlaylistData();

			_This->CurrentPlaylistId = Playlist->PlaylistId;
			_This->CurrentPlaylistName = Playlist->PlaylistName;
			_This->FortGameSession->MaxPlayers = Playlist->MaxPlayers;
			_This->FortGameSession->MaxPartySize = Playlist->MaxSocialPartySize;
		}

		static bool Initialized = false;
		if (!Initialized)
		{
			Initialized = true;

			FName NetDriverDefinition = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
			UNetDriver* NetDriver = UEngine::GetEngine()->CreateNetDriver(UWorld::GetWorld(), NetDriverDefinition);

			NetDriver->World = UWorld::GetWorld();
			NetDriver->NetDriverName = NetDriverDefinition;

			FString Error;
			FURL URL;
			URL.Port = 7777;

			if (!NetDriver->InitListen(UWorld::GetWorld(), URL, false, Error)) return false;

			NetDriver->SetWorld(UWorld::GetWorld());

			UWorld::GetWorld()->NetDriver = NetDriver;

			for (FLevelCollection& LevelCollection : UWorld::GetWorld()->LevelCollections) { LevelCollection.NetDriver = UWorld::GetWorld()->NetDriver; }

			_This->bWorldIsReady = true;
		}

		return _This->AlivePlayers.Num() > 0;
	}
};