// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimDataFromJSON.h"
#include "AnimDataFromJSONStyle.h"
#include "AnimDataFromJSONCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "ToolMenus.h"
#include "Editor.h"
#include "Misc/FileHelper.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Misc/Paths.h"
#include "FileHelpers.h"
#include "Misc/MessageDialog.h"
#include "EngineUtils.h"
#include "HAL/FileManagerGeneric.h"
#include "MovieScene.h"
#include "LevelSequence.h"
#include <LevelSequencePlayer.h>
#include "LevelSequenceActor.h"
#include "MovieScene/Public/Channels/MovieSceneChannelProxy.h"
#include "Sequencer/Public/ISequencerModule.h"
#include "LevelEditorViewport.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <Tracks/MovieScene3DTransformTrack.h>
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Tracks/MovieSceneTransformTrack.h"
#include <animDataStruct.h>
#include "Sections/MovieSceneCameraCutSection.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/IAssetRegistry.h"  
#include "Math/Rotator.h"
#include "Modules/ModuleManager.h"
#include "Framework/Application/SlateApplication.h"  
#include "Serialization/BulkData.h"  
#include "Editor/EditorEngine.h"  

static const FName AnimDataFromJSONTabName("AnimDataFromJSON");

#define LOCTEXT_NAMESPACE "FAnimDataFromJSONModule"

void FAnimDataFromJSONModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FAnimDataFromJSONStyle::Initialize();
	FAnimDataFromJSONStyle::ReloadTextures();

	FAnimDataFromJSONCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FAnimDataFromJSONCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FAnimDataFromJSONModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAnimDataFromJSONModule::RegisterMenus));
}

void FAnimDataFromJSONModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAnimDataFromJSONStyle::Shutdown();

	FAnimDataFromJSONCommands::Unregister();
}

int32 FAnimDataFromJSONModule::ExtractNumberFromFilename(const FString& Filename)
{
	FString BaseName = FPaths::GetBaseFilename(Filename);
	BaseName = BaseName.Replace(TEXT("frame_"), TEXT(""));
	return FCString::Atoi(*BaseName);
}

void FAnimDataFromJSONModule::PluginButtonClicked()
{
	SetTotalFrames();
	CreateLevelSequenceAsset();
}
/// <summary>
/// Read the JSON file and reconstruct the data.
/// </summary>
/// <param name="OutAnimTrack">Stores the reconstructed animation data structure. </param>
/// <returns></returns>
bool FAnimDataFromJSONModule::loadAnimationData(FAnimDataTrack& OutAnimTrack)
{
	TArray<FString> OutFiles = getAllFiles();
	int32 totalFrame = OutFiles.Num();
	TMap<FName, FCamTrack> camMap;


	for (int FrameIndex = 0; FrameIndex < totalFrame; ++FrameIndex)
	{
		FString JsonFilePath = GetEachJson(OutFiles, FrameIndex);
		FString JsonString;
		if (FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				for (const auto& Pair : JsonObject->Values)
				{
					FName camName = FName(*Pair.Key);
					const TSharedPtr<FJsonObject> camDataObject = Pair.Value->AsObject();
					FVector Position = FVector(
						camDataObject->GetNumberField("tx"),
						camDataObject->GetNumberField("ty") * (-1),
						camDataObject->GetNumberField("tz")
					);
					double MayaRotX = camDataObject->GetNumberField("rx");
					double MayaRotY = camDataObject->GetNumberField("ry");
					double MayaRotZ = camDataObject->GetNumberField("rz");
					FRotator Rotation = FRotator(MayaRotY, MayaRotZ, MayaRotX);

					//FQuat Rotation = Rotator.Quaternion();
					FVector Scale = FVector(1, 1, 1);

					if (!camMap.Contains(camName))
					{
						FCamTrack tempcam;
						tempcam.Key = camName;
						tempcam.Value.PosKeys.Add(Position);
						tempcam.Value.RotKeys.Add(Rotation);
						tempcam.Value.ScaleKeys.Add(Scale);
						camMap.Add(camName, tempcam);
					}
					else
					{
						camMap[camName].Value.PosKeys.Add(Position);
						camMap[camName].Value.RotKeys.Add(Rotation);
						camMap[camName].Value.ScaleKeys.Add(Scale);
					}
				}

			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("can't read file"), );
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("can't find file"), );
			return false;
		}
	}
	for (auto& cam : camMap)
	{
		OutAnimTrack.Tracks.Add(cam.Value);
		UE_LOG(LogTemp, Warning, TEXT("OutAnimTrack's track key: %s"), *cam.Value.Key.ToString());
		int32 NumCamTracks = camMap.Num();
		UE_LOG(LogTemp, Warning, TEXT("Number of cameras in camMap: %d"), NumCamTracks);

	}

	return true;
}
/// <summary>
/// Use the read data to create level sequence assets and add keyframe data to the channel.
/// </summary>
void FAnimDataFromJSONModule::CreateLevelSequenceAsset()
{
	FAnimDataTrack Anim;
	loadAnimationData(Anim);

	// just for highlight
	UE_LOG(LogTemp, Error, TEXT("camera count: %d"), Anim.Tracks.Num());
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UObject* NewAsset = nullptr;
	TArray<AActor*>cameraActorList;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* CurrentClass = *It;
		if (CurrentClass->IsChildOf(UFactory::StaticClass()) && !(CurrentClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UFactory* Factory = Cast<UFactory>(CurrentClass->GetDefaultObject());
			if (Factory->CanCreateNew() && Factory->ImportPriority >= 0 && Factory->SupportedClass == ULevelSequence::StaticClass())
			{
				NewAsset = AssetTools.CreateAssetWithDialog(ULevelSequence::StaticClass(), Factory);
				break;
			}
		}
	}
	if (!NewAsset)
	{
		return;
	}

	if (NewAsset && NewAsset->IsA<ULevelSequence>())
	{

		ULevelSequence* levelsequence = static_cast<ULevelSequence*>(NewAsset);
		// get the track of camera
		UMovieSceneCameraCutTrack* CameraCutTrack = Cast<UMovieSceneCameraCutTrack>(levelsequence->MovieScene->GetCameraCutTrack());
		CameraCutTrack = Cast<UMovieSceneCameraCutTrack>(levelsequence->MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass()));

		for (auto& a : Anim.Tracks)
		{
			UWorld* World = GWorld;
			AActor* cameraActor = FindOrCreateCamera(World, a.Key.ToString());
			cameraActorList.Add(cameraActor);
		}
		int32 StartFrame = 0;
		int32 totalFrame = SetTotalFrames();
		FFrameNumber EndFrameNumber(totalFrame - 1);

		int32 FrameRate = 30;  
		int32 TicksPerSecond = levelsequence->MovieScene->GetTickResolution().AsDecimal(); 
		int32 FrameTickValue = TicksPerSecond / FrameRate;  

		FFrameNumber StartFrameNumber = FFrameNumber(StartFrame * FrameTickValue);

	
		UE_LOG(LogTemp, Error, TEXT("camera count: %d"), Anim.Tracks.Num());
		for (int32 i = 0; i < Anim.Tracks.Num(); i++)
		{
			AActor* CameraActor = cameraActorList.IsValidIndex(i) ? cameraActorList[i] : nullptr;
			// cameraActorList NUM: 70 
			UE_LOG(LogTemp, Warning, TEXT("cameraActorList NUM in Tracks: %d "), cameraActorList.Num());
			//UE_LOG(LogTemp, Warning, TEXT("totalFrame: %d "), totalFrame);
			if (cameraActorList[i])
			{
				//get the binding of camera
				FGuid Guid = levelsequence->FindBindingFromObject(cameraActorList[i], cameraActorList[i]->GetWorld());
				Guid = Cast<UMovieSceneSequence>(levelsequence)->CreatePossessable(cameraActorList[i]);
				//camera cut 
				UMovieSceneCameraCutSection* Section = CameraCutTrack->AddNewCameraCut(UE::MovieScene::FRelativeObjectBindingID(Guid), FFrameNumber(StartFrame * FrameTickValue));

				Section->SetEndFrame(EndFrameNumber);
				Section->SetIsLocked(true);

				UMovieScene3DTransformTrack* TransformTrack = levelsequence->GetMovieScene()->AddTrack<UMovieScene3DTransformTrack>(Guid);

				UMovieScene3DTransformSection* TransformSection = Cast<UMovieScene3DTransformSection>(TransformTrack->CreateNewSection());
				TransformTrack->AddSection(*TransformSection);
				TransformSection->SetRange(TRange<FFrameNumber>(FFrameNumber(0), FFrameNumber(EndFrameNumber)));

				TArrayView<FMovieSceneDoubleChannel*> DoubleChannels = TransformSection->GetChannelProxy().GetChannels<FMovieSceneDoubleChannel>();

				int32 totalFrames = SetTotalFrames();
				for (int32 FrameIndex = 0; FrameIndex < totalFrames; ++FrameIndex)
				{

					//Time is right, but the frame is wrong, maybe 3*

					FFrameNumber FrameNumber = FFrameNumber(FrameIndex * FrameTickValue);
					UE_LOG(LogTemp, Warning, TEXT("%d"), FrameTickValue);
					DoubleChannels[0]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.PosKeys[FrameIndex].X);
					DoubleChannels[1]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.PosKeys[FrameIndex].Y);
					DoubleChannels[2]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.PosKeys[FrameIndex].Z);
					//the order in channel is different from the axis of the target
					DoubleChannels[3]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.RotKeys[FrameIndex].Roll);
					DoubleChannels[4]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.RotKeys[FrameIndex].Pitch);
					DoubleChannels[5]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.RotKeys[FrameIndex].Yaw);

					DoubleChannels[6]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.ScaleKeys[FrameIndex].X);
					DoubleChannels[7]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.ScaleKeys[FrameIndex].Y);
					DoubleChannels[8]->AddCubicKey(FrameNumber, Anim.Tracks[i].Value.ScaleKeys[FrameIndex].Z);
				}

				levelsequence->Modify();
				levelsequence->MarkPackageDirty();

				levelsequence->PostEditChange();
				TransformSection->MarkAsChanged();

			}
		}

		// Spawn an actor at the origin, and either move infront of the camera or focus camera on it (depending on the viewport) and open for edit

		UActorFactory* ActorFactory = GEditor->FindActorFactoryForActorClass(ALevelSequenceActor::StaticClass());
		if (!ensure(ActorFactory))
		{
			return;
		}

		AActor* Actor = GEditor->UseActorFactory(ActorFactory, FAssetData(NewAsset), &FTransform::Identity);
		if (Actor == nullptr)
		{
			return;
		}
		ALevelSequenceActor* NewActor = CastChecked<ALevelSequenceActor>(Actor);
		if (GCurrentLevelEditingViewportClient != nullptr && GCurrentLevelEditingViewportClient->IsPerspective())
		{
			GEditor->MoveActorInFrontOfCamera(*NewActor, GCurrentLevelEditingViewportClient->GetViewLocation(), GCurrentLevelEditingViewportClient->GetViewRotation().Vector());
		}
		else
		{
			GEditor->MoveViewportCamerasToActor(*NewActor, false);
		}
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewAsset);
		//GEditor->SyncBrowserToObjects(NewAsset);


	}
}
/// <summary>
/// Get the corresponding camera if it already exists, otherwise create it.
/// </summary>
/// <param name="World">Target (here is the unique identifier of the camera)</param>
/// <param name="CameraName">The name of camera which come from the key name</param>
/// <returns></returns>
ACameraActor* FAnimDataFromJSONModule::FindOrCreateCamera(UWorld* World, const FString& CameraName)
{
	for (TActorIterator<ACameraActor> It(World); It; ++It) {
		if (It->GetActorLabel() == CameraName) {
			return *It;
		}
	}
	FActorSpawnParameters SpawnParams;
	ACameraActor* NewCineCameraActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	// Only create a new camera if none was found
	if (NewCineCameraActor)
	{
		NewCineCameraActor->SetActorLabel(CameraName);
		//TODO:zh
		NewCineCameraActor->SetActorLocationAndRotation(FVector(0, 0, 0), FRotator(0, 0, 0));

		if (!NewCineCameraActor->GetCameraComponent()) {
			UE_LOG(LogTemp, Warning, TEXT("HERE ISN'T CAMERA component"));
			UCameraComponent* CameraComponent = NewObject<UCameraComponent>(NewCineCameraActor);
			CameraComponent->SetupAttachment(NewCineCameraActor->GetRootComponent());
			NewCineCameraActor->AddOwnedComponent(CameraComponent);
			CameraComponent->RegisterComponent();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE IS CAMERA component"));
		}
	}

	return NewCineCameraActor;
}
/// <summary>
/// Use IDesktopPlatform related API to open the external resource reader, 
/// note that FindBestParentWindowHandleForDialogs will sometimes use the recent directory directly, so you need to clear the cache in time!
/// It's very fragile, so I also offer one version with several extra layers of detection to prevent null pointers.
///</summary>
/// <returns></returns>
// FString FAnimDataFromJSONModule::SelectDirectory()
// {
// 	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
// 	if (DesktopPlatform)
// 	{
// 		void* ParentWindowHandle = const_cast<void*>(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr));
// 		const FText Title = FText::FromString(TEXT("Choose Directory"));
// 		const FString DefaultPath = FPaths::ProjectContentDir();
// 		bool bOpened = DesktopPlatform->OpenDirectoryDialog(ParentWindowHandle, Title.ToString(), DefaultPath, SelectedDirectory);
// 		if (bOpened)
// 		{
// 			UE_LOG(LogTemp, Log, TEXT("Selected Directory: %s"), *SelectedDirectory);
// 			return SelectedDirectory;
// 		}
// 		else
// 		{
// 			UE_LOG(LogTemp, Error, TEXT("Failed to open directory dialog or no directory was selected."));
// 			return FString();
// 		}
// 	}
// 	else
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("DesktopPlatform is not available."));
// 		return FString();

// 		if (SelectedDirectory.IsEmpty())
// 		{
// 			UE_LOG(LogTemp, Error, TEXT("Selected directory is empty."));
// 		}

// 		return SelectedDirectory;
// 	}
// }
FString FrotPluginTestModule::SelectDirectory()
{

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		void* ParentWindowHandle = const_cast<void*>(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr));
		const FText Title = FText::FromString(TEXT("Choose Directory"));
		const FString DefaultPath = FPaths::ProjectContentDir();
		bool bOpened = DesktopPlatform->OpenDirectoryDialog(ParentWindowHandle, Title.ToString(), DefaultPath, SelectedDirectory);
		if (bOpened)
		{
			UE_LOG(LogTemp, Log, TEXT("Selected Directory: %s"), *SelectedDirectory);
		}
		
	}

	return SelectedDirectory;
}

TArray<FString> FAnimDataFromJSONModule::getAllFiles()
{
	TArray<FString> OutFiles;
	FString Directory;

	if (SelectedDirectory.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No directory selected. Opening directory dialog..."));
		if (SelectDirectory().IsEmpty())
		{
			// Use default directory if no directory is selected
			Directory = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("animData/default"));
		}
		else
		{
			// If a directory was selected, use it
			Directory = SelectedDirectory;
		}
	}
	else
	{
		Directory = SelectedDirectory;
	}

	// Confirm the directory exists
	if (!FPaths::DirectoryExists(Directory))
	{
		UE_LOG(LogTemp, Error, TEXT("The specified directory does not exist: %s"), *Directory);
		return OutFiles; 
	}

	FString Wildcard = TEXT("*.json");
	FString SearchPath = FPaths::Combine(Directory, Wildcard);
	UE_LOG(LogTemp, Log, TEXT("Search Path: %s"), *SearchPath);

	FFileManagerGeneric FileMgr;
	FileMgr.FindFiles(OutFiles, *SearchPath, true, false);

	UE_LOG(LogTemp, Log, TEXT("Found %d files"), OutFiles.Num());
	
	OutFiles.Sort([this](const FString& A, const FString& B) {
		return ExtractNumberFromFilename(A) < ExtractNumberFromFilename(B);
		});

	return OutFiles;
}

FString FAnimDataFromJSONModule::GetEachJson(TArray<FString>& allFiles, const int32 index)
{
	int32 totalFrame = allFiles.Num();

	if (SelectedDirectory.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No directory selected"));
		return FString();
	}

	FString f = allFiles[index];
	FString JsonFilePath = FPaths::Combine(SelectedDirectory, f);
	UE_LOG(LogTemp, Warning, TEXT("JSON file path: %s"), *JsonFilePath);
	return JsonFilePath;
}

int32 FAnimDataFromJSONModule::SetTotalFrames()
{
	TArray<FString> files = getAllFiles(); 
	int32 totalFrame = files.Num(); 
	return totalFrame;
}

void FAnimDataFromJSONModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.Cinematics");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("LevelEditorNewCinematics");
			Section.AddMenuEntryWithCommandList(FAnimDataFromJSONCommands::Get().PluginAction, PluginCommands);

		}
	}


}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAnimDataFromJSONModule, AnimDataFromJSON)