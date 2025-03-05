// Copyright Epic Games, Inc. All Rights Reserved.

#include "mikuWhatsappGameMode.h"
#include "mikuWhatsappCharacter.h"
#include "UObject/ConstructorHelpers.h"

AmikuWhatsappGameMode::AmikuWhatsappGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
