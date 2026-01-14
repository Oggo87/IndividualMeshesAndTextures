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
        static void Hook()
        {
            if (tweakerInstance)
                return;

            GP4MemLib::MemUtils::rerouteFunction(HookAddress, PtrToUlong(LoadTweakerHook), VAR_NAME(LoadTweakerHook));

            tweakerInstance = new Tweaker();
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

        // Disable destructor
        ~Tweaker() = default;

        // Disable copy constructor and assignment operator
        Tweaker(const Tweaker&) = delete;
        Tweaker& operator=(const Tweaker&) = delete;

        // Member variables
        bool enabled;
        IniLib::IniFile tweakerIni;

        // Static instance pointer
        static Tweaker* tweakerInstance;

        // Addresses
        static const DWORD LoadTweakerFunc = 0x005372DE;
        static const DWORD HookAddress = 0x0054BB57;
        static const DWORD JumpBackAddress = 0x0054BB5C;
    };
}
