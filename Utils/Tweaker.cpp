#include "Tweaker.h"

namespace GP4PP
{
    // Static member initialization
    Tweaker* Tweaker::tweakerInstance = nullptr;

    __declspec (naked) void LoadTweakerHook()
    {
        // Call the Hook method of Tweaker
        __asm call Tweaker::LoadTweakerFunc;

        // Update the enabled state
        Tweaker::getInstance()->initEnabled();

		// Override tweaker brake light tweak
		Tweaker::getInstance()->brakeLightTweakOverride();

        // Jump back to original code flow
        __asm jmp Tweaker::JumpBackAddress
    }
}