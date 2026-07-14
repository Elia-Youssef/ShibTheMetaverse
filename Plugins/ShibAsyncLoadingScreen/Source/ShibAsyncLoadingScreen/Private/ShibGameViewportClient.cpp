// Copyright Shiba Inu Games LLC.

#include "ShibGameViewportClient.h"
#include "Engine/Canvas.h"

void UShibGameViewportClient::PostRender(UCanvas* Canvas)
{
	Super::PostRender(Canvas);
	
	// Fade if requested, you could use the same DrawScreenFade method from any canvas such as the HUD
	if (bFading)
	{
		DrawScreenFade(Canvas);
	}
}

void UShibGameViewportClient::ClearFade()
{
	bFading = false;	
}

void UShibGameViewportClient::Fade(const float WarmupDuration, const float Duration, const bool ToBlack)
{
	if (World)
	{
		bFading = true;
		bToBlack = ToBlack;
		FadeWarmupTime = WarmupDuration;
		FadeDuration = Duration;
		FadeStartTime = World->GetTimeSeconds();
	}	
}

void UShibGameViewportClient::DrawScreenFade(UCanvas* Canvas)
{
	if (World)
	{
		const float Time = World->GetTimeSeconds();
		float Alpha;
		float FadeColorAlpha = bToBlack ? 1.f : 0.f;
		if (FadeWarmupTime > 0.0f)
		{
			if (Time - FadeStartTime >= FadeWarmupTime)
			{
				if (FadeDuration > 0)
				{
					Alpha = FMath::Clamp((Time - (FadeStartTime + FadeWarmupTime)) / FadeDuration, 0.f, 1.f);
					FadeColorAlpha = bToBlack ? Alpha : 1 - Alpha;
				}
				
				// Make sure that we stay black in a fade to black
				if (!bToBlack && FadeColorAlpha == 0.f)
				{
					bFading = false;
				}
			}
			else
			{
				FadeColorAlpha = bToBlack ? FadeColorAlpha : 1 - FadeColorAlpha;
			}
		}
		else
		{
			if (FadeDuration > 0)
			{
				Alpha = FMath::Clamp((Time - FadeStartTime) / FadeDuration, 0.f, 1.f);
				FadeColorAlpha = bToBlack ? Alpha : 1 - Alpha;
			}
			
			// Make sure that we stay black in a fade to black
			if (!bToBlack && FadeColorAlpha == 0.f)
			{
				bFading = false;
			}
		}
		
		FColor OldColor = Canvas->DrawColor;
		FLinearColor FadeColor = FLinearColor::Black;
		FadeColor.A = FadeColorAlpha;
		Canvas->DrawColor = FadeColor.ToFColor(true);
		Canvas->DrawTile(Canvas->DefaultTexture, 0, 0, Canvas->ClipX, Canvas->ClipY, 0, 0, Canvas->DefaultTexture->GetSizeX(), Canvas->DefaultTexture->GetSizeY());
		Canvas->DrawColor = OldColor;
	}
}	