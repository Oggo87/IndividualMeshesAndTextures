#include "Tweaker.h"

// Static member initialization
Tweaker* Tweaker::tweakerInstance = nullptr;

__declspec (naked) void LoadTweakerHook()
{
    // Call the Hook method of Tweaker
    __asm call LoadTweakerFunc;

    // Update the enabled state
    Tweaker::getInstance()->initEnabled();

    // Jump back to original code flow
    __asm jmp TweakerJumpBackAddress
}