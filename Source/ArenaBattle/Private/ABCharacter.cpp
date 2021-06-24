// Fill out your copyright notice in the Description page of Project Settings.

#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABWeapon.h"
#include "ABCharacterStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterWidget.h"
#include "ABAIController.h"
#include "ABCharacterSetting.h"
#include "ABGameInstance.h"
#include "ABPlayerController.h"
#include "ABPlayerState.h"
#include "ABHUDWidget.h"

// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));
    CharacterStat = CreateDefaultSubobject<UABCharacterStatComponent>(TEXT("CHARACTERSTAT"));
    HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBARWIDGET"));

    SpringArm->SetupAttachment(GetCapsuleComponent());
    Camera->SetupAttachment(SpringArm);
    HPBarWidget->SetupAttachment(GetMesh());

    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

    // SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard'
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CARDBOARD(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard"));
    if (SK_CARDBOARD.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(SK_CARDBOARD.Object);
    }

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

    // AnimBlueprint'/Game/Book/Animations/WarriorAnimBlueprint.WarriorAnimBlueprint'
    static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANIM(TEXT("/Game/Book/Animations/WarriorAnimBlueprint.WarriorAnimBlueprint_C"));
    if (WARRIOR_ANIM.Succeeded())
    {
        GetMesh()->SetAnimInstanceClass(WARRIOR_ANIM.Class);
    }

    FName WeaponSocket(TEXT("hand_rSocket"));
    if (GetMesh()->DoesSocketExist(WeaponSocket))
    {
        Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WEAPON"));
        // SkeletalMesh'/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_BlackKnight/SK_Blade_BlackKnight.SK_Blade_BlackKnight'
        static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_WEAPON(TEXT("/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_BlackKnight/SK_Blade_BlackKnight.SK_Blade_BlackKnight"));
        if (SK_WEAPON.Succeeded())
        {
            Weapon->SetSkeletalMesh(SK_WEAPON.Object);
        }

        Weapon->SetupAttachment(GetMesh(), WeaponSocket);
    }

    SetControlMode(EControlMode::DIABLO);

    ArmLengthSpeed = 3.0f;
    ArmRotationSpeed = 10.0f;
    GetCharacterMovement()->JumpZVelocity = 800.0f;

    IsAttacking = false;

    MaxCombo = 4;
    AttackEndComboState();

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCharacter"));
    AttackRange = 200.0f;
    AttackRadius = 50.0f;

    HPBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);

    // WidgetBlueprint'/Game/Book/UI/UI_HPBar.UI_HPBar'
    static ConstructorHelpers::FClassFinder<UUserWidget> UI_HUD(TEXT("/Game/Book/UI/UI_HPBar.UI_HPBar_C"));
    if (UI_HUD.Succeeded())
    {
        HPBarWidget->SetWidgetClass(UI_HUD.Class);
        HPBarWidget->SetDrawSize(FVector2D(150.0f, 50.0f));
    }

    AIControllerClass = AABAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    //auto DefaultSetting = GetDefault<UABCharacterSetting>();
    //if (DefaultSetting->CharacterAssets.Num() > 0)
    //{
    //    for (auto CharacterAsset : DefaultSetting->CharacterAssets)
    //    {
    //    }
    //}

    AssetIndex = 4;

    SetActorHiddenInGame(true);
    HPBarWidget->SetHiddenInGame(true);
    bCanBeDamaged = false;

    DeadTimer = 5.0f;
}

   void AABCharacter::SetCharacterState(ECharacterState NewState)
{
    ABCHECK(CurrentState != NewState);
    CurrentState = NewState;

    switch (CurrentState)
    {
        case ECharacterState::LOADING:
            if (bIsPlayer)
            {
                DisableInput(ABPlayerController);

                ABPlayerController->GetHUDWidget()->BindCharacterStat(CharacterStat);

                auto ABPlayerState = Cast<AABPlayerState>(PlayerState);
                ABCHECK(nullptr != ABPlayerState);
                CharacterStat->SetNewLevel(ABPlayerState->GetCharacterLevel());
            }
            SetActorHiddenInGame(true);
            HPBarWidget->SetHiddenInGame(true);
            bCanBeDamaged = false;
            break;
        case ECharacterState::READY:
            SetActorHiddenInGame(false);
            HPBarWidget->SetHiddenInGame(false);
            bCanBeDamaged = true;

            CharacterStat->OnHPIsZero.AddLambda([this]()->void
            {
                SetCharacterState(ECharacterState::DEAD);
            });

            {
                auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
                ABCHECK(nullptr != CharacterWidget);
                CharacterWidget->BindCharacterStat(CharacterStat);
            }

            if (bIsPlayer)
            {
                SetControlMode(EControlMode::DIABLO);
                GetCharacterMovement()->MaxWalkSpeed = 600.0f;
                EnableInput(ABPlayerController);
            }
            else
            {
                SetControlMode(EControlMode::NPC);
                GetCharacterMovement()->MaxWalkSpeed = 400.0f;
                ABAIController->RunAI();
            }

            break;
        case ECharacterState::DEAD:
            SetActorEnableCollision(false);
            GetMesh()->SetHiddenInGame(false);
            HPBarWidget->SetHiddenInGame(true);
            ABAnim->SetDeadAnim();
            bCanBeDamaged = false;

            if (bIsPlayer)
            {
                DisableInput(ABPlayerController);
            }
            else
            {
                ABAIController->StopAI();
            }

            GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda([this]() -> void
            {
                if (bIsPlayer)
                {
                    ABPlayerController->RestartLevel();
                }
                else
                {
                    Destroy();
                }
            }), DeadTimer, false);

            break;
    }
}

ECharacterState AABCharacter::GetChracterState() const
{
    return CurrentState;
}

int32 AABCharacter::GetExp() const
{
    return CharacterStat->GetDropExp();
}

// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();

    bIsPlayer = IsPlayerControlled();
    if (bIsPlayer)
    {
        ABPlayerController = Cast<AABPlayerController>(GetController());
        ABCHECK(nullptr != ABPlayerController);
    }
    else
    {
        ABAIController = Cast<AABAIController>(GetController());
        ABCHECK(nullptr != ABAIController);
    }

    auto DefaultSetting = GetDefault<UABCharacterSetting>();

    if (bIsPlayer)
    {
        AssetIndex = 4;
    }
    else
    {
        AssetIndex = FMath::RandRange(0, DefaultSetting->CharacterAssets.Num() - 1);
    }

    CharacterAssetToLoad = DefaultSetting->CharacterAssets[AssetIndex];
    auto ABGameInstance = Cast<UABGameInstance>(GetGameInstance());
    ABCHECK(nullptr != ABGameInstance);
    AssetStreamingHandle = ABGameInstance->StreambleManager.RequestAsyncLoad(CharacterAssetToLoad, FStreamableDelegate::CreateUObject(this, &AABCharacter::OnAssetLoadCompleted));
    SetCharacterState(ECharacterState::LOADING);
}

void AABCharacter::SetControlMode(EControlMode NewControlMode)
{
    CurrentControlMode = NewControlMode;

    switch (CurrentControlMode)
    {
    case AABCharacter::EControlMode::GTA:
        //SpringArm->TargetArmLength = 450.0f;
        //SpringArm->SetRelativeRotation(FRotator::ZeroRotator);
        ArmLengthTo = 450.0f;
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bInheritRoll = true;
        SpringArm->bInheritYaw = true;
        SpringArm->bDoCollisionTest = true;
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        break;
    case AABCharacter::EControlMode::DIABLO:
        //SpringArm->TargetArmLength = 800.0f;;
        //SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
        ArmLengthTo  = 800.0f;
        ArmRotationTo = FRotator(-45.0f, 0.0f, 0.0f);
        SpringArm->bUsePawnControlRotation = false;
        SpringArm->bInheritRoll = false;
        SpringArm->bInheritYaw = false;
        SpringArm->bDoCollisionTest = false;
        bUseControllerRotationYaw = true;
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        break;
    case AABCharacter::EControlMode::NPC:
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.0f, 0.0f);
        break;
    }
}

// Called every frame
void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, ArmLengthTo, DeltaTime, ArmLengthSpeed);

    switch (CurrentControlMode)
    {
    case AABCharacter::EControlMode::GTA:
        break;
    case AABCharacter::EControlMode::DIABLO:
        SpringArm->RelativeRotation = FMath::RInterpTo(SpringArm->RelativeRotation, ArmRotationTo, DeltaTime, ArmRotationSpeed);
        if (DirectionToMove.SizeSquared() > 0.0f)
        {
            GetController()->SetControlRotation(FRotationMatrix::MakeFromX(DirectionToMove).Rotator());
            AddMovementInput(DirectionToMove);
        }
        break;
    }
}

void AABCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
    ABCHECK(nullptr != ABAnim)

    ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

    ABAnim->OnNextAttackCheck.AddLambda([this]() -> void
    {
        ABLOG(Warning, TEXT("OnNextAttackCheck"));
        CanNextCombo = false;

        if (IsComboInputOn)
        {
            AttackStartComboState();
            ABAnim->JumpToAttackMontageSection(CurrentCombo);
        }
    });

    ABAnim->OnAttackHitCheck.AddUObject(this, &AABCharacter::AttackCheck);

    CharacterStat->OnHPIsZero.AddLambda([this]()->void
    {
        ABLOG(Warning, TEXT("OnHPIsZero"));
        ABAnim->SetDeadAnim();
        SetActorEnableCollision(false);
    });

    auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
    if (nullptr != CharacterWidget)
    {
        CharacterWidget->BindCharacterStat(CharacterStat);
    }
}

// Called to bind functionality to input
void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
    PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &AABCharacter::Attack);

    PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AABCharacter::UpDown);
    PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &AABCharacter::LeftRight);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);


}

float AABCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
    float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CharacterStat->SetDamage(FinalDamage);
    if (CurrentState == ECharacterState::DEAD)
    {
        if (EventInstigator->IsPlayerController())
        {
            auto ABPlayerController = Cast<AABPlayerController>(EventInstigator);
            ABCHECK(nullptr != ABPlayerController, 0.0f);
            ABPlayerController->NPCKill(this);
        }
    }

    return FinalDamage;
}

void AABCharacter::PossessedBy(AController * NewController)
{
    Super::PossessedBy(NewController);

    if (IsPlayerControlled())
    {
        SetControlMode(EControlMode::DIABLO);
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    }
    else
    {
        SetControlMode(EControlMode::NPC);
        GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    }
}

bool AABCharacter::CanSetWeapon()
{
    return (nullptr == CurrentWeapon);
}

void AABCharacter::SetWeapon(AABWeapon * NewWeapon)
{
    ABCHECK(nullptr != NewWeapon && nullptr == CurrentWeapon);
    FName WeaponSocket(TEXT("hand_rSocket"));
    if (nullptr != NewWeapon)
    {
        NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
        NewWeapon->SetOwner(this);
        CurrentWeapon = NewWeapon;
    }
}

void AABCharacter::UpDown(float NewAxisValue)
{
    switch (CurrentControlMode)
    {
    case AABCharacter::EControlMode::GTA:
        AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), NewAxisValue);
        break;
    case AABCharacter::EControlMode::DIABLO:
        DirectionToMove.X = NewAxisValue;
        break;
    }

}

void AABCharacter::LeftRight(float NewAxisValue)
{
    switch (CurrentControlMode)
    {
    case AABCharacter::EControlMode::GTA:
        AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), NewAxisValue);
        break;
    case AABCharacter::EControlMode::DIABLO:
        DirectionToMove.Y = NewAxisValue;
        break;
    default:
        break;
    }
}

void AABCharacter::LookUp(float NewAxisValue)
{
    switch (CurrentControlMode)
    {
    case AABCharacter::EControlMode::GTA:
        AddControllerPitchInput(NewAxisValue);
        break;
    default:
        break;
    }
}

void AABCharacter::Turn(float NewAxisValue)
{
    switch (CurrentControlMode)
    {
    case AABCharacter::EControlMode::GTA:
        AddControllerYawInput(NewAxisValue);
        break;
    default:
        break;
    }
}

void AABCharacter::ViewChange()
{
    switch (CurrentControlMode)
    {
    case AABCharacter::EControlMode::GTA:
        GetController()->SetControlRotation(GetActorRotation());
        SetControlMode(EControlMode::DIABLO);
        break;
    case AABCharacter::EControlMode::DIABLO:
        GetController()->SetControlRotation(SpringArm->RelativeRotation);
        SetControlMode(EControlMode::GTA);
        break;
    default:
        break;
    }
}

void AABCharacter::Attack()
{
    if (IsAttacking)
    {
        ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo));
        if (CanNextCombo)
        {
            IsComboInputOn = true;
        }
    }
    else
    {
        ABCHECK(CurrentCombo == 0);
        AttackStartComboState();
        ABAnim->PlayAttackMontage();
        ABAnim->JumpToAttackMontageSection(CurrentCombo);
        IsAttacking = true;
    }

}

void AABCharacter::OnAttackMontageEnded(UAnimMontage * Montage, bool bInterrupted)
{
    
    ABCHECK(IsAttacking);
    ABCHECK(CurrentCombo > 0);
    IsAttacking = false;
    AttackEndComboState();
    OnAttackEnd.Broadcast();
}

void AABCharacter::AttackStartComboState()
{
    CanNextCombo = true;
    IsComboInputOn = false;
    ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 0, MaxCombo - 1 ));
    CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1 , MaxCombo);
}

void AABCharacter::AttackEndComboState()
{
    IsComboInputOn = false;
    CanNextCombo = false;
    CurrentCombo = 0;
}

void AABCharacter::AttackCheck()
{
    FHitResult HitResult;
    FCollisionQueryParams Params(NAME_None, false, this);
    bool bResult = GetWorld()->SweepSingleByChannel(
        HitResult, 
        GetActorLocation(), 
        GetActorLocation() + GetActorForwardVector() * 200.0f, 
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(50.0f),
        Params);
#if ENABLE_DRAW_DEBUG
    
    FVector TraceVec = GetActorForwardVector() * AttackRange;
    FVector Center = GetActorLocation() + TraceVec * 0.5f;
    float HalfHeight = AttackRange * 0.5f + AttackRadius;
    FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
    FColor DrawColor = bResult ? FColor::Green : FColor::Red;
    float DebugLifeTime = 5.0f;

    DrawDebugCapsule(
        GetWorld(),
        Center,
        HalfHeight,
        AttackRadius,
        CapsuleRot,
        DrawColor,
        false,
        DebugLifeTime);

#endif

    if (bResult)
    {
        if (HitResult.Actor.IsValid())
        {
            FDamageEvent DamageEvent;
            HitResult.Actor->TakeDamage(CharacterStat->GetAttack(), DamageEvent, GetController(), this);
            ABLOG(Warning, TEXT("Hit Actor Name : %s"), *HitResult.Actor->GetName());
        }
    }
}

void AABCharacter::OnAssetLoadCompleted()
{
    AssetStreamingHandle->ReleaseHandle();
    TSoftObjectPtr<USkeletalMesh> LoadedAssetPath(CharacterAssetToLoad);
    ABCHECK(LoadedAssetPath.IsValid());

    GetMesh()->SetSkeletalMesh(LoadedAssetPath.Get());
    SetCharacterState(ECharacterState::READY);
}

