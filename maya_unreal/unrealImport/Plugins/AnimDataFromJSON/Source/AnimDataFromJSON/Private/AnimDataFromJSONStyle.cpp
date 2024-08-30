// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimDataFromJSONStyle.h"
#include "AnimDataFromJSON.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FAnimDataFromJSONStyle::StyleInstance = nullptr;

void FAnimDataFromJSONStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAnimDataFromJSONStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAnimDataFromJSONStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AnimDataFromJSONStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FAnimDataFromJSONStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("AnimDataFromJSONStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("AnimDataFromJSON")->GetBaseDir() / TEXT("Resources"));

	Style->Set("AnimDataFromJSON.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FAnimDataFromJSONStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FAnimDataFromJSONStyle::Get()
{
	return *StyleInstance;
}
