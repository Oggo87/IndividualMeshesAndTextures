#pragma once
#include "../IniLib/IniLib.h"

using namespace IniLib;

namespace D3D
{

	// Defaulting D3DDEVTYPE to int since we only need it for the function signature and not for any specific values in this patch, and including the full enum would be unnecessary
    typedef int D3DDEVTYPE;

	// Partial implementation of D3DFORMAT enum, only including z-buffer formats since that's all we need for this patch. The full enum contains many more values for various texture and surface formats, but they are not relevant to our current patch and including them would be unnecessary.
    enum D3DFORMAT
    {
        D3DFMT_D32 = 71,
        D3DFMT_D15S1 = 73,
        D3DFMT_D24S8 = 75,
        D3DFMT_D24X8 = 77,
        D3DFMT_D24X4S4 = 79,
        D3DFMT_D16 = 80,

    };

	void LoadSettings(IniFile iniSettings);

	void ApplyPatches();
};