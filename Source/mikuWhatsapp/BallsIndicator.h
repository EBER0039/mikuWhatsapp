// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallsIndicator.generated.h"

UCLASS()
class MIKUWHATSAPP_API ABallsIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABallsIndicator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ballObject;

	UPROPERTY(EditAnywhere)
	UStaticMesh* ballMesh;

	UPROPERTY(EditAnywhere)
	UMaterial* ballMat;

};
