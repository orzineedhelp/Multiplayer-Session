// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSdemo_tcCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FPSdemo_tc.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include <Online/OnlineSessionNames.h>



AFPSdemo_tcCharacter::AFPSdemo_tcCharacter()//:
	////初始化委托成员变量
	//CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AFPSdemo_tcCharacter::OnCreateSessionComplete)) ,
	//FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &AFPSdemo_tcCharacter::OnFindSessionsComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}
	
}

void AFPSdemo_tcCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFPSdemo_tcCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFPSdemo_tcCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSdemo_tcCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSdemo_tcCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFPSdemo_tcCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogFPSdemo_tc, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

///// <summary>
///// 打开大厅将其设置为监听服务器
///// </summary>
//void AFPSdemo_tcCharacter::OpenLobby()
//{
//	UWorld* World = GetWorld();
//	if (World)
//	{
//		World->ServerTravel("/Game/FirstPerson/Lobby?listen"); //？listen ：以监听服务器模式打开
//	}
//}

///// <summary>
///// 客户端连接函数，用于让客户端连接到指定的服务器地址（可以是IP、域名或关卡名称）
///// </summary>
///// <param name="Address">指定的IP、域名或关卡名称</param>
//void AFPSdemo_tcCharacter::CallOpenLevel(const FString& Address)
//{
//	UGameplayStatics::OpenLevel(this, *Address);
//}

///// <summary>
///// 客户端连接
///// </summary>
///// <param name="Address"></param>
//void AFPSdemo_tcCharacter::CallClientTravel(const FString& Address)
//{
//	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
//	if (PlayerController) 
//	{
//		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
//	}
//}

///// <summary>
///// 创建会话
///// </summary>
//void AFPSdemo_tcCharacter::CreateGameSession()
//{
//	//启用时进行会话创建
//	if (!OnlineSessionInterface.IsValid())
//	{
//		return;
//	}
//	//检查是否有已存在的会话
//	auto ExistSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
//	if (ExistSession != nullptr) 
//	{
//		OnlineSessionInterface->DestroySession(NAME_GameSession);
//	}
//
//	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);//绑定回调函数
//
//	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
//	SessionSettings->bIsLANMatch = false;
//	SessionSettings->NumPublicConnections = 4;
//	SessionSettings->bAllowJoinInProgress = true;
//	SessionSettings->bAllowJoinViaPresence = true;
//	SessionSettings->bShouldAdvertise = true;
//	SessionSettings->bUsesPresence = true;
//	SessionSettings->bUseLobbiesIfAvailable = true;
//
//	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
//	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
//
//}
//
///// <summary>
///// 加入会话
///// </summary>
//void AFPSdemo_tcCharacter::JoinGameSession()
//{
//	//寻找会话	
//	if (!OnlineSessionInterface.IsValid())
//	{
//		return;
//	}
//
//	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
//
//	SessionSerch = MakeShareable(new FOnlineSessionSearch());
//	SessionSerch->MaxSearchResults = 10000;
//	SessionSerch->bIsLanQuery = false;
//	SessionSerch->QuerySettings.Set(SEARCH_PRESENCE,true,EOnlineComparisonOp::Equals);
//	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
//	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSerch.ToSharedRef());
//}
//
///// <summary>
///// 创建会话后的回调函数
///// </summary>
///// <param name="SessionName"></param>
///// <param name="bWasSuccessful"></param>
//void AFPSdemo_tcCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
//{
//	if (bWasSuccessful)
//	{
//		if (GEngine)
//		{
//			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Created session:%s"),*SessionName.ToString()));
//		}
//	}
//	else
//	{
//		if(GEngine)
//		{
//			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("Failed to create session")));
//		}
//	}
//}
//
///// <summary>
///// 找寻会话后的回调函数
///// </summary>
///// <param name="SessionName"></param>
///// <param name="bWasSuccessful"></param>
//void AFPSdemo_tcCharacter::OnFindSessionsComplete(bool bWasSuccessful)
//{
//	for (auto Result : SessionSerch->SearchResults)
//	{
//		FString Id = Result.GetSessionIdStr();
//		FString User = Result.Session.OwningUserName;
//		if (GEngine)
//		{
//			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, FString::Printf(TEXT("ID:%s,User:%s"), *Id,*User));
//		}
//	}
//
//
//}


void AFPSdemo_tcCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AFPSdemo_tcCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AFPSdemo_tcCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AFPSdemo_tcCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AFPSdemo_tcCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AFPSdemo_tcCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
