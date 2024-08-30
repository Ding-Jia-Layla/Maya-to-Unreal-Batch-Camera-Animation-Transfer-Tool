// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FAnimDataFromJSONModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	int32 ExtractNumberFromFilename(const FString& Filename);
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	bool loadAnimationData(FAnimDataTrack& OutAnimTrack);
	void CreateLevelSequenceAsset();
	ACameraActor* FindOrCreateCamera(UWorld* World, const FString& CameraName);
	FString SelectDirectory();
	TArray<FString> getAllFiles();
	FString GetEachJson(TArray<FString>& allFiles, const int32 index);
	int32 SetTotalFrames();

private:

	void RegisterMenus();

	TSharedPtr<class FUICommandList> PluginCommands;
	FString SelectedDirectory;
};
