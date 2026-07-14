#include "MiniGames/Fishing/FishingBobber.h"

AFishingBobber::AFishingBobber()
{
	
}

void AFishingBobber::SetMaxWaitingTime(float NewTime)
{
	OnCountdownReady(NewTime);
}
