#include "RearLight.h"
#include "../Utils/Helpers.h"
#include "../Utils/CarDynamicData.h"
#include "../Utils/PitLimiter.h"
#include "../GP4MemLib/GP4MemLib.h"
#include<array>

using namespace GP4MemLib;
using namespace GP4PP;
using namespace std;
using namespace chrono;

namespace RearLight
{
	//General Settings
	bool wetWeatherLight = true;
	bool brakeLight = false;
	bool chargeERSLight = false;

	short brakeThreshold = 5; //%
	bool pitLimiterLight = true;

	//Target Addresses
	DWORD rearLightFunctionAddress = 0x005108f4;

	//Jump Back Addresses
	DWORD rearLightFunctionJumpBackAddress = 0x005108f9;

	//Function Addresses
	DWORD isTrackWetFunc = 0x00510778;

	//Parameters
	unsigned short maxBrakeValue = 0x4000;
	unsigned short minBrakeValue = 0x333;

	//Data Variables
	unsigned int trackWet = 0;
	unsigned int rearLightOn = 0;

	unsigned int carIndex = 0;

	DWORD carDynDataAddress = 0;

	CarDynamicData *carDynData;

	/*
	F1 rear lights flash at different rates (Hz) to signal conditions:
	4Hz (fast) in the wet for visibility;
	2Hz (slow) for energy harvesting (ERS recovery) or Pit Limiter use; and
	2Hz for 10 secs after Safety Car/VSC;
	*/

	bool RearLight::wetWeatherBlinking = false;
	int RearLight::wetWeatherPeriodMs = 250;

	bool RearLight::pitLimiterBlinking = true;
	int RearLight::pitLimiterPeriodMs = 500;

	bool RearLight::chargeERSBlinking = true;
	int RearLight::chargeERSPeriodMs = 500;

	//static Random Number Generator for random time offsets
	std::mt19937 Blinker::rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));

	array<RearLight, 22> rearLights;

	int calcRearLightState()
	{
		carDynData = MemUtils::addressToPtr<CarDynamicData>(carDynDataAddress);

		if (brakeLight && (carDynData->brake > minBrakeValue))
		{
			rearLights[carIndex].setState(BRAKE);
		}
		else if(pitLimiterLight && PitLimiter::updateSatus(carDynData))
		{
			rearLights[carIndex].setState(PIT_LIMITER);
		}
		// charging when speed > 1ft/s (~1km/h) and either throttle not pressed or brake pressed
		else if (chargeERSLight && (carDynData->speed > 0x400000) && ((carDynData->throttle == 0) || (carDynData->brake > 0))) 
		{
			rearLights[carIndex].setState(CHARGE_ERS);
		}
		else if (wetWeatherLight && trackWet)
		{
			rearLights[carIndex].setState(WET);
		}
		else
		{
			rearLights[carIndex].setState(OFF);
		}

		return rearLights[carIndex].update();
	}

	__declspec(naked) void rearLightFunction()
	{
		RegUtils::saveVolatileRegisters();

		__asm mov carIndex, EAX

		__asm mov carDynDataAddress, ESI

		__asm call isTrackWetFunc

		__asm mov trackWet, EAX

		rearLightOn = calcRearLightState();

		RegUtils::restoreVolatileRegisters();

		__asm mov EAX, rearLightOn

		__asm jmp rearLightFunctionJumpBackAddress //jump back into regular flow
	}

	void LoadSettings(IniFile iniSettings)
	{
		//Utility string builder
		std::ostringstream messageBuilder;

		//Wet weather rear light
		try
		{
			wetWeatherLight = iniSettings["RearLight"]["WetWeather"].getAs<bool>();
		}
		catch (exception ex) {}

		OutputGP4PPDebugString("Rear Light - Wet Weather : " + string(wetWeatherLight ? "Enabled" : "Disabled"));

		if (wetWeatherLight)
		{
			//Wet weather rear light blinking
			try
			{
				RearLight::wetWeatherBlinking = iniSettings["RearLight"]["WetWeatherBlinking"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputGP4PPDebugString("Rear Light - Wet Weather Blinking : " + string(RearLight::wetWeatherBlinking ? "Enabled" : "Disabled"));

			if (RearLight::wetWeatherBlinking)
			{
				try
				{
					RearLight::wetWeatherPeriodMs = iniSettings["RearLight"]["WetWeatherPeriod"].getAs<int>();
				}
				catch (exception ex) {}

				OutputGP4PPDebugString("Rear Light - Wet Weather Period : " + to_string(RearLight::wetWeatherPeriodMs) + " ms");
			}
		}

		//Brake rear light
		try
		{
			brakeLight = iniSettings["RearLight"]["Brake"].getAs<bool>();
		}
		catch (exception ex) {}

		OutputGP4PPDebugString("Rear Light - Brake : " + string(brakeLight ? "Enabled" : "Disabled"));

		if (brakeLight)
		{
			try
			{
				brakeThreshold = iniSettings["RearLight"]["BrakeThreshold"].getAs<short>();
			}
			catch (exception ex) {}

			messageBuilder.str(string());

			messageBuilder << "Rear Light - Brake Threshold : " << brakeThreshold << " %";

			if (brakeThreshold > 100)
			{
				messageBuilder << " (Clamped to 100 %)";
				brakeThreshold = 100;
			}
			else if (brakeThreshold < 0)
			{
				messageBuilder << " (Clamped to 0 %)";
				brakeThreshold = 0;
			}

			minBrakeValue = static_cast<unsigned short>((brakeThreshold / 100.0f) * maxBrakeValue);

			OutputGP4PPDebugString(messageBuilder.str());
		}

		// Pit Limiter rear light
		try
		{
			pitLimiterLight = iniSettings["RearLight"]["PitLimiter"].getAs<bool>();
		}
		catch (exception ex) {}

		OutputGP4PPDebugString("Rear Light - Pit Limiter : " + string(pitLimiterLight ? "Enabled" : "Disabled"));

		if (pitLimiterLight)
		{
			//Pit Limiter rear light blinking
			try
			{
				RearLight::pitLimiterBlinking = iniSettings["RearLight"]["PitLimiterBlinking"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputGP4PPDebugString("Rear Light - Pit Limiter Blinking : " + string(RearLight::pitLimiterBlinking ? "Enabled" : "Disabled"));

			if (RearLight::pitLimiterBlinking)
			{
				try
				{
					RearLight::pitLimiterPeriodMs = iniSettings["RearLight"]["PitLimiterPeriod"].getAs<int>();
				}
				catch (exception ex) {}

				OutputGP4PPDebugString("Rear Light - Pit Limiter Period : " + to_string(RearLight::pitLimiterPeriodMs) + " ms");
			}
		}

		// Charge ERS rear light
		try 
		{
			chargeERSLight = iniSettings["RearLight"]["ChargeERS"].getAs<bool>();
		}
		catch (exception ex) {}

		OutputGP4PPDebugString("Rear Light - Charge ERS : " + string(chargeERSLight ? "Enabled" : "Disabled"));

		if (chargeERSLight)
		{
			//Charge ERS rear light blinking
			try
			{
				RearLight::chargeERSBlinking = iniSettings["RearLight"]["ChargeERSBlinking"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputGP4PPDebugString("Rear Light - Charge ERS Blinking : " + string(RearLight::chargeERSBlinking ? "Enabled" : "Disabled"));

			if (RearLight::chargeERSBlinking)
			{
				try
				{
					RearLight::chargeERSPeriodMs = iniSettings["RearLight"]["ChargeERSPeriod"].getAs<int>();
				}
				catch (exception ex) {}

				OutputGP4PPDebugString("Rear Light - Charge ERS Period : " + to_string(RearLight::chargeERSPeriodMs) + " ms");
			}
		}
	}



	void ApplyPatches()
	{
		MemUtils::rerouteFunction(rearLightFunctionAddress, PtrToUlong(rearLightFunction), VAR_NAME(rearLightFunction));
	}
}