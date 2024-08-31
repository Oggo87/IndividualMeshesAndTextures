# Individual Meshes And Textures

A DLL to enable per-track meshes and textures in Grand Prix 4

## Description

This DLL expands the original Grand Prix 4 to allow per-track meshes (cars, front/rear wheels, cockpits, helmets) and textures (cockpits, helmets). Tracks are indexed 1 through 17. When meshes or textures for the specific track are not available, the ones from track 1 will be used instead.

## Getting Started

### Prerequisites

* Grand Prix 4
* CSM by ZaZ or GP4 Memory Access by Carl_gpgames

### Installing

#### GP4 Memory Access

* Add an entry pointing to `IndividualMeshesAndTextures.dll`

#### CSM

* Copy `IndividualMeshesAndTextures.dll` into your mod Data folder (or any of its subfolders of your choice)
* In the main `.csm` file of you mod, `[Files]` section, add an entry to the `InjectDLL` key 
  
  Example: 
  ```ini
  InjectDll=Files\IndividualMeshesAndTextures.dll
  ```

### Usage

#### Meshes

To enable per-track meshes, the mesh file name templates need to be changed. The strings are located starting at offset `0x00244390` of `GP4.exe`. The track index will be evaluated last, so appending `track_%d` to the original file names will suffice.
* Front Wheels: `CAR_Wheel_Front_LOD_%d_track_%d`
* Rear Wheels: `CAR_Wheel_Rear_LOD_%d_track_%d`
* Helmets: `CAR_Helmet_%02d_track_%d`
* Cockpits: `CAR_Cockpit_%02d_track_%d`
* Cars: `CAR_%s_CAR%d_LOD_%d_track_%d`

You can also enable individual front/rear wheels, helmets and cockpits by setting the bytes located at offset  `0x0024428c` to `0` (one byte per mesh type)
If you're looking to use LOD 0 only, you can either edit the number of LODs per mesh type, located at offset `0x00244294`  (int values, 4 bytes per mesh type) or the LOD table located at  `0x002442a8` (int values, 4 bytes per LOD, 5 entries per mesh type)

All these changes can be done directly in `GP4.exe` using a hex editor (not recommended), or through CSM's `patch.ini` or using GP4 Memory Access. 

Note: future releases will most likely include configurable options to do all of the above.

#### Textures

Currently, texture file names are hard-coded in the DLL (due to the size limitations of the original strings) and are as follows:

* Cockpits: `cp_teamname_carX_track_Y.tex` e.g. `cp_ferrari_car1_track_4.tex` or `cp_minardi_car2_track_15.tex`. Please note that cockpit textures are now per-car and not per-team.
* Helmets: `driverX_Y_track_Z.tex` e.g. `driver3_1_track_1.tex` or `driver22_2_track_15.tex`

Note: future releases will most likely include configurable options to do all of the above.

## Authors

Diego "Öggo" Noriega

## Version History

* 0.1
    * Pre-Release
