// Fill out your copyright notice in the Description page of Project Settings.

#include "CPPTestFountain.h"


// Sets default values
ACPPTestFountain::ACPPTestFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    RotateSpeed = 0.0f;
}

// Called when the game starts or when spawned
void ACPPTestFountain::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACPPTestFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPPTestFountain::RotateFountain(float DeltaTime)
{
    AddActorLocalRotation(FRotator(0.0f, RotateSpeed * DeltaTime, 0.0f));
}
