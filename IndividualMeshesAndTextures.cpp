#include "GP4MemLib/GP4MemLib.h"
#include "IniLib/IniLib.h"
#include <iomanip>
#include <malloc.h>
#include <map>
#include <string>
#include <windows.h>

using namespace std;
using namespace GP4MemLib;
using namespace IniLib;

struct CGP4MeshShaderData {
	DWORD* shader;
	DWORD params[8];
};

enum ASSET_ID
{
	FRONT_WHEELS,
	REAR_WHEELS,
	HELMETS,
	COCKPITS,
	CARS,
	COLLISION_MESH,
	HELMET_1,
	HELMET_2,
	COCKPIT_TEX
};

bool trackFolders = false;
bool cockpitVisor = false;

//Enable/Disable Individual Meshes for Front Wheels, Rear Wheels, Helmets and Cockpits
bool individualMeshesEnabled[] = { false, false, false, false, true, true };
bool lod0Only[] = { false, false, false, false, false };
bool perTeam[] = { false, false, false, false, false, false, false, false, true };
bool perDriver[] = { false, false, false, false, true, false, true, true, false };
bool perTrack[] = { false, false, false, false, false, false, false, false, false };

vector<int> defaultTracks = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17 };
vector<int> tracks[9];

string assetNames[] = { "Front Wheels", "Rear Wheels", "Helmets", "Cockpits", "Cars", "Collision Mesh", "Helmet 2", "Helmet 1", "Cockpit" };
string assetIDs[] = { "FrontWheels", "RearWheels", "Helmets", "Cockpits", "Cars", "CollisionMesh", "HelmetTextures", "HelmetTextures", "CockpitTextures" };
string defaultFileNames[] = { "CAR_Wheel_Front_LOD_{lod}", "CAR_Wheel_Rear_LOD_{lod}", "CAR_Helmet_0{lod}", "CAR_Cockpit_0{lod}", "CAR_{teamname}_CAR{car}_LOD_{lod}", "car_collision_mesh.ct", "Driver{driver}_1.tex", "Driver{driver}_2.tex", "cp_{teamname}.tex" };
string filePartialNames[] = { "Wheel_Front", "Wheel_Rear", "Helmet", "Cockpit", "Car", "collision_mesh", "Driver", "Driver", "cp"};

//Array of mesh and textures filenames
string fileNames[8];

//map to store the variables for subsitution
map<string, string> variables;

//Target Addresses
DWORD individualMeshesAddress = 0x0064428C;
DWORD lodsPerMeshAddress = 0x00644294;
DWORD lodTableAddress = 0x006442A8;
DWORD meshFileNamesTable = 0x00644390;

DWORD genericMeshStartAddress = 0x00487107;
DWORD individualMeshStartAddress = 0x00487085;
DWORD cockpitTextureSartAddress = 0x00486b8b;
DWORD helmetTexture1StartAddress = 0x00486c1b;
DWORD helmetTexture2StartAddress = 0x00486c5c;
DWORD collisionMeshStartAddress = 0x00485feb;
DWORD cockpitVisorStartAddress = 0x004858f9;
DWORD cockpitVisorStartAddress2 = 0x00487a61;

DWORD cockpitMirrorsSingleStartAddress = 0x00485e55;
DWORD cockpitMirrorsPerCarStartAddress = 0x00485f43;

//Jump Back Addresses
DWORD genericMeshJumpBackAddress = 0x0048710C;
DWORD individualMeshJumpBackAddress = 0x0048708A;
DWORD cockpitTextureJumpBackAddress = 0x00486b90;
DWORD helmetTexture1JumpBackAddress = 0x00486c20;
DWORD helmetTexture2JumpBackAddress = 0x00486c61;
DWORD collisionMeshJumpBackAddress = 0x00485ff0;
DWORD cockpitVisorJumpBackAddress = 0x004858fe;
DWORD cockpitVisorJumpBackAddress2 = 0x00488a0b;

DWORD cockpitMirrorsSingleJumpBackAddress = 0x00485e5b;
DWORD cockpitMirrorsPerCarJumpBackAddress = 0x00485f53;

//Vars and Data Addresses
DWORD trackIndex = 0x007AD894;
DWORD carsFolder = 0x00644EE4;
DWORD gp4Extension = 0x0063EE08;

DWORD arrTeamNames = 0x643ba8;
DWORD arrLodsPerMeshType = 0x6442a8;
DWORD defaultTeam = 0x644edc;
DWORD cockpitTexture = 0x644d94;
DWORD helmetTexture1 = 0x644d84;
DWORD helmetTexture2 = 0x644d74;

DWORD cockpitMesh = 0x00a4d5ec;

//Function Addresses
DWORD ReplaceWildCards = 0x005DB088;
DWORD AssetFileExists = 0x0046af40;

DWORD GetObjectIndexByName = 0x00457200;
DWORD GetShader = 0x0045ee70;
DWORD ApplyShaderToObject = 0x00457260;
DWORD SetCockpitMirrorShaderParameters = 0x00485080;

DWORD LoadCarRelatedMesh = 0x00486f60;
DWORD UnkFunction1 = 0x00470d70;
DWORD UnkFunction2 = 0x0045e770;
DWORD UnkMaterialFunction = 0x00459290;
DWORD meth_0x457660 = 0x00457660;
DWORD meth_0x459600 = 0x00459600;

//Storage Variables
DWORD eaxVar, ecxVar, edxVar, espVar;
DWORD fileNameVar;
int assetIndex;
int track;

DWORD CGP4Car = NULL;

string collisionMesh = "";

DWORD fileNotExists = false;

char textureFileName[128] = "";

//Cockpit Glass Object Variables
string visorObjectName = "Visor";
int visorObjectIndex = -1;
std::vector<short> visorBytes = { 128, 128, 128, 0 };
DWORD visorColour = 0x808080;
float transparencyMultiplier = 0.5;

string replaceVariables(const string& input, const map<string, string>& replacements) {
	string result = input;
	size_t startPos = 0;

	// Loop through the string to find occurrences of variables
	while ((startPos = result.find('{', startPos)) != string::npos) {
		size_t endPos = result.find('}', startPos);
		if (endPos == string::npos) {
			// No closing brace found, stop processing
			break;
		}

		// Extract the variable name (between the curly braces)
		string varName = result.substr(startPos + 1, endPos - startPos - 1);

		// Check if the variable is in the replacements map
		if (replacements.find(varName) != replacements.end()) {
			// Replace the entire {varName} with the corresponding value
			result.replace(startPos, endPos - startPos + 1, replacements.at(varName));

			// Move past the current replaced text to search for the next variable
			endPos = startPos + replacements.at(varName).length();
		}
		else
		{
			// Move past the current variable to search for the next one
			startPos = endPos + 1;
		}
	}

	return result;
}

void prepFileNameString(DWORD& fileNameStr)
{
	//comparison has to include subfolders within cars.wad (i.e. cars\ in this case)
	string carsFolderString = MemUtils::addressToPtr<char>(carsFolder);
	string textureFileNameString = MemUtils::addressToPtr<char>(fileNameStr);
	string tgaExtension = ".tga";
	string texExtension = ".tex";

	size_t extensionPos = textureFileNameString.find(tgaExtension);

	if (extensionPos != std::string::npos)
	{
		textureFileNameString.replace(extensionPos, tgaExtension.length(), texExtension);
	}


	strcpy_s(textureFileName, (carsFolderString + textureFileNameString).c_str());

	fileNameStr = PtrToUlong(textureFileName);
}

void calcFileName(bool useDefaultFileName = false)
{
	//Get Cars folder and .gp4 extension to compose file path
	string carsFolderString = MemUtils::addressToPtr<char>(carsFolder);
	string gp4ExtensionString = MemUtils::addressToPtr<char>(gp4Extension);

	//Compose file path, including variables
	string path = "";
	if (assetIndex <= ASSET_ID::COLLISION_MESH)
	{
		path += carsFolderString;
	}

	path += replaceVariables(useDefaultFileName ? defaultFileNames[assetIndex] : fileNames[assetIndex], variables);

	if (assetIndex <= ASSET_ID::CARS)
	{
		path += gp4ExtensionString;
	}

	if(useDefaultFileName)
	{
		OutputDebugStringA("Reverting to default GP4 file name [");
	}

	OutputDebugStringA(path.c_str());

	if (useDefaultFileName)
	{
		OutputDebugStringA("]");
	}

	if (assetIndex == ASSET_ID::COLLISION_MESH)
	{
		//save path to collision mesh variable
		collisionMesh = path;
	}
	else
	{
		//The first value in the stack is the string that needs to be replaced
		char* stackPtr = MemUtils::addressToValue<char*>(espVar);

		memcpy(stackPtr, path.c_str(), path.size() + 1);
	}

}

void initMeshIndex(bool individual = false)
{
	//0x00000478
	//0x00000480 for individual meshes

	int meshIndexStack = espVar + 0x478;

	if (individual)
	{
		meshIndexStack += 0x08;
	}

	assetIndex = MemUtils::addressToValue<int>(meshIndexStack);
}

void initTrackIndex(bool fallback = false)
{
	if (fallback)
	{
		track = tracks[assetIndex][MemUtils::addressToValue<int>(trackIndex)];
	}
	else
	{
		track = MemUtils::addressToValue<int>(trackIndex) + 1;
	}
}

void initGenericMeshVariables()
{
	//clear map and save all necessary variables/values
	variables.clear();
	variables["track"] = to_string(track);
	variables["lod"] = to_string(MemUtils::addressToValue<int>(espVar + 0x08));
}

__declspec(naked) void genericMeshFunc()
{
	//dummy call to ReplaceWildCards to ensure registries and stack are properly set
	__asm call ReplaceWildCards

	//save stack pointer
	__asm mov espVar, ESP

	//set current mesh index
	initMeshIndex();

	//set current track index
	initTrackIndex();

	//set variables
	initGenericMeshVariables();

	//calculate new file name
	calcFileName();

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //check if file exists
		push 0
		push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
		call AssetFileExists
		mov fileNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to specified track index
	if (fileNotExists)
	{
		//set fall-back track index
		initTrackIndex(true);

		//set variables
		initGenericMeshVariables();

		OutputDebugStringA("Reverting to default track [");

		//calculate new file name
		calcFileName();

		OutputDebugStringA("]");

		//save volatile registers
		RegUtils::saveVolatileRegisters();

		__asm { //check if file exists
			push 0
			push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
			call AssetFileExists
			mov fileNotExists, EAX //save comparison result
		}

		//restore volatile registers
		RegUtils::restoreVolatileRegisters();
	}

	//fall-back to default GP4 file name
	if (fileNotExists)
	{
		//dummy call to ReplaceWildCards to ensure registries and stack are properly set
		__asm call ReplaceWildCards

		//calculate new file name using GP4 default naming convention
		calcFileName(true);
	}

	__asm jmp genericMeshJumpBackAddress //jump back into regular flow
}

void initIndividualMeshVariables()
{
	//clear map and save all necessary variables/values
	variables.clear();
	variables["track"] = to_string(track);
	variables["teamname"] = MemUtils::addressToValue<char*>(espVar + 0x08);
	variables["car"] = to_string(MemUtils::addressToValue<int>(espVar + 0x0C));
	variables["lod"] = to_string(MemUtils::addressToValue<int>(espVar + 0x10));
	variables["team"] = to_string(MemUtils::addressToValue<int>(espVar + 0x484) + 1); //19E20C
	variables["driver"] = to_string(MemUtils::addressToValue<int>(espVar + 0x774) & 0xFF); //0019E4FC - The driver number is only the first byte of the DWORD 
	variables["driverindex"] = to_string(MemUtils::addressToValue<int>(espVar + 0x484 + 0x0C) + 1); //19E218
}

__declspec(naked) void individualMeshFunc()
{
	//dummy call to ReplaceWildCards to ensure registries and stack are properly set
	__asm call ReplaceWildCards

	//save stack pointer
	__asm mov espVar, ESP

	//set current mesh index
	initMeshIndex(true);

	//set current track index
	initTrackIndex();

	//set variables
	initIndividualMeshVariables();

	//calculate new file name
	calcFileName();

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //check if file exists
		push 0
		push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
		call AssetFileExists
		mov fileNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to specified track index
	if (fileNotExists)
	{
		//set fall-back track index
		initTrackIndex(true);

		//set variables
		initIndividualMeshVariables();

		OutputDebugStringA("Reverting to default track [");

		//calculate new file name
		calcFileName();

		OutputDebugStringA("]");

		//save volatile registers
		RegUtils::saveVolatileRegisters();

		__asm { //check if file exists
			push 0
			push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
			call AssetFileExists
			mov fileNotExists, EAX //save comparison result
		}

		//restore volatile registers
		RegUtils::restoreVolatileRegisters();
	}

	//fall-back to default GP4 file name
	if (fileNotExists)
	{
		//dummy call to ReplaceWildCards to ensure registries and stack are properly set
		__asm call ReplaceWildCards

		//calculate new file name using GP4 default naming convention
		calcFileName(true);
	}

	__asm jmp individualMeshJumpBackAddress //jump back into regular flow
}

void initAssetIndex(ASSET_ID index)
{
	assetIndex = index;
}

void initCockpitTextureVariables()
{
	//clear map and save all necessary variables/values
	variables.clear();
	variables["track"] = to_string(track);
	variables["teamname"] = MemUtils::addressToValue<char*>(espVar + 0x08);
	variables["car"] = to_string(MemUtils::addressToValue<int>(espVar + 0x14) + 1);
	variables["team"] = to_string(MemUtils::addressToValue<int>(espVar + 0x0C) + 1);
	variables["driver"] = to_string(MemUtils::addressToValue<int>(espVar + 0x2f4) & 0xFF); //The driver number is only the first byte of the DWORD
	variables["driverindex"] = to_string(MemUtils::addressToValue<int>(espVar + 0x10) + 1);
}

__declspec(naked) void cockpitTextureFunc()
{
	//dummy call to ReplaceWildCards to ensure registries and stack are properly set
	__asm call ReplaceWildCards

	//save stack pointer
	__asm mov espVar, ESP

	//set cockpit asset index
	initAssetIndex(COCKPIT_TEX);

	//set current track index
	initTrackIndex();

	//set variables
	initCockpitTextureVariables();

	//calculate new file name
	calcFileName();

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	fileNameVar = MemUtils::addressToValue<DWORD>(espVar);

	prepFileNameString(fileNameVar);

	__asm { //check if file exists
		push 0
		push fileNameVar //filename
		call AssetFileExists
		mov fileNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to specified track index
	if (fileNotExists)
	{
		//set fall-back track index
		initTrackIndex(true);

		//set variables
		initCockpitTextureVariables();

		OutputDebugStringA("Reverting to default track [");

		//calculate new file name
		calcFileName();

		OutputDebugStringA("]");

		//save volatile registers
		RegUtils::saveVolatileRegisters();

		fileNameVar = MemUtils::addressToValue<DWORD>(espVar);

		prepFileNameString(fileNameVar);

		__asm { //check if file exists
			push 0
			push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
			call AssetFileExists
			mov fileNotExists, EAX //save comparison result
		}

		//restore volatile registers
		RegUtils::restoreVolatileRegisters();
	}

	//fall-back to default GP4 file name
	if (fileNotExists)
	{
		//dummy call to ReplaceWildCards to ensure registries and stack are properly set
		__asm call ReplaceWildCards

		//calculate new file name using GP4 default naming convention
		calcFileName(true);
	}

	__asm jmp cockpitTextureJumpBackAddress //jump back into regular flow
}

void initHelmetTextureVariables()
{
	int teamIndex = MemUtils::addressToValue<int>(espVar + 0x0C);
	//clear map and save all necessary variables/values
	variables.clear();
	variables["track"] = to_string(track);
	variables["teamname"] = MemUtils::addressToPtr<char>(arrTeamNames + (teamIndex * 0x20));
	variables["car"] = to_string(MemUtils::addressToValue<int>(espVar + 0x14) + 1);
	variables["team"] = to_string(teamIndex + 1);
	variables["driver"] = to_string(MemUtils::addressToValue<int>(espVar + 0x08));
	variables["driverindex"] = to_string(MemUtils::addressToValue<int>(espVar + 0x10) + 1);
}

__declspec(naked) void helmetTexture1Func()
{
	//dummy call to ReplaceWildCards to ensure registries and stack are properly set
	__asm call ReplaceWildCards

	//save stack pointer
	__asm mov espVar, ESP

	//set helmet 1 asset index
	initAssetIndex(HELMET_1);

	//set current track index
	initTrackIndex();

	//set variables
	initHelmetTextureVariables();

	//calculate new file name
	calcFileName();

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	fileNameVar = MemUtils::addressToValue<DWORD>(espVar);

	prepFileNameString(fileNameVar);

	__asm { //check if file exists
		push 0
		push fileNameVar //filename
		call AssetFileExists
		mov fileNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to specified track index
	if (fileNotExists)
	{
		//set fall-back track index
		initTrackIndex(true);

		//set variables
		initHelmetTextureVariables();

		OutputDebugStringA("Reverting to default track [");

		//calculate new file name
		calcFileName();

		OutputDebugStringA("]");

		//save volatile registers
		RegUtils::saveVolatileRegisters();

		fileNameVar = MemUtils::addressToValue<DWORD>(espVar);

		prepFileNameString(fileNameVar);

		__asm { //check if file exists
			push 0
			push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
			call AssetFileExists
			mov fileNotExists, EAX //save comparison result
		}

		//restore volatile registers
		RegUtils::restoreVolatileRegisters();
	}

	//fall-back to default GP4 file name
	if (fileNotExists)
	{
		//dummy call to ReplaceWildCards to ensure registries and stack are properly set
		__asm call ReplaceWildCards

		//calculate new file name using GP4 default naming convention
		calcFileName(true);
	}

	__asm jmp helmetTexture1JumpBackAddress //jump back into regular flow
}

__declspec(naked) void helmetTexture2Func()
{

	//dummy call to ReplaceWildCards to ensure registries and stack are properly set
	__asm call ReplaceWildCards

	//save stack pointer
	__asm mov espVar, ESP

	//set helmet 2 asset index
	initAssetIndex(HELMET_2);

	//set current track index
	initTrackIndex();

	//set variables
	initHelmetTextureVariables();

	//calculate new file name
	calcFileName();

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	fileNameVar = MemUtils::addressToValue<DWORD>(espVar);

	prepFileNameString(fileNameVar);

	__asm { //check if file exists
		push 0
		push fileNameVar //filename
		call AssetFileExists
		mov fileNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to specified track index
	if (fileNotExists)
	{
		//set fall-back track index
		initTrackIndex(true);

		//set variables
		initHelmetTextureVariables();

		OutputDebugStringA("Reverting to default track [");

		//calculate new file name
		calcFileName();

		OutputDebugStringA("]");

		//save volatile registers
		RegUtils::saveVolatileRegisters();

		fileNameVar = MemUtils::addressToValue<DWORD>(espVar);

		prepFileNameString(fileNameVar);

		__asm { //check if file exists
			push 0
			push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
			call AssetFileExists
			mov fileNotExists, EAX //save comparison result
		}

		//restore volatile registers
		RegUtils::restoreVolatileRegisters();
	}

	//fall-back to default GP4 file name
	if (fileNotExists)
	{
		//dummy call to ReplaceWildCards to ensure registries and stack are properly set
		__asm call ReplaceWildCards

		//calculate new file name using GP4 default naming convention
		calcFileName(true);
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

	__asm call LoadCarRelatedMesh

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

void initCollisionMeshVariables()
{
	int teamIndex = MemUtils::addressToValue<int>(espVar + 0x04);
	//clear map and save all necessary variables/values
	variables.clear();
	variables["track"] = to_string(track);
	variables["teamname"] = MemUtils::addressToPtr<char>(arrTeamNames + (teamIndex * 0x20));
	variables["car"] = to_string(MemUtils::addressToValue<int>(espVar + 0x0c) + 1);
	variables["team"] = to_string(teamIndex + 1);
	variables["driver"] = to_string(MemUtils::addressToValue<int>(espVar + 0x2EC) & 0xFF); //19E4DC - 19E1F0 - The driver number is only the first byte of the DWORD 
	variables["driverindex"] = to_string(MemUtils::addressToValue<int>(espVar + 0x08) + 1);
}

__declspec(naked) void collisionMeshFunc()
{
	//dummy push of original string
	//__asm push 0x644e44

	//save stack pointer
	__asm mov espVar, ESP

	//set collision mesh asset index
	initAssetIndex(COLLISION_MESH);

	//set current track index
	initTrackIndex();

	//set variables
	initCollisionMeshVariables();

	//calculate new file name
	calcFileName();

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //check if file exists
		push 0
		push collisionMesh //filename
		call AssetFileExists
		mov fileNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to specified track index
	if (fileNotExists)
	{
		//set fall-back track index
		initTrackIndex(true);

		//set variables
		initCollisionMeshVariables();

		OutputDebugStringA("Reverting to default track [");

		//calculate new file name
		calcFileName();

		OutputDebugStringA("]");

		//save volatile registers
		RegUtils::saveVolatileRegisters();

		__asm { //check if file exists
			push 0
			push collisionMesh //filename
			call AssetFileExists
			mov fileNotExists, EAX //save comparison result
		}

		//restore volatile registers
		RegUtils::restoreVolatileRegisters();
	}

	//fall-back to default GP4 file name
	if (fileNotExists)
	{
		//calculate new file name using GP4 default naming convention
		calcFileName(true);
	}
	//push new collision mesh file name
	__asm push collisionMesh

	__asm jmp collisionMeshJumpBackAddress //jump back into regular flow
}

void SetCockpitVisorShaderParameters()
{
	//Get cockpit mesh
	byte* cockpitMeshStruct = GP4MemLib::MemUtils::addressToValue<byte*>(cockpitMesh);

	//Get number of cockpit objects
	int* numCockpitObjects = (int*)(cockpitMeshStruct + 0x1b0);

	if (visorObjectIndex > -1 && visorObjectIndex < *numCockpitObjects)
	{
		//Get array of mesh objects
		byte* arrMeshObjects = cockpitMeshStruct + 0x1b4;

		//Get pointer to visor object
		DWORD* visorObject = (DWORD*)(*(DWORD*)arrMeshObjects + visorObjectIndex * 0x34);

		//Get Index of visor object
		int* visorObjectIndex = (int*)(&visorObject[3]);

		//Get visor object data
		DWORD** visorObjectData = (DWORD**)(visorObject[2]);

		//Get Array of Shader Data
		CGP4MeshShaderData* arrayShaderData = (CGP4MeshShaderData*)visorObjectData[5];

		//Set Shader Parameters
		//Colour
		std::memcpy(&arrayShaderData[*visorObjectIndex].params[0], &visorColour, 4);

		//Multiplier
		std::memcpy(&arrayShaderData[*visorObjectIndex].params[1], &transparencyMultiplier, 4);

		//Unknown
		arrayShaderData[*visorObjectIndex].params[2] = 0xffffff;

		//Multiplier
		std::memcpy(&arrayShaderData[*visorObjectIndex].params[3], &transparencyMultiplier, 4);

		//Unknown
		arrayShaderData[*visorObjectIndex].params[4] = GP4MemLib::MemUtils::addressToValue<DWORD>(0x00644274);

		//Unknown, possibly reflections
		if(CGP4Car != NULL)
		{
			float val1 = atan2f(GP4MemLib::MemUtils::addressToValue<float>(CGP4Car + 0x354), GP4MemLib::MemUtils::addressToValue<float>(CGP4Car + 0x35c));
			float val2 = atan2f(GP4MemLib::MemUtils::addressToValue<float>(0x00a53294), GP4MemLib::MemUtils::addressToValue<float>(0x00a5329c));

			float val = (val2 + 3.1415927f + val1 + 3.1415927f) * 0.5f * 0.03183099f;

			std::memcpy(&arrayShaderData[*visorObjectIndex].params[5], &val, 4);
		}
		else
		{
			arrayShaderData[*visorObjectIndex].params[5] = 0;
		}

	}

	/* Assembly code for visor object

	__asm {
		//get cockpit mesh
		mov EDI, cockpitMesh
		mov EDI, dword ptr[EDI]

		//get number of mesh objects
		mov EDX, dword ptr[EDI + 0x1b0]

		//get cockpit visor object index
		mov EAX, visorObjectIndex

		cmp EAX, EDX

		//skip if index > number of objects
		jnc InvalidIndex

		//get mesh objects array

		lea ECX, [EAX + EAX * 0x2]
		lea EAX, [EAX + ECX * 0x4]

		mov ECX, dword ptr[EDI + 0x1b4]

		//get pointer to object
		lea ECX, [ECX + EAX * 0x4]

		//get pointer to shader parameters
		mov EAX, dword ptr[ECX + 0xc]
		mov ECX, dword ptr[ECX + 0x8]
		mov ECX, dword ptr[ECX + 0x14]
		lea EAX, [EAX + EAX * 0x8]
		lea ECX, [ECX + EAX * 0x4 + 0x4]

		//set parameters
		//param 0 = colour
		mov dword ptr[ECX], 0x808080

		//param 1
		mov dword ptr[ECX + 0x4], 0x3f000000 //0x3e99999a //Multiplier

		//param 2
		mov dword ptr[ECX + 0x8], 0xffffff

		//param 3
		mov dword ptr[ECX + 0xc], 0x3f000000 //0x3f800000 //Multiplier

		//param 4
		mov EDX, dword ptr[0x00644274]
		mov EDX, dword ptr[EDX]
		mov dword ptr[ECX + 0x10], EDX

		//param 5
		mov dword ptr[ECX + 0x14], 0x0
	}
InvalidIndex:

	//set pointer to 0
	__asm xor ECX, ECX
	*/
}

__declspec(naked) void cockpitVisorFunc()
{
	__asm {
		//set mesh
		mov ECX, dword ptr[EDI + ESI * 0x4 + 0x4]

		//set name of object to be visor
		//push visorObjectName
		lea EBP, visorObjectName
		push EBP

		//get index of visor object, it will be in EAX
		call GetObjectIndexByName

		//save index in EBP
		mov EBP, EAX

		//skip if object not found
		cmp EBP, -1

		//save index into variable
		mov visorObjectIndex, EBP

		jz noObjectFound

		//if found
		//set flag
		push 0x1

		//set object index
		push EBP

		//get the shader (it will be in EAX)
		push 0x11

		call GetShader

		//set mesh (this)
		mov ECX, dword ptr[EDI + ESI * 0x4 + 0x4]

		//advance stack
		add ESP, 0x4

		//set shader
		push EAX

		//apply shader
		call ApplyShaderToObject
	}

	SetCockpitVisorShaderParameters();

noObjectFound:

	__asm call SetCockpitMirrorShaderParameters //original function call
		
	__asm jmp cockpitVisorJumpBackAddress //jump back into regular flow
}

__declspec(naked) void cockpitVisorFunc2()
{
	__asm {
		mov CGP4Car, EBP
	}

	SetCockpitVisorShaderParameters();

	__asm call SetCockpitMirrorShaderParameters //original function call

	__asm jmp cockpitVisorJumpBackAddress2 //jump back into regular flow
}

DWORD WINAPI MainThread(LPVOID param) {

	//Utility string builders
	ostringstream messageBuilder;
	ostringstream fileNameBuilder;

	//Get the DLL path and use it to load the ini file
	char currentPath[MAX_PATH];
	HMODULE dllHandle = GetModuleHandleA("IndividualMeshesAndTextures.dll");
	GetModuleFileNameA(dllHandle, currentPath, MAX_PATH);
	size_t pos = string(currentPath).find_last_of("\\/");
	string iniFilePath = string(currentPath).substr(0, pos) + "\\IndividualMeshesAndTextures.ini";

	IniFile iniSettings;

	if (iniSettings.load(iniFilePath))
	{

		try
		{
			trackFolders = iniSettings["Settings"]["TrackFolders"].getAs<bool>();
		}
		catch (exception ex) {}

		OutputDebugStringA(("Track Folders : " + string(trackFolders ? "Enabled" : "Disabled")).c_str());

		try
		{
			cockpitVisor = iniSettings["Settings"]["CockpitVisor"].getAs<bool>();
		}
		catch (exception ex) {}

		OutputDebugStringA(("Cockpit Visor : " + string(trackFolders ? "Enabled" : "Disabled")).c_str());

		if(cockpitVisor)
		{
			try
			{
				visorObjectName = iniSettings["CockpitVisor"]["ObjectName"].getString();
			}
			catch (exception ex) {}

			OutputDebugStringA(("Cockpit Visor Object Name: " + visorObjectName).c_str());

			try
			{
				visorBytes = iniSettings["CockpitVisor"]["Colour"].getVectorAs<short>();
			}
			catch (exception ex) {}

			while (visorBytes.size() < 4)
			{
				visorBytes.push_back(0);
			}

			visorColour = 0;

			for (unsigned int i = 0; i < visorBytes.size(); i++)
			{
				if (visorBytes[i] > 255)
				{
					visorBytes[i] = 255;
				}
				visorColour += (visorBytes[i] << i * 8);
			}

			OutputDebugStringA(("Cockpit Visor Colour: B: " + to_string(visorBytes[0]) + " G: " + to_string(visorBytes[1]) + " R: " + to_string(visorBytes[2]) + " A: " + to_string(visorBytes[3])).c_str());

			try
			{
				transparencyMultiplier = iniSettings["CockpitVisor"]["Transparency"].getAs<float>();
			}
			catch (exception ex) {}

			OutputDebugStringA(("Cockpit Visor Transparency Multiplier: " + to_string(transparencyMultiplier)).c_str());
		}

		for (int assetIndex = 0; assetIndex < 9; assetIndex++)
		{

			if (assetIndex < 4)
			{
				try
				{
					individualMeshesEnabled[assetIndex] = iniSettings["IndividualMeshes"][assetIDs[assetIndex]].getAs<bool>();
				}
				catch (exception ex) {}

				MemUtils::patchAddress((LPVOID)(individualMeshesAddress + assetIndex), MemUtils::toBytes(!individualMeshesEnabled[assetIndex]), sizeof(bool));

				OutputDebugStringA(("Individual " + assetNames[assetIndex] + ": " + (individualMeshesEnabled[assetIndex] ? "Enabled" : "Disabled")).c_str());
			}

			if (assetIndex < 5)
			{
				//Check if LOD 0 Only is enabled
				try
				{
					lod0Only[assetIndex] = iniSettings[assetIDs[assetIndex]]["LOD0Only"].getAs<bool>();
				}
				catch (exception ex) {}

				OutputDebugStringA(("LOD 0 Only " + assetNames[assetIndex] + ": " + (lod0Only[assetIndex] ? "Enabled" : "Disabled")).c_str());

				vector<int> lodEntries;

				//If not LOD 0 Only, read LOD Table
				if (!lod0Only[assetIndex])
				{
					try
					{
						lodEntries = iniSettings["LODTable"][assetIDs[assetIndex]].getVectorAs<int>();
					}
					catch (exception ex) {}

					messageBuilder.str(string());

					messageBuilder << "LOD Table for " + assetNames[assetIndex] + ": ";

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
				int nLods;
				for (nLods = 1; lodEntries[nLods] > 0 && nLods < 5; nLods++);

				messageBuilder.str(string());

				messageBuilder << "Number LODs for " << assetNames[assetIndex] << ": " << nLods;

				OutputDebugStringA(messageBuilder.str().c_str());

				//Patch Number of LODs per Mesh
				MemUtils::patchAddress((LPVOID)(lodsPerMeshAddress + assetIndex * sizeof(int)), MemUtils::toBytes(nLods), sizeof(int));

				//Patch LOD Table entries
				DWORD tableEntryAddress = lodTableAddress + (assetIndex * sizeof(int) * lodEntries.size());

				MemUtils::patchAddress((LPVOID)tableEntryAddress, MemUtils::toBytes(lodEntries[0]), sizeof(int) * lodEntries.size());
			}

			//Check if AutoName
			bool autoName = false;

			try
			{
				autoName = iniSettings[assetIDs[assetIndex]]["AutoName"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputDebugStringA(("AutoName " + assetNames[assetIndex] + ": " + (autoName ? "Enabled" : "Disabled")).c_str());

			//Check if per Team
			try
			{
				perTeam[assetIndex] = iniSettings[assetIDs[assetIndex]]["PerTeam"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputDebugStringA(("Per Team " + assetNames[assetIndex] + ": " + (autoName ? "Enabled" : "Disabled")).c_str());

			//Check if per Driver
			try
			{
				perDriver[assetIndex] = iniSettings[assetIDs[assetIndex]]["PerDriver"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputDebugStringA(("Per Driver " + assetNames[assetIndex] + ": " + (autoName ? "Enabled" : "Disabled")).c_str());

			//Check if per Track
			try
			{
				perTrack[assetIndex] = iniSettings[assetIDs[assetIndex]]["PerTrack"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputDebugStringA(("Per Track " + assetNames[assetIndex] + ": " + (autoName ? "Enabled" : "Disabled")).c_str());

			//Load Track Table
			try
			{
				tracks[assetIndex] = iniSettings[assetIDs[assetIndex]]["Tracks"].getVectorAs<int>();
			}
			catch (exception ex) {}

			// Compute file name
			if (autoName)
			{
				fileNameBuilder.str(string());

				if (perTrack[assetIndex] && trackFolders)
				{
					fileNameBuilder << "Track{track}\\";
				}

				if (assetIndex < ASSET_ID::HELMET_1)
				{

					fileNameBuilder << "car_";

					if (individualMeshesEnabled[assetIndex] && (perTeam[assetIndex] || perDriver[assetIndex]))
					{
						fileNameBuilder << "{teamname}_";
					}

					fileNameBuilder << filePartialNames[assetIndex];

					if (perDriver[assetIndex] && individualMeshesEnabled[assetIndex])
					{
						fileNameBuilder << "{car}";
					}

					if (assetIndex > ASSET_ID::REAR_WHEELS && assetIndex < ASSET_ID::CARS)
					{
						fileNameBuilder << "_0{lod}";
					}
					else if (assetIndex < ASSET_ID::COLLISION_MESH)
					{
						fileNameBuilder << "_LOD_{lod}";
					}

					if (perTrack[assetIndex] && !trackFolders)
					{
						fileNameBuilder << "_track_{track}";
					}

					if (assetIndex == ASSET_ID::COLLISION_MESH)
					{
						fileNameBuilder << ".ct";
					}
				}
				else
				{
					fileNameBuilder << filePartialNames[assetIndex];

					if (perDriver[assetIndex])
					{
						fileNameBuilder << "{driver}";
					}
					if (perTeam[assetIndex])
					{
						fileNameBuilder << "_{teamname}";
					}
					if (perTrack[assetIndex] && !trackFolders)
					{
						fileNameBuilder << "_track_{track}";
					}
					if (assetIndex < ASSET_ID::COCKPIT_TEX)
					{
						fileNameBuilder << "_" << assetIndex - (ASSET_ID::HELMET_1 - 1);
					}
					fileNameBuilder << ".tex";
				}

				fileNames[assetIndex] = fileNameBuilder.str().c_str();

				OutputDebugStringA(("File name computed for " + assetNames[assetIndex] + ": " + fileNames[assetIndex]).c_str());
			}
			else //If not, load the specified file name
			{
				string key = "FileName";
				if (assetIndex == ASSET_ID::HELMET_1)
				{
					key += "1";
				}
				else if (assetIndex == ASSET_ID::HELMET_2)
				{
					key += "2";
				}
				try
				{
					fileNames[assetIndex] = iniSettings[assetIDs[assetIndex]][key].getAs<string>();
				}
				catch (exception ex) {}

				OutputDebugStringA(("File name loaded for " + assetNames[assetIndex] + ": " + fileNames[assetIndex]).c_str());
			}

			//Fall back to default name
			if (fileNames[assetIndex].empty())
			{
				fileNames[assetIndex] = defaultFileNames[assetIndex];

				OutputDebugStringA(("Reverting to default file name for " + assetNames[assetIndex] + ": " + fileNames[assetIndex]).c_str());
			}

		}

	}
	else
	{
		OutputDebugStringA("Failed to open INI file");
	}

	//fill track tables if necessary
	for (int assetIndex = 0; assetIndex < 8; assetIndex++)
	{
		if (tracks[assetIndex].empty())
		{
			tracks[assetIndex] = defaultTracks;
		}
		if (tracks[assetIndex].size() < 17)
		{
			for (int i = tracks[assetIndex].size(); tracks[assetIndex].size() < 17; i++)
			{
				tracks[assetIndex].push_back(1);
			}
		}
	}

	//Re-route for cockpit mirrors 
	MemUtils::rerouteFunction(cockpitMirrorsSingleStartAddress, PtrToUlong(checkNullPointerSingleCockpitMirrors), VAR_NAME(checkNullPointerSingleCockpitMirrors));

	//Re-route for cockpit mirrors per car
	MemUtils::rerouteFunction(cockpitMirrorsPerCarStartAddress, PtrToUlong(applyCockpitMirrorsPerCar), VAR_NAME(applyCockpitMirrorsPerCar));

	//Re-route for cockpit visor
	MemUtils::rerouteFunction(cockpitVisorStartAddress, PtrToUlong(cockpitVisorFunc), VAR_NAME(cockpitVisorFunc));

	//Re-route for cockpit visor 2
	MemUtils::rerouteFunction(cockpitVisorStartAddress2, PtrToUlong(cockpitVisorFunc2), VAR_NAME(cockpitVisorFunc2));

	//Re-route for generic meshes
	MemUtils::rerouteFunction(genericMeshStartAddress, PtrToUlong(genericMeshFunc), VAR_NAME(genericMeshFunc));

	//Re-route for individual meshes
	MemUtils::rerouteFunction(individualMeshStartAddress, PtrToUlong(individualMeshFunc), VAR_NAME(individualMeshFunc));

	//Re-route for cockpit textures
	MemUtils::rerouteFunction(cockpitTextureSartAddress, PtrToUlong(cockpitTextureFunc), VAR_NAME(cockpitTextureFunc));

	//Re-route for helmet texture 1
	MemUtils::rerouteFunction(helmetTexture1StartAddress, PtrToUlong(helmetTexture1Func), VAR_NAME(helmetTexture1Func));

	//Re-route for helmet texture 2
	MemUtils::rerouteFunction(helmetTexture2StartAddress, PtrToUlong(helmetTexture2Func), VAR_NAME(helmetTexture2Func));

	//Re-route for collision mesh
	MemUtils::rerouteFunction(collisionMeshStartAddress, PtrToUlong(collisionMeshFunc), VAR_NAME(collisionMeshFunc));

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