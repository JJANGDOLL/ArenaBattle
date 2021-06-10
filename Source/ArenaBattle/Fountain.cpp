// Fill out your copyright notice in the Description page of Project Settings.

#include "Fountain.h"


// Sets default values
AFountain::AFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 자식 오브젝트들 생성
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BODY"));
    Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WATER"));
    Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("LIGHT"));
    Splash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SPLASH"));

    // 오브젝트의 스태틱 매시 값을 에디터에서 설정하는 것이 아닌  코드 상으로 해당 메시의 경로를 알아와서 적용하는 과정
    // 분수 스태틱 메시 레퍼런스 복사 값 :  StaticMesh'/Game/InfinityBladeGrassLands/Environments/Plains/Env_Plains_Ruins/StaticMesh/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_BODY(TEXT("/Game/InfinityBladeGrassLands/Environments/Plains/Env_Plains_Ruins/StaticMesh/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01"));
    if (SM_BODY.Succeeded())
    {
        Body->SetStaticMesh(SM_BODY.Object);
    }

    // 분수 스태틱 메시 2 :  StaticMesh'/Game/InfinityBladeGrassLands/Effects/FX_Meshes/Env/SM_Plains_Fountain_02.SM_Plains_Fountain_02'
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_WATER(TEXT("/Game/InfinityBladeGrassLands/Effects/FX_Meshes/Env/SM_Plains_Fountain_02.SM_Plains_Fountain_02"));
    if (SM_WATER.Succeeded())
    {
        Water->SetStaticMesh(SM_WATER.Object);
    }

    // 분수 스태틱 메시 2 :  ParticleSystem'/Game/InfinityBladeGrassLands/Effects/FX_Ambient/Water/P_Water_Fountain_Splash_Base_01.P_Water_Fountain_Splash_Base_01'
    static ConstructorHelpers::FObjectFinder<UParticleSystem> PS_SPLASH(TEXT("/Game/InfinityBladeGrassLands/Effects/FX_Ambient/Water/P_Water_Fountain_Splash_Base_01.P_Water_Fountain_Splash_Base_01"));
    if (PS_SPLASH.Succeeded())
    {
        Splash->SetTemplate(PS_SPLASH.Object);
    }

    // 루트 컴포넌트 지정(분수 몸체)
    RootComponent = Body;

    // 오브젝트를 루트 컴포넌트 하위로 붙임
    Water->SetupAttachment(Body);
    Light->SetupAttachment(Body);
    Splash->SetupAttachment(Body);

    // 오브젝트를 루트 컴포넌트 기준으로 상대 위치만큼 이동
    Water->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
    Light->SetRelativeLocation(FVector(0.0f, 0.0f, 195.0f));
    Splash->SetRelativeLocation(FVector(0.0f, 0.0f, 195.0f));
}

// Called when the game starts or when spawned
void AFountain::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

