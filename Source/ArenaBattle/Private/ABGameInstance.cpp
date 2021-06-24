// Fill out your copyright notice in the Description page of Project Settings.

#include "ABGameInstance.h"

UABGameInstance::UABGameInstance()
{
    // DataTable'/Game/Book/GameData/ABCharacterData.ABCharacterData'
    FString CharacterataPath = TEXT("/Game/Book/GameData/ABCharacterData.ABCharacterData");
    static ConstructorHelpers::FObjectFinder<UDataTable> DT_ABCHARACTER(*CharacterataPath);
    ABCHECK(DT_ABCHARACTER.Succeeded());
    ABCharacterTable = DT_ABCHARACTER.Object;
    ABCHECK(ABCharacterTable->RowMap.Num() > 0);
}

void UABGameInstance::Init()
{
    Super::Init();
}

FABCharcterData * UABGameInstance::GetABCharacterData(int32 Level)
{
    return ABCharacterTable->FindRow<FABCharcterData>(*FString::FromInt(Level), TEXT(""));
}
