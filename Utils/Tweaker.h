#pragma once
#include "Helpers.h"
#include "../IniLib/IniLib.h"
#include "../GP4MemLib/GP4MemLib.h"
#include <windows.h>

namespace GP4PP
{
    void LoadTweakerHook();

    class Tweaker
    {
    public:
        // Friend function to allow hook access
        friend void LoadTweakerHook();

        // Accessor — does NOT allocate
        static Tweaker* getInstance()
        {
            return tweakerInstance;
        }

        // Hook responsible for instancing + injection logic
        static void Hook(IniLib::IniFile iniSettings)
        {
            if (tweakerInstance)
                return;

            GP4MemLib::MemUtils::rerouteFunction(HookAddress, PtrToUlong(LoadTweakerHook), VAR_NAME(LoadTweakerHook));

            tweakerInstance = new Tweaker();

			// Load Override Brake Light Tweak setting
            try
            {
                tweakerInstance->overrideBrakeLightTweak = iniSettings["Tweaker"]["OverrideBrakeLight"].getAs<bool>();
            }
            catch (exception ex) {}

            OutputGP4PPDebugString("Override Brake Light Tweak: " + string(tweakerInstance->overrideBrakeLightTweak ? "Enabled" : "Disabled"));
        }

        // enabled property (default: false)
        bool isEnabled() const
        {
            return enabled;
        }

        // Get the state of a specific tweak from the INI file
        bool getTweakState(const std::string& key) const
        {
            try
            {
                return tweakerIni["SYSTEM_TWEAKS_ENABLE"][key].getAs<bool>();
            }
            catch (...)
            {
                return false;
            }
        }

    private:
        Tweaker()
            : enabled(false), tweakerIni()
        {
        }

        void initEnabled()
        {
            HMODULE dllTweakHandle = GetModuleHandleA("GP4Tweak.dll");
            enabled = (dllTweakHandle != NULL);

            OutputGP4PPDebugString("GP4 Tweaker: " + string(enabled ? "Detected" : "Not Detected"));
            if (enabled)
            {
                char currentPath[MAX_PATH];
                GetModuleFileNameA(dllTweakHandle, currentPath, MAX_PATH);
                size_t pos = string(currentPath).find_last_of("\\/");
                string basePath = string(currentPath).substr(0, pos) + "\\";
                string iniFilePath = basePath + "gp4tweak.ini";

                if (tweakerIni.load(iniFilePath))
                {
                    OutputGP4PPDebugString("GP4 Tweaker INI file loaded from: " + iniFilePath);
                }
            }
        }

        void brakeLightTweakOverride() const
        {
            if (!enabled)
                return;

            if (overrideBrakeLightTweak && getTweakState("RearLightBrake"))
            {

                // Restore original GP4 op-code, overriding the tweaker patch
				DWORD tweakerBrakePatchAddress1 = 0x00487a81;
				BYTE tweakerBrakePatch1[] = { 0x8a, 0x85, 0xd0, 0x49, 0x00, 0x00, 0x84, 0xc0, 0x74, 0x22, 0x8a, 0x85, 0xd4, 0x13, 0x00, 0x00 };
				GP4MemLib::MemUtils::patchAddress((LPVOID)tweakerBrakePatchAddress1, tweakerBrakePatch1, sizeof(tweakerBrakePatch1));

                DWORD tweakerBrakePatchAddress2 = 0x0048ccba;
                BYTE tweakerBrakePatch2[] = { 0x8a, 0x86, 0xd0, 0x49, 0x00, 0x00, 0x84, 0xc0, 0x0f, 0x84, 0x64, 0x04, 0x00, 0x00 };
                GP4MemLib::MemUtils::patchAddress((LPVOID)tweakerBrakePatchAddress2, tweakerBrakePatch2, sizeof(tweakerBrakePatch2));

            }
		}

        // Disable destructor
        ~Tweaker() = default;

        // Disable copy constructor and assignment operator
        Tweaker(const Tweaker&) = delete;
        Tweaker& operator=(const Tweaker&) = delete;

        // Member variables
        bool enabled = false;
		bool overrideBrakeLightTweak = false;
        IniLib::IniFile tweakerIni;

        // Static instance pointer
        static Tweaker* tweakerInstance;

        // Addresses
        static const DWORD LoadTweakerFunc = 0x005372DE;
        static const DWORD HookAddress = 0x0054BB57;
        static const DWORD JumpBackAddress = 0x0054BB5C;
    };
}
