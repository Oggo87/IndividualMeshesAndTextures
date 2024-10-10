#include "GP4MemLib/GP4MemLib.h"
#include "IniLib/IniLib.h"
#include <iomanip>
#include <malloc.h>
#include <string>
#include <windows.h>

using namespace std;
using namespace GP4MemLib;
using namespace IniLib;

enum MESH_ID
{
	FRONT_WHEELS,
	REAR_WHEELS,
	HELMETS,
	COCKPITS,
	CARS
};

string meshNames[] = { "Front Wheels", "Rear Wheels", "Helmets", "Cockpits", "Cars" };
string meshIDs[] = { "FrontWheels", "RearWheels", "Helmets", "Cockpits", "Cars" };

//Target Addresses
DWORD individualMeshesAddress = 0x0064428C;
DWORD lodsPerMeshAddress = 0x00644294;
DWORD lodTableAddress = 0x006442A8;

DWORD genericMeshStartAddress = 0x00487100;
DWORD individualMeshStartAddress = 0x00487059;
DWORD defaultMeshStartAddress = 0x004870A3;
DWORD cockpitTextureSartAddress = 0x00486b76;
DWORD helmetTexture1StartAddress = 0x00486c07;
DWORD helmetTexture2StartAddress = 0x00486c48;

DWORD cockpitMirrorsSingleStartAddress = 0x00485e55;
DWORD cockpitMirrorsPerCarStartAddress = 0x00485f43;

//Jump Back Addresses
DWORD genericMeshJumpBackAddress = 0x0048710C;
DWORD individualMeshJumpBackAddress = 0x0048708A;
DWORD defaultMeshJumpBackAddress = 0x004870C7;
DWORD cockpitTextureJumpBackAddress = 0x00486b90;
DWORD helmetTexture1JumpBackAddress = 0x00486c20;
DWORD helmetTexture2JumpBackAddress = 0x00486c61;

DWORD cockpitMirrorsSingleJumpBackAddress = 0x00485e5b;
DWORD cockpitMirrorsPerCarJumpBackAddress = 0x00485f53;

//Vars and Data Addresses
DWORD trackIndex = 0x007AD894;
DWORD arrTeamNames = 0x643ba8;
DWORD arrLodsPerMeshType = 0x6442a8;
DWORD defaultTeam = 0x644edc;
DWORD cockpitTexture = 0x644d94;
DWORD helmetTexture1 = 0x644d84;
DWORD helmetTexture2 = 0x644d74;
DWORD carsFolderStr = 0x00644ee4;

//Function Addresses
DWORD ReplaceWildCards = 0x005DB088;
DWORD MeshFileExists = 0x0046af40;

DWORD LoadCarRelatedMesh = 0x00486f60;
DWORD UnkFunction1 = 0x00470d70;
DWORD UnkFunction2 = 0x0045e770;
DWORD UnkMaterialFunction = 0x00459290;

//Storage Variables
DWORD eaxVar, ecxVar, edxVar;
DWORD fileNameVar, fileNameTemplateVar, lodVar, trackIndexVar, driverNumberVar, driverIdVar, teamNameVar;
DWORD teamName;

DWORD meshNotExists = false;

char* extension = NULL;
char textureFileName[128];

void prepFileNameString(DWORD& fileNameStr)
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
		add EAX, 0x1 //make track index rage 1-17
		push EAX
	}
	__asm { //original instructions (adjusted for new var in Stack)
		lea EAX, [ESP + 0x28] //Original is + 0x24
		push EDX //LOD number
		push EAX //filename template
		push ECX //filename
		call ReplaceWildCards
	}

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //check if mesh exists
		push 0
		push dword ptr[ESP + 0x4] //filename
		call MeshFileExists
		mov meshNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back
	if (meshNotExists)
	{
		_asm { //save variables from stack
			pop fileNameVar //filename
			pop fileNameTemplateVar //filename template
			pop lodVar //LOD number
			pop trackIndexVar //track index
		}
		_asm { //new stack for fall-back name
			push 0x1 //default to track 1
			push lodVar //LOD number
			push fileNameTemplateVar //filename template
			push fileNameVar //filename
			call ReplaceWildCards
			mov fileNameVar, ESP //save filename
		}

		OutputDebugStringA("Reverting to default mesh [");

		OutputDebugStringA(*(char**)ULongToPtr(fileNameVar));

		OutputDebugStringA("]\n");
	}
	_asm { //pop modified stack
		pop fileNameVar //filename
		pop fileNameTemplateVar //filename template
		pop lodVar //LOD number
		pop trackIndexVar //track index
	}

	_asm { //restore stack as it should be
		push lodVar //LOD number
		push fileNameTemplateVar //filename template
		push fileNameVar //filename
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
		pop fileNameVar //filename
		pop fileNameTemplateVar //filename template
		pop teamNameVar //team name
		pop driverIdVar //driver 1 / driver 2
		pop lodVar //LOD number
		pop trackIndexVar //track index
	}

	_asm { //restore stack as it should be
		push lodVar //LOD number
		push driverIdVar //driver 1 / driver 2
		push teamNameVar //team name
		push fileNameTemplateVar //filename template
		push fileNameVar //filename
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
		pop fileNameVar //filename
		pop fileNameTemplateVar //filename template
		pop teamNameVar //team name
		pop driverIdVar //driver 1 / driver 2
		pop lodVar //LOD number
		pop trackIndexVar //track index
	}

	_asm { //restore stack as it should be
		push lodVar //LOD number
		push driverIdVar //driver 1 / driver 2
		push teamNameVar //team name
		push fileNameTemplateVar //filename template
		push fileNameVar //filename
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
		mov EAX, dword ptr[ESP + 0xc]
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

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //save filename from stack
		mov EAX, dword ptr[ESP]
		mov fileNameVar, EAX
	}

	//prep file name string for comparison
	prepFileNameString(fileNameVar);

	__asm { //check if file exists
		push 0
		push fileNameVar //filename
		call MeshFileExists
		mov meshNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back
	if (meshNotExists)
	{
		_asm { //save variables from stack
			pop fileNameVar //filename
			pop fileNameTemplateVar //filename template
			pop teamNameVar //team name
			pop driverIdVar //driver 1 / driver 2
			pop trackIndexVar //track index
		}
		_asm { //new stack for fall-back name
			push 0x1 //default to track 1
			push driverIdVar //driver 1 / driver 2
			push teamNameVar //team name
			push fileNameTemplateVar //filename template
			push fileNameVar //filename 
			call ReplaceWildCards
			mov fileNameVar, ESP //save filename
		}

		OutputDebugStringA("Reverting to default texture [");

		OutputDebugStringA(*(char**)ULongToPtr(fileNameVar));

		OutputDebugStringA("]\n");
	}
	_asm { //pop modified stack
		pop fileNameVar //filename
		pop fileNameTemplateVar //filename template
		pop teamNameVar //team name
		pop driverIdVar //driver 1 / driver 2
		pop trackIndexVar //track index
	}

	_asm { //restore stack as it should be
		push teamNameVar //team name
		push fileNameTemplateVar //filename template
		push fileNameVar //filename 
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

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //save filename from stack
		mov EAX, dword ptr[ESP]
		mov fileNameVar, EAX
	}

	//prep file name string for comparison
	prepFileNameString(fileNameVar);

	__asm { //check if file exists
		push 0
		push fileNameVar //filename
		call MeshFileExists
		mov meshNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back
	if (meshNotExists)
	{
		_asm { //save variables from stack
			pop fileNameVar //filename
			pop fileNameTemplateVar //filename template
			pop driverNumberVar //driver number
			pop trackIndexVar //track index
		}
		_asm { //new stack for fall-back name
			push 0x1 //default to track 1
			push driverNumberVar //driver number
			push fileNameTemplateVar //filename template
			push fileNameVar //filename
			call ReplaceWildCards
			mov fileNameVar, ESP //save filename
		}

		OutputDebugStringA("Reverting to default texture [");

		OutputDebugStringA(*(char**)ULongToPtr(fileNameVar));

		OutputDebugStringA("]\n");
	}
	_asm { //pop modified stack
		pop fileNameVar //filename
		pop fileNameTemplateVar //filename template
		pop driverNumberVar //driver number
		pop trackIndexVar //track index
	}

	_asm { //restore stack as it should be
		push driverNumberVar //driver number
		push fileNameTemplateVar //filename template
		push fileNameVar //filename
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

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //save filename from stack
		mov EAX, dword ptr[ESP]
		mov fileNameVar, EAX
	}

	//prep file name string for comparison
	prepFileNameString(fileNameVar);

	__asm { //check if file exists
		push 0
		push fileNameVar //filename
		call MeshFileExists
		mov meshNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back
	if (meshNotExists)
	{
		_asm { //save variables from stack
			pop fileNameVar //filename
			pop fileNameTemplateVar //filename template
			pop driverNumberVar //driver number
			pop trackIndexVar //track index
		}
		_asm { //new stack for fall-back name
			push 0x1 //default to track 1
			push driverNumberVar //driver number
			push fileNameTemplateVar //filename template
			push fileNameVar //filename
			call ReplaceWildCards
			mov fileNameVar, ESP //save filename
		}

		OutputDebugStringA("Reverting to default texture [");

		OutputDebugStringA(*(char**)ULongToPtr(fileNameVar));

		OutputDebugStringA("]\n");
	}
	_asm { //pop modified stack
		pop fileNameVar //filename
		pop fileNameTemplateVar //filename template
		pop driverNumberVar //driver number
		pop trackIndexVar //track index
	}

	_asm { //restore stack as it should be
		push driverNumberVar //driver number
		push fileNameTemplateVar //filename template
		push fileNameVar //filename
	}
	__asm jmp helmetTexture2JumpBackAddress //jump back into regular flow

}

__declspec(naked) void checkNullPointerSingleCockpitMirrors() {

	//if 0x00a4d67c is null, skip to 0x00485e90, else continue to normal flow
	__asm {
		mov ECX, dword ptr[0x00a4d67c]
		mov ECX, dword ptr[ECX]
		mov ecxVar, ECX
	}

	//original instructions
	__asm {
		mov ECX, dword ptr[0x00a4fcc8]
		mov ECX, dword ptr[ECX]
	}

	if (ecxVar == 0) {
		cockpitMirrorsSingleJumpBackAddress = 0x00485e90;
	}
	__asm jmp cockpitMirrorsSingleJumpBackAddress //jump back
}

__declspec(naked) void applyCockpitMirrorsPerCar() {

	__asm {
		call LoadCarRelatedMesh
	}

	//copy of original code for mirrors, adapted for individual cars
	__asm {
		mov ECX, dword ptr[0x00a4fcc8]
		mov ECX, dword ptr[ECX]
		call UnkFunction1	//return EAX 0x0D
		cmp EAX, -1
		jz skip //0x00485e90
		mov ECX, dword ptr[0x00a51f9c]
		mov ECX, dword ptr[ECX]	//valid dynamic address
		push EAX
		call UnkFunction2	//return EAX 0x0320
		cmp EAX, -1
		jz skip //0x00485e90
		//mov EDX, dword ptr[0x00a4d67c] //EDI?
		mov EDX, dword ptr[EDI]
		mov ECX, dword ptr[EDX + 0x58]
		cmp ECX, -1
		jz skip //0x00485e90
		push 0x1
		push EAX
		push ECX
		mov ECX, dword ptr[EDX + 0x4]
		call UnkMaterialFunction
	}

skip:

	__asm jmp cockpitMirrorsPerCarJumpBackAddress //jump back into regular flow
}

DWORD WINAPI MainThread(LPVOID param) {


	ostringstream messageBuilder;

	//Get the DLL path and use it to load the ini file
	char currentPath[MAX_PATH];
	HMODULE dllHandle = GetModuleHandleA("IndividualMeshesAndTextures.dll");
	GetModuleFileNameA(dllHandle, currentPath, MAX_PATH);
	size_t pos = string(currentPath).find_last_of("\\/");
	string iniFilePath = string(currentPath).substr(0, pos) + "\\IndividualMeshesAndTextures.ini";

	IniFile iniSettings;

	if (iniSettings.load(iniFilePath))
	{
		//Enable/Disable Individual Meshes for Front Wheels, Rear Wheels, Helmets and Cockpits
		bool individualMeshesEnabled[] = { false, false, false, false, true };

		for (int meshIndex = 0; meshIndex < 4; meshIndex++)
		{
			try
			{
				individualMeshesEnabled[meshIndex] = iniSettings["IndividualMeshes"][meshIDs[meshIndex]].getAs<bool>();
			}
			catch (exception ex) {}

			string enabled = individualMeshesEnabled[meshIndex] ? "Enabled" : "Disabled";

			MemUtils::patchAddress((LPVOID)(individualMeshesAddress + meshIndex), MemUtils::toBytes(!individualMeshesEnabled[meshIndex]), sizeof(bool));

			OutputDebugStringA(("Individual " + meshNames[meshIndex] + ": " + enabled + "\n").c_str());

		}

		//Patch LOD Table
		for (int meshIndex = 0; meshIndex < 5; meshIndex++)
		{
			//Check if LOD 0 Only is enabled
			bool lod0Only[] = { false, false, false, false, false };
			try
			{
				lod0Only[meshIndex] = iniSettings[meshIDs[meshIndex]]["LOD0Only"].getAs<bool>();
			}
			catch (exception ex) {}

			string enabled = lod0Only[meshIndex] ? "Enabled" : "Disabled";

			OutputDebugStringA(("LOD 0 Only " + meshNames[meshIndex] + ": " + enabled + "\n").c_str());

			vector<int> lodEntries;

			//If not LOD 0 Only, read LOD Table
			if (!lod0Only[meshIndex])
			{
				try
				{
					lodEntries = iniSettings["LODTable"][meshIDs[meshIndex]].getVectorAs<int>();
				}
				catch (exception ex) {}

				messageBuilder.str(std::string());

				messageBuilder << "LOD Table for " + meshNames[meshIndex] + ": ";

				for (unsigned int lodIndex = 0; lodIndex < lodEntries.size(); lodIndex++)
				{
					if (lodIndex > 0)
						messageBuilder << ", ";

					messageBuilder << lodEntries[lodIndex];
				}

				OutputDebugStringA(messageBuilder.str().c_str());
			}

			//Fill with 0s if vector is too short
			while (lodEntries.size() < 5)
			{
				lodEntries.push_back(0);
			}

			//Calculate LODs per Mesh
			int nLods = 1;
			for (; lodEntries[nLods] > 0 && nLods < 5; nLods++);

			messageBuilder.str(std::string());

			messageBuilder << "Number LODs for " << meshNames[meshIndex] << ": " << nLods;

			OutputDebugStringA(messageBuilder.str().c_str());

			//Patch Number of LODs per Mesh
			MemUtils::patchAddress((LPVOID)(lodsPerMeshAddress + meshIndex * sizeof(int)), MemUtils::toBytes(nLods), sizeof(int));

			//Patch LOD Table entries
			DWORD tableEntryAddress = lodTableAddress + (meshIndex * sizeof(int) * lodEntries.size());

			MemUtils::patchAddress((LPVOID)tableEntryAddress, MemUtils::toBytes(lodEntries[0]), sizeof(int) * lodEntries.size());

		}

	}
	else
	{
		OutputDebugStringA("Failed to open INI file");
	}

	//Re-route for cockpit mirrors 
	MemUtils::rerouteFunction(cockpitMirrorsSingleStartAddress, PtrToUlong(checkNullPointerSingleCockpitMirrors), VAR_NAME(checkNullPointerSingleCockpitMirrors));

	//Re-route for cockpit mirrors per car
	MemUtils::rerouteFunction(cockpitMirrorsPerCarStartAddress, PtrToUlong(applyCockpitMirrorsPerCar), VAR_NAME(applyCockpitMirrorsPerCar));

	//Re-route for generic meshes
	MemUtils::rerouteFunction(genericMeshStartAddress, PtrToUlong(genericMeshPerTrack), VAR_NAME(genericMeshPerTrack));

	//Re-route for individual meshes
	MemUtils::rerouteFunction(individualMeshStartAddress, PtrToUlong(individualMeshPerTrack), VAR_NAME(individualMeshPerTrack));

	//Re-route for default meshes
	MemUtils::rerouteFunction(defaultMeshStartAddress, PtrToUlong(defaultMeshPerTrack), VAR_NAME(defaultMeshPerTrack));

	//Allocate memory for new file name string
	char* cockpitTextureFileName = (char*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (cockpitTextureFileName)
	{
		//Set new cockpit texture file name per driver and per track
		strcpy_s(cockpitTextureFileName, 256, "cp_%s_car%d_track_%d.tga");
		cockpitTexture = PtrToUlong(cockpitTextureFileName);

		//Re-route for cockpit textures
		MemUtils::rerouteFunction(cockpitTextureSartAddress, PtrToUlong(cockpitTexturePerTrack), VAR_NAME(cockpitTexturePerTrack));
	}

	//Allocate memory for new file name string
	char* helmetTexture1FileName = (char*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (helmetTexture1FileName)
	{
		//Set new helmet texture 1 file name per track
		strcpy_s(helmetTexture1FileName, 256, "Driver%d_1_track_%d.tga");
		helmetTexture1 = PtrToUlong(helmetTexture1FileName);

		//Re-route for helmet texture 1
		MemUtils::rerouteFunction(helmetTexture1StartAddress, PtrToUlong(helmetTexture1PerTrack), VAR_NAME(helmetTexture1PerTrack));
	}

	//Allocate memory for new file name string
	char* helmetTexture2FileName = (char*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (helmetTexture2FileName)
	{
		//Set new helmet texture 2 file name per track
		strcpy_s(helmetTexture2FileName, 256, "Driver%d_2_track_%d.tga");
		helmetTexture2 = PtrToUlong(helmetTexture2FileName);

		//Re-route for helmet texture 2
		MemUtils::rerouteFunction(helmetTexture2StartAddress, PtrToUlong(helmetTexture2PerTrack), VAR_NAME(helmetTexture2PerTrack));
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