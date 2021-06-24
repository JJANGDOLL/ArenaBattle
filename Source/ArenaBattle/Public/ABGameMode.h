// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "ABCharacter.h"
#include "ABGameMode.generated.h"


/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	AABGameMode();
	
public:
    virtual void PostInitializeComponents() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
	void AddScore(class AABPlayerController * ScoredPlayer);

private:
    UPROPERTY()
    class AABGameState* ABGameState;
};
