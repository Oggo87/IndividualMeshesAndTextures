#include "GP4MemLib/GP4MemLib.h"
#include "IniLib/IniLib.h"
#include <iomanip>
#include <malloc.h>
#include <string>
#include <map>
#include <windows.h>

using namespace std;
using namespace GP4MemLib;
using namespace IniLib;

enum ASSET_ID
{
	FRONT_WHEELS,
	REAR_WHEELS,
	HELMETS,
	COCKPITS,
	CARS,
	HELMET_2,
	HELMET_1,
	COCKPIT
};

bool trackFolders = false;

//Enable/Disable Individual Meshes for Front Wheels, Rear Wheels, Helmets and Cockpits
bool individualMeshesEnabled[] = { false, false, false, false, true };
bool lod0Only[] = { false, false, false, false, false };
bool perTeam[] = { false, false, false, false, false, false, false, true };
bool perDriver[] = { false, false, false, false, true, true, true, false };
bool perTrack[] = { false, false, false, false, false, false, false, false };

vector<int> defaultTracks = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17 };
vector<int> tracks[8];

string assetNames[] = { "Front Wheels", "Rear Wheels", "Helmets", "Cockpits", "Cars", "Helmet 2", "Helmet 1", "Cockpit" };
string assetIDs[] = { "FrontWheels", "RearWheels", "Helmets", "Cockpits", "Cars", "HelmetTextures", "HelmetTextures", "CockpitTextures"};
string defaultFileNames[] = { "CAR_Wheel_Front_LOD_%d", "CAR_Wheel_Rear_LOD_%d", "CAR_Helmet_%02d", "CAR_Cockpit_%02d", "CAR_%s_CAR%d_LOD_%d", "Driver%d_2.tga", "Driver%d_1.tga", "cp_%s.tga" };
string newFormatDefaultFileNames[] = {"CAR_Wheel_Front_LOD_{lod}", "CAR_Wheel_Rear_LOD_{lod}", "CAR_Helmet_0{lod}", "CAR_Cockpit_0{lod}", "CAR_{teamname}_CAR{car}_LOD_{lod}", "Driver{driver}_2.tga", "Driver{driver}_1.tga", "cp_{teamname}.tga"};
string filePartialNames[] = { "Wheel_Front", "Wheel_Rear", "Helmet", "Cockpit", "Car", "Driver", "Driver", "cp" };

//Array of mesh and textures filenames
string fileNames[8];

//map to store the variables for subsitution
map<string, string> variables;
int meshIndex;
int track;

DWORD espVar;

char fileNameString[64] = "";

//Target Addresses
DWORD individualMeshesAddress = 0x0064428C;
DWORD lodsPerMeshAddress = 0x00644294;
DWORD lodTableAddress = 0x006442A8;
DWORD meshFileNamesTable = 0x00644390;

DWORD genericMeshStartAddress = 0x00487107; //0x00487100;
DWORD individualMeshStartAddress = 0x00487085; //0x00487059;
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
DWORD meshIndexStack = 0x0019E208;
DWORD trackIndex = 0x007AD894;
DWORD carsFolder = 0x00644EE4;
DWORD gp4Extension = 0x0063EE08;

DWORD arrTeamNames = 0x643ba8;
DWORD arrLodsPerMeshType = 0x6442a8;
DWORD defaultTeam = 0x644edc;
DWORD cockpitTexture = 0x644d94;
DWORD helmetTexture1 = 0x644d84;
DWORD helmetTexture2 = 0x644d74;

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
	strcpy_s(textureFileName, 128, (char*)carsFolder);
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

void calcFileName(bool useDefaultFileName = false)
{
	//Get Cars folder and .gp4 extension to compose file path
	string carsFolderString = MemUtils::addressToPtr<char>(carsFolder);
	string gp4ExtensionString = MemUtils::addressToPtr<char>(gp4Extension);

	//Compose file path, including variables
	string path = carsFolderString + replaceVariables(useDefaultFileName ? newFormatDefaultFileNames[meshIndex] : fileNames[meshIndex], variables) + gp4ExtensionString;

	OutputDebugStringA(path.c_str());

	//The first value in the stack is the string that needs to be replaced
	char* stackPtr = MemUtils::addressToValue<char*>(espVar); 

	memcpy(stackPtr, path.c_str(), path.size() + 1);

}

void initMeshIndex()
{
	meshIndex = MemUtils::addressToValue<int>(meshIndexStack);
}

void initTrackIndex()
{
	track = MemUtils::addressToValue<int>(trackIndex);

	if (perTrack[meshIndex])
	{
		track = tracks[meshIndex][track];
	}
}

void initGenericMeshVariables()
{
	//clear map and save all necessary variables/values
	variables.clear();
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
		call MeshFileExists
		mov meshNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to default GP4 file name
	if (meshNotExists)
	{
		//dummy call to ReplaceWildCards to ensure registries and stack are properly set
		__asm call ReplaceWildCards

		//calculate new file name using GP4 default naming convention
		calcFileName(true);

		OutputDebugStringA("Reverting to default GP4 file name [");

		OutputDebugStringA(MemUtils::addressToValue<char*>(espVar));

		OutputDebugStringA("]");
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
	variables["driver"] = to_string(MemUtils::addressToValue<int>(espVar + 0x484 + 0x0C) + 1); //19E218
}

__declspec(naked) void individualMeshFunc()
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
	initIndividualMeshVariables();

	//calculate new file name
	calcFileName();

	//save volatile registers
	RegUtils::saveVolatileRegisters();

	__asm { //check if file exists
		push 0
		push dword ptr[ESP + 0x04] //filename is now in the second value in the stack
		call MeshFileExists
		mov meshNotExists, EAX //save comparison result
	}

	//restore volatile registers
	RegUtils::restoreVolatileRegisters();

	//fall-back to default GP4 file name
	if (meshNotExists)
	{
		//dummy call to ReplaceWildCards to ensure registries and stack are properly set
		__asm call ReplaceWildCards

		//calculate new file name using GP4 default naming convention
		calcFileName(true);

		OutputDebugStringA("Reverting to default GP4 file name [");

		OutputDebugStringA(MemUtils::addressToValue<char*>(espVar));

		OutputDebugStringA("]");
	}

	__asm jmp individualMeshJumpBackAddress //jump back into regular flow
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

		for (int assetIndex = 0; assetIndex < 8; assetIndex++)
		{
			try
			{
				trackFolders = iniSettings["Settings"]["TrackFolders"].getAs<bool>();
			}
			catch (exception ex) {}

			OutputDebugStringA(("Track Folders : " + string(trackFolders ? "Enabled" : "Disabled")).c_str());

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
			if (perTrack[assetIndex])
			{
				try
				{
					tracks[assetIndex] = iniSettings[assetIDs[assetIndex]]["Tracks"].getVectorAs<int>();
				}
				catch (exception ex) {}

				if (tracks[assetIndex].empty())
				{
					tracks[assetIndex] = defaultTracks;
				}

			}

			// Compute file name
			if (autoName)
			{
				fileNameBuilder.str(string());

				if (perTrack[assetIndex] && trackFolders)
				{
					fileNameBuilder << "Track{track}\\";
				}

				if(assetIndex < 5)
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
					
					fileNameBuilder << "_";

					if (assetIndex > 1 && assetIndex < 4)
					{
						fileNameBuilder << "0{lod}";
					}
					else
					{
						fileNameBuilder << "LOD_{lod}";
					}

					if (perTrack[assetIndex] && !trackFolders)
					{
						fileNameBuilder << "_track_{track}";
					}
				}
				else
				{
					fileNameBuilder << filePartialNames[assetIndex];

					if (perTeam[assetIndex])
					{
						fileNameBuilder << "_%s";
					}
					if (perDriver[assetIndex])
					{
						fileNameBuilder << "%d";
					}
					if (perTrack[assetIndex] && !trackFolders)
					{
						fileNameBuilder << "_track_%d";
					}
					if(assetIndex < 7)
					{
						fileNameBuilder << "_" << 7 - assetIndex;
					}
					fileNameBuilder << ".tga";
				}

				fileNames[assetIndex] = fileNameBuilder.str().c_str();

				OutputDebugStringA(("File name computed for " + assetNames[assetIndex] + ": " + fileNames[assetIndex]).c_str());
			}
			else //If not, load the specified file name
			{
				try
				{
					fileNames[assetIndex] = iniSettings[assetIDs[assetIndex]]["FileName"].getAs<string>();
				}
				catch (exception ex) {}

				OutputDebugStringA(("File name loaded for " + assetNames[assetIndex] + ": " + fileNames[assetIndex]).c_str());
			}

			//Fall back to default name
			if (fileNames[assetIndex].empty())
			{
				fileNames[assetIndex] = newFormatDefaultFileNames[assetIndex];

				OutputDebugStringA(("Reverting to default file name for " + assetNames[assetIndex] + ": " + fileNames[assetIndex]).c_str());
			}

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
	MemUtils::rerouteFunction(genericMeshStartAddress, PtrToUlong(genericMeshFunc), VAR_NAME(genericMeshFunc));

	//Re-route for individual meshes
	MemUtils::rerouteFunction(individualMeshStartAddress, PtrToUlong(individualMeshFunc), VAR_NAME(individualMeshFunc));

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