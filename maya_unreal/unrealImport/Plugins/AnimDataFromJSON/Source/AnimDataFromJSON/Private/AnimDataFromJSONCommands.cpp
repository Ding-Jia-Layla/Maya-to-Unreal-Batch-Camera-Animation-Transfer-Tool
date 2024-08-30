// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimDataFromJSONCommands.h"

#define LOCTEXT_NAMESPACE "FAnimDataFromJSONModule"

void FAnimDataFromJSONCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "AnimDataFromJSON", "Execute AnimDataFromJSON action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE




