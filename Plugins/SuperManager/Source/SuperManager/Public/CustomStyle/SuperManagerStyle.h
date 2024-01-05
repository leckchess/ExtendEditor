// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Styling/SlateStyle.h"

class FSuperManagerStyle
{
public:
	static void InitIcons();
	static void ShutDown();

	static FName GetStyleSetName() { return StyleSetName; }

private:
	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();

private:
	static FName StyleSetName;
	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;
};
