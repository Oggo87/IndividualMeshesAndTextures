#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <malloc.h>

#define VAR_NAME(var) (#var)

using namespace std;

string dwordToString(DWORD address) {

    ostringstream outputString;

    outputString.str(string());
    outputString << "0x" << hex << setw(sizeof(DWORD) * 2) << setfill('0') << address;

    return outputString.str();

}

string ptrToString(LPVOID address) {

    return dwordToString(PtrToUlong(address));
}

void PatchAddress(LPVOID address,LPBYTE patch, SIZE_T size) {
    DWORD oldProtect;

    string addressString = ptrToString(address);

    if (VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {

        memcpy(address, patch, size);
        VirtualProtect(address, size, oldProtect, &oldProtect);

        OutputDebugStringA(("Memory patched successfully at address " + addressString + "\n").c_str());
    }
    else {
        OutputDebugStringA(("Error while patching address " + addressString + "\n").c_str());
    }
}

DWORD genericMeshStartAddress = 0x00487100;
DWORD individualMeshStartAddress = 0x00487059;
DWORD defaultMeshStartAddress = 0x004870A3;
DWORD cockpitTextureSartAddress = 0x00486b76;
DWORD helmetTexture1StartAddress = 0x00486c07;
DWORD helmetTexture2StartAddress = 0x00486c48;

DWORD genericMeshJumpBackAddress = 0x0048710C;
DWORD individualMeshJumpBackAddress = 0x0048708A;
DWORD defaultMeshJumpBackAddress = 0x004870C7;
DWORD cockpitTextureJumpBackAddress = 0x00486b90;
DWORD helmetTexture1JumpBackAddress = 0x00486c20;
DWORD helmetTexture2JumpBackAddress = 0x00486c61;

DWORD trackIndex = 0x007AD894;
DWORD arrTeamNames = 0x643ba8;
DWORD arrLodsPerMeshType = 0x6442a8;
DWORD defaultTeam = 0x644edc;
DWORD cockpitTexture = 0x644d94;
DWORD helmetTexture1 = 0x644d84;
DWORD helmetTexture2 = 0x644d74;

DWORD carsFolderStr = 0x00644ee4;

DWORD ReplaceWildCards = 0x005DB088;
DWORD MeshFileExists = 0x0046af40;

DWORD lastLoadedFileIndex = 0x00a05c3c;

DWORD teamName;

DWORD var1, var2, var3, var4, var5, var6;

DWORD meshNotExists = false;

char* extension = NULL;
char textureFileName[128];

void prepFileNameString(DWORD &fileNameStr)
{
    //comparison has to include subfolders within cars.wad (i.e. cars\ in this case)
    strcpy_s(textureFileName, 128, (char*)carsFolderStr);
    strcat_s(textureFileName, 128, (char*)fileNameStr);

    //extension needs to be tex for the comparison to work
    extension = strstr(textureFileName, ".tga");
    if (extension != NULL)
    {
        extension[2] = 'e';
        extension[3] = 'x';
    }

    fileNameStr = PtrToUlong(textureFileName);
}

__declspec(naked) void genericMeshPerTrack() {
    
    __asm { //load trackIndex and push into stack
        mov EAX, trackIndex
        mov EAX, dword ptr[EAX]
        add EAX,0x1 //make track index rage 1-17
        push EAX
    }
    __asm { //original instructions (adjusted for new var in Stack)
        lea EAX, [ESP + 0x28] //Original is + 0x24
        push EDX //LOD number
        push EAX //filename template
        push ECX //filename
        call ReplaceWildCards
    }
    __asm { //check if mesh exists
        mov var1, EAX //save EAX register
        push 0
        push dword ptr[ESP+0x4] //filename
        call MeshFileExists
        mov meshNotExists, EAX //save comparison result
        mov EAX, var1 //restore EAX register
    }
    //fall-back
    if (meshNotExists)
    {
        _asm { //save variables from stack
            pop var1 //filename
            pop var2 //filename template
            pop var3 //LOD number
            pop var4 //track index
        }
        _asm { //new stack for fall-back name
            push 0x1 //default to track 1
            push var3 //LOD number
            push var2 //filename template
            push var1 //filename
            call ReplaceWildCards
            mov var1, ESP //save filename
        }

        OutputDebugStringA("Reverting to default mesh [");

        OutputDebugStringA(*(char**)ULongToPtr(var1));

        OutputDebugStringA("]\n");
    }
    _asm { //pop modified stack
        pop var1 //filename
        pop var2 //filename template
        pop var3 //LOD number
        pop var4 //track index
    }

    _asm { //restore stack as it should be
        push var3 //LOD number
        push var2 //filename template
        push var1 //filename
    }
    __asm jmp genericMeshJumpBackAddress //jump back into regular flow
}

__declspec(naked) void individualMeshPerTrack() {

    __asm { //load trackIndex and push into stack
        mov EAX, trackIndex
        mov EAX, dword ptr[EAX]
        add EAX, 0x1 //make track index rage 1-17
        push EAX
    }
    __asm { //original instructions (adjusted for new var in Stack and reordered to work with local variables)
        lea EAX, [EDX + EBP * 0x1]
        mov EDX, arrLodsPerMeshType
        lea ESI, [EAX * 0x4 + EDX]
        mov EAX, dword ptr[EAX * 0x4 + EDX]
        mov EDX, dword ptr[ESP + 0x478] //Original is + 0x474
        inc EDX
        add ECX, arrTeamNames
        mov teamName, ECX //save current team name string address
        push EAX //LOD number
        push EDX //driver 1 / driver 2
        push ECX //team name
        lea EAX, [ESP + 0x34] //Original is + 0x30
        lea ECX, [ESP + 0x74] //Original is + 0x70
        push EAX //filename template
        push ECX //filename
        call ReplaceWildCards
    }
    _asm { //pop modified stack
        pop var1 //filename
        pop var2 //filename template
        pop var3 //team name
        pop var4 //driver 1 / driver 2
        pop var5 //LOD number
        pop var6 //track index
    }

    _asm { //restore stack as it should be
        push var5 //LOD number
        push var4 //driver 1 / driver 2
        push var3 //team name
        push var2 //filename template
        push var1 //filename
    }
    __asm jmp individualMeshJumpBackAddress //jump back into regular flow
}

__declspec(naked) void defaultMeshPerTrack() {

    __asm { //default trackIndex to 1 and push into stack
        push 1
    }
    __asm { //original instructions (adjusted for new var in Stack)
        mov EAX, dword ptr[ESI]
        lea ECX, [ESP + 0x28] //Original is + 0x24
        push EAX //LOD number
        push 0x1 //use driver 1 for default
        push teamName //use current team name 
        lea EDX, [ESP + 0x74] //Original is + 0x70
        push ECX //filename template
        push EDX //filename
        call ReplaceWildCards
    }
    _asm { //pop modified stack
        pop var1 //filename
        pop var2 //filename template
        pop var3 //team name
        pop var4 //driver 1 / driver 2
        pop var5 //LOD number
        pop var6 //track index
    }

    _asm { //restore stack as it should be
        push var5 //LOD number
        push var4 //driver 1 / driver 2
        push var3 //team name
        push var2 //filename template
        push var1 //filename
    }
    __asm jmp defaultMeshJumpBackAddress //jump back into regular flow
}

__declspec(naked) void cockpitTexturePerTrack() {

    __asm { //load trackIndex and push into stack
        mov EAX, trackIndex
        mov EAX, dword ptr[EAX]
        add EAX, 0x1 //make track index rage 1-17
        push EAX
    }

    __asm { //load driver 1 / driver 2 and push into stack
        mov EAX, dword ptr[ESP+0xc]
        add EAX, 0x1
        push EAX
    }

    __asm { //original instructions (adjusted for new var in Stack and reordered to work with local variables)
        mov EAX, dword ptr[ESP + 0xd4] //Original is + 0xcc
        lea ECX, [ESP + 0xdc] //Original + 0xd4
        push EAX //team name
        mov EAX, cockpitTexture
        push EAX //filename template
        push ECX //filename
        call ReplaceWildCards
    }
    __asm { //save volatile registers and filename from stack
        mov var1, EAX
        mov var2, ECX
        mov var3, EDX
        mov EAX, dword ptr[ESP]
        mov var4, EAX
    }

    //prep file name string for comparison
    prepFileNameString(var4);

    __asm { //check if file exists
        push 0
        push var4 //filename
        call MeshFileExists
        mov meshNotExists, EAX //save comparison result
    }
    __asm { //restore volatile registers
        mov EAX, var1
        mov ECX, var2
        mov EDX, var3
    }
 
    //fall-back
    if (meshNotExists)
    {
        _asm { //save variables from stack
            pop var1 //filename
            pop var2 //filename template
            pop var3 //team name
            pop var4 //driver 1 / driver 2
            pop var5 //track index
        }
        _asm { //new stack for fall-back name
            push 0x1 //default to track 1
            push var4 //driver 1 / driver 2
            push var3 //team name
            push var2 //filename template
            push var1 //filename 
            call ReplaceWildCards
            mov var1, ESP //save filename
        }

        OutputDebugStringA("Reverting to default texture [");

        OutputDebugStringA(*(char**)ULongToPtr(var1));

        OutputDebugStringA("]\n");
    }
    _asm { //pop modified stack
        pop var1 //filename
        pop var2 //filename template
        pop var3 //team name
        pop var4 //driver 1 / driver 2
        pop var5 //track index
    }

    _asm { //restore stack as it should be
        push var3 //team name
        push var2 //filename template
        push var1 //filename 
    }
    __asm jmp cockpitTextureJumpBackAddress //jump back into regular flow

}

__declspec(naked) void helmetTexture1PerTrack() {

    __asm { //load trackIndex and push into stack
        mov EAX, trackIndex
        mov EAX, dword ptr[EAX]
        add EAX, 0x1 //make track index rage 1-17
        push EAX
    }

    __asm { //original instructions (adjusted for new var in Stack and reordered to work with local variables)
        mov ESI, dword ptr[ESP + 0x2ec] //Original is + 0x2e8
        and ESI, 0xff
        push ESI //driver number
        mov EAX, helmetTexture1
        push EAX //filename template
        push ECX //filename
        call ReplaceWildCards
    }
    __asm { //save volatile registers and filename from stack
        mov var1, EAX
        mov var2, ECX
        mov var3, EDX
        mov EAX, dword ptr[ESP]
        mov var4, EAX
    }

    //prep file name string for comparison
    prepFileNameString(var4);

    __asm { //check if file exists
        push 0
        push var4 //filename
        call MeshFileExists
        mov meshNotExists, EAX //save comparison result
    }
    __asm { //restore volatile registers
        mov EAX, var1
        mov ECX, var2
        mov EDX, var3
    }

    //fall-back
    if (meshNotExists)
    {
        _asm { //save variables from stack
            pop var1 //filename
            pop var2 //filename template
            pop var3 //driver number
            pop var4 //track index
        }
        _asm { //new stack for fall-back name
            push 0x1 //default to track 1
            push var3 //driver number
            push var2 //filename template
            push var1 //filename
            call ReplaceWildCards
            mov var1, ESP //save filename
        }

        OutputDebugStringA("Reverting to default texture [");

        OutputDebugStringA(*(char**)ULongToPtr(var1));

        OutputDebugStringA("]\n");
    }
    _asm { //pop modified stack
        pop var1 //filename
        pop var2 //filename template
        pop var3 //driver number
        pop var4 //track index
    }

    _asm { //restore stack as it should be
        push var3 //driver number
        push var2 //filename template
        push var1 //filename
    }
    __asm jmp helmetTexture1JumpBackAddress //jump back into regular flow

}

__declspec(naked) void helmetTexture2PerTrack() {

    __asm { //load trackIndex and push into stack
        mov ECX, trackIndex
        mov ECX, dword ptr[ECX]
        add ECX, 0x1 //make track index rage 1-17
        push ECX
    }

    __asm { //original instructions (adjusted for new var in Stack and reordered to work with local variables)
        push ESI //driver number
        mov ECX, helmetTexture2
        push ECX //filename template
        lea ECX, [ESP + 0xe0] //Original is + 0xd8
        push ECX //filename
        mov dword ptr[EBP + 0xa8], EAX
        call ReplaceWildCards
    }
    __asm { //save volatile registers and filename from stack
        mov var1, EAX
        mov var2, ECX
        mov var3, EDX
        mov EAX, dword ptr[ESP]
        mov var4, EAX
    }

    //prep file name string for comparison
    prepFileNameString(var4);

    __asm { //check if file exists
        push 0
        push var4 //filename
        call MeshFileExists
        mov meshNotExists, EAX //save comparison result
    }
    __asm { //restore volatile registers
        mov EAX, var1
        mov ECX, var2
        mov EDX, var3
    }

    //fall-back
    if (meshNotExists)
    {
        _asm { //save variables from stack
            pop var1 //filename
            pop var2 //filename template
            pop var3 //driver number
            pop var4 //track index
        }
        _asm { //new stack for fall-back name
            push 0x1 //default to track 1
            push var3 //driver number
            push var2 //filename template
            push var1 //filename
            call ReplaceWildCards
            mov var1, ESP //save filename
        }

        OutputDebugStringA("Reverting to default texture [");

        OutputDebugStringA(*(char**)ULongToPtr(var1));

        OutputDebugStringA("]\n");
    }
    _asm { //pop modified stack
        pop var1 //filename
        pop var2 //filename template
        pop var3 //driver number
        pop var4 //track index
    }

    _asm { //restore stack as it should be
        push var3 //driver number
        push var2 //filename template
        push var1 //filename
    }
    __asm jmp helmetTexture2JumpBackAddress //jump back into regular flow

}

void RerouteFunction(DWORD jumpToAddress, DWORD targetFunction, string functionName = "")
{
    BYTE jmpCode[5] = { 0xe9, 0x0, 0x0, 0x0, 0x0 };

    //Offset to jump into the re-routed function
    DWORD jumpOffset = targetFunction - jumpToAddress - 5;

    //append the jump offset to the jmp asm instruction code
    memcpy(&jmpCode[1], &jumpOffset, sizeof(DWORD));

    OutputDebugStringA(("Rerouting starting at address " + dwordToString(jumpToAddress) + "\n").c_str());

    if (functionName == "")
        functionName = "target function";

    OutputDebugStringA(("Address of " + functionName + ": " + dwordToString(targetFunction) + "\n").c_str());

    //Patch memory to jump
    PatchAddress((LPVOID)jumpToAddress, (BYTE*)&jmpCode, sizeof(jmpCode));

}

DWORD WINAPI MainThread(LPVOID param) {

    ostringstream outputString;
    BYTE patchCodeJmp = 0xe9;

    //Re-route for generic meshes
    RerouteFunction(genericMeshStartAddress, PtrToUlong(genericMeshPerTrack), VAR_NAME(genericMeshPerTrack));

    //Re-route for individual meshes
    RerouteFunction(individualMeshStartAddress, PtrToUlong(individualMeshPerTrack), VAR_NAME(individualMeshPerTrack));

    //Re-route for default meshes
    RerouteFunction(defaultMeshStartAddress, PtrToUlong(defaultMeshPerTrack), VAR_NAME(defaultMeshPerTrack));

    //Allocate memory for new file name string
    char* cockpitTextureFileName = (char*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (cockpitTextureFileName)
    {
        //Set new cockpit texture file name per driver and per track
        strcpy_s(cockpitTextureFileName, 256, "cp_%s_car%d_track_%d.tga");
        cockpitTexture = PtrToUlong(cockpitTextureFileName);

        //Re-route for cockpit textures
        RerouteFunction(cockpitTextureSartAddress, PtrToUlong(cockpitTexturePerTrack), VAR_NAME(cockpitTexturePerTrack));
    }

    //Allocate memory for new file name string
    char* helmetTexture1FileName = (char*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (helmetTexture1FileName)
    {
        //Set new helmet texture 1 file name per track
        strcpy_s(helmetTexture1FileName, 256, "Driver%d_1_track_%d.tga");
        helmetTexture1 = PtrToUlong(helmetTexture1FileName);

        //Re-route for helmet texture 1
        RerouteFunction(helmetTexture1StartAddress, PtrToUlong(helmetTexture1PerTrack), VAR_NAME(helmetTexture1PerTrack));
    }

    //Allocate memory for new file name string
    char* helmetTexture2FileName = (char*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (helmetTexture2FileName)
    {
        //Set new helmet texture 2 file name per track
        strcpy_s(helmetTexture2FileName, 256, "Driver%d_2_track_%d.tga");
        helmetTexture2 = PtrToUlong(helmetTexture2FileName);

        //Re-route for helmet texture 2
        RerouteFunction(helmetTexture2StartAddress, PtrToUlong(helmetTexture2PerTrack), VAR_NAME(helmetTexture2PerTrack));
    }
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}