// Copyright Epic Games, Inc. All Rights Reserved.


#include "mikuWhatsappWeaponComponent.h"
#include "mikuWhatsappCharacter.h"
#include "mikuWhatsappProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"


// Sets default values for this component's properties
UmikuWhatsappWeaponComponent::UmikuWhatsappWeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	ballIndicators.Reserve(30);
}

void UmikuWhatsappWeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);
	
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	
			// Spawn the projectile at the muzzle
			World->SpawnActor<AmikuWhatsappProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
	
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

bool UmikuWhatsappWeaponComponent::AttachWeapon(AmikuWhatsappCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UmikuWhatsappWeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UmikuWhatsappWeaponComponent::Fire);
		}
	}

	return true;
}

void UmikuWhatsappWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < 20; i++)
	{
		ABallsIndicator* newBall = GetWorld()->SpawnActor<ABallsIndicator>(ballsClass, FVector(-100, -100, -100), FRotator(0, 0, 0));
		if (newBall != NULL)
		{ ballIndicators.Add(newBall); }
	}
}

void UmikuWhatsappWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ensure we have a character owner
	if (Character != nullptr)
	{
		// remove the input mapping context from the Player Controller
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(FireMappingContext);
			}
		}
	}

	// maintain the EndPlay call chain
	Super::EndPlay(EndPlayReason);
}

void UmikuWhatsappWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
	if (CameraManager)
	{
		FRotator SpawnRotation = CameraManager->GetCameraRotation();
		FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

		FVector Velocity = CameraManager->GetActorForwardVector() * 3000;
		float MaxSimTime = 5.f;
		FPredictProjectilePathParams Params = FPredictProjectilePathParams(10.f, SpawnLocation, Velocity, MaxSimTime);
		Params.bTraceWithCollision = true;
		Params.bTraceComplex = true;
		Params.ActorsToIgnore = bullets;

		Params.DrawDebugType = EDrawDebugTrace::None;
		Params.SimFrequency = 30.f;
		Params.OverrideGravityZ = 0.f;
		Params.ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
		Params.bTraceWithChannel = false;

		FPredictProjectilePathResult Presult;
		bool bHit = UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Presult);

		FHitResult OutHit = Presult.HitResult;
		FVector OutLastTraceDestination = Presult.LastTraceDestination.Location;
		TArray<FVector> OutPathLocations;
		OutPathLocations.Empty(Presult.PathData.Num());
		for (const FPredictProjectilePathPointData& PathPoint : Presult.PathData)
		{
			OutPathLocations.Add(PathPoint.Location);
		}

		for (int i = 0; i < ballIndicators.Num(); i++)
		{
			if (OutPathLocations.IsValidIndex(i))
			{
				FVector newLocation = OutPathLocations[i]; 
				ballIndicators[i]->SetActorLocation(newLocation);
			}
			else
			{
				ballIndicators[i]->SetActorLocation(FVector(0, 0, 0));
			}
		}
	}
}