// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/ShibFunctionLibrary.h"
#include "Game/ShibDeveloperSettings.h"
#include "Kismet/KismetMathLibrary.h"

FDateTime UShibFunctionLibrary::GetLocalTimeFromUTC(FDateTime UTC)
{
	const FDateTime UtcNow = FDateTime::UtcNow();
	const FDateTime Now = FDateTime::Now();
	const FTimespan Diff = UKismetMathLibrary::Subtract_DateTimeDateTime(Now, UtcNow);

	return UKismetMathLibrary::Add_DateTimeTimespan(UTC, Diff);
}

bool UShibFunctionLibrary::IsPointInArea(FVector2D Point, FVector2D From, FVector2D To)
{
	const bool bPointBetweenX = From.X < To.X
		                            ? Point.X >= From.X && Point.X <= To.X
		                            : Point.X <= From.X && Point.X >= To.X;
	const bool bPointBetweenY = From.Y < To.Y
		                            ? Point.Y >= From.Y && Point.Y <= To.Y
		                            : Point.Y <= From.Y && Point.Y >= To.Y;

	return bPointBetweenX && bPointBetweenY;
}

void UShibFunctionLibrary::SetCommandLineToken(const FString Token)
{
    FString FullCommandLine = FCommandLine::Get();
    FString TokenArg = FString::Printf(TEXT("-token=%s"), *Token);

    const FString Pattern = FString("-token=[^ ]*");
    const FRegexPattern RegexPattern(Pattern);
    FRegexMatcher Matcher(RegexPattern, FullCommandLine);

    if (Matcher.FindNext())
    {
        FullCommandLine = FullCommandLine.Left(Matcher.GetMatchBeginning()) +
                          TokenArg +
                          FullCommandLine.Right(FullCommandLine.Len() - Matcher.GetMatchEnding());
    }
    else
    {
        FullCommandLine += TEXT(" ") + TokenArg;
    }

    FCommandLine::Set(*FullCommandLine);
}

TArray<FRichTextToken> UShibFunctionLibrary::ExtractHyperLinks(const FString& Text)
{
	TArray<FRichTextToken> Out;

	int32 i = 0;
	bool bFoundLink = false;
	FRichTextToken Temp;
	while (i < Text.Len())
	{
		if (Text[i] == '[')
		{
			bFoundLink = true; // it's a link
			if (!Temp.Text.IsEmpty())
			{
				Out.Add(Temp); // add previous text as a string
				Temp = FRichTextToken(); // empty the token
			}
			i++; // skip the bracket
			continue;
		}
		if (Text[i] == ']' && bFoundLink)
		{
			i++; // skip the bracket
			if (i >= Text.Len()) break;

			if (Text[i] != '(') // if it's NOT a link
			{
				bFoundLink = false; // it's not a link
				Out.Add(Temp); // add it as a string
				Temp = FRichTextToken(); // empty the token
				continue;
			}

			i++; // skip the '('
			if (i >= Text.Len()) break;

			while (i < Text.Len())
			{
				if (Text[i] == ')') // if the link ended
				{
					Out.Add(Temp); // add it as a link
					Temp = FRichTextToken(); // empty the token
					i++; // skip the ')'
					break; // break & continue
				}

				Temp.Link += Text[i++];
			}

			continue;
		}

		Temp.Text += Text[i++];
	}

	if (!Temp.Text.IsEmpty())
	{
		Out.Add(Temp);
	}

	return Out;
}

bool UShibFunctionLibrary::RunningInPIE()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

void UShibFunctionLibrary::UpdateLevelEditorPlaySettings(const FString& DevToken, const FString& DevWallet)
{
	const auto* ShibDevSettings = UShibDeveloperSettings::Get();
	
	if (!GConfig || !ShibDevSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("GConfig or ShibDeveloperSettings is null, cannot apply level editor play settings."));
		return;
	}

	// Define the section name where the settings are located
	const FString ConfigSection = TEXT("/Script/UnrealEd.LevelEditorPlaySettings");

	FString FullAdditionalLaunchParams = ShibDevSettings->AdditionalLaunchParameters;
	if (!DevToken.IsEmpty() && !DevWallet.IsEmpty())
	{
		FullAdditionalLaunchParams += " -token="+DevToken+" -user_data="+DevWallet;
	}
	
	GConfig->SetBool(*ConfigSection, TEXT("bLaunchSeparateServer"), true, GEditorPerProjectIni);
	GConfig->SetString(*ConfigSection, TEXT("PlayNetMode"), TEXT("PIE_Standalone"), GEditorPerProjectIni);
	GConfig->SetBool(*ConfigSection, TEXT("RunUnderOneProcess"), true, GEditorPerProjectIni);
	GConfig->SetString(*ConfigSection, TEXT("AdditionalLaunchParameters"), *FullAdditionalLaunchParams, GEditorPerProjectIni);
	GConfig->SetString(*ConfigSection, TEXT("ServerMapNameOverride"), *ShibDevSettings->ServerMapNameOverride, GEditorPerProjectIni);
	GConfig->SetString(*ConfigSection, TEXT("AdditionalServerGameOptions"), *ShibDevSettings->AdditionalServerGameOption, GEditorPerProjectIni);
	
	// Force the configuration to flush and save
	GConfig->Flush(true, GEditorPerProjectIni);

	UE_LOG(LogTemp, Warning, TEXT("Level Editor Play Settings updated"));
}




bool UShibFunctionLibrary::IsClientOrStandalone()
{
	if (!GEngine) return false;

	TIndirectArray<FWorldContext> Contexts = GEngine->GetWorldContexts();
	if (Contexts.IsEmpty()) return false;

	const UWorld* World = Contexts[0].World();
	if (!IsValid(World)) return false;
	
	return World->GetNetMode() == NM_Standalone || World->GetNetMode() == NM_Client;
}

bool UShibFunctionLibrary::IsPixelStreaming()
{
	FString Url;
	return FParse::Value(FCommandLine::Get(), *FString("PixelStreamingURL="), Url) && !Url.IsEmpty();
}
