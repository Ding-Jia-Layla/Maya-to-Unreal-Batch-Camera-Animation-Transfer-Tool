// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AnimDataFromJSONStyle.h"

class FAnimDataFromJSONCommands : public TCommands<FAnimDataFromJSONCommands>
{
public:

	FAnimDataFromJSONCommands()
		: TCommands<FAnimDataFromJSONCommands>(TEXT("AnimDataFromJSON"), NSLOCTEXT("Contexts", "AnimDataFromJSON", "AnimDataFromJSON Plugin"), NAME_None, FAnimDataFromJSONStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};

