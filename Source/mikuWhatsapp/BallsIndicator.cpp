// Fill out your copyright notice in the Description page of Project Settings.


#include "BallsIndicator.h"

// Sets default values
ABallsIndicator::ABallsIndicator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ballObject = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyBody"));

	ballObject->SetStaticMesh(ballMesh);
	ballObject->SetupAttachment(RootComponent);

	ballObject->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));

	ballObject->SetMaterial(0, ballMat);
}

// Called when the game starts or when spawned
void ABallsIndicator::BeginPlay()
{
	Super::BeginPlay();
	
	ballObject->SetStaticMesh(ballMesh);
	ballObject->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));
	ballObject->SetMaterial(0, ballMat);
}

// Called every frame
void ABallsIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

