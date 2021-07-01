// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "GameFramework/Actor.h"
#include "CPPTestFountain.generated.h"

UCLASS()
class ARENABATTLE_API ACPPTestFountain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPPTestFountain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;	

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoatationSpeed)
    float RotateSpeed;

public:
    UFUNCTION(BlueprintCallable)
    void RotateFountain(float DeltaTime);
};
