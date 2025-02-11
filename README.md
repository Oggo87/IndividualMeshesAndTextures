# Individual Meshes And Textures

A DLL to enable customisable loading for meshes and textures in Grand Prix 4, plus a couple of extra features.

## Description

This DLL expands original Grand Prix 4 to allow full control of how meshes (front/rear wheels, helmets, cockpits, cars) and textures (cockpits, helmets) are loaded, supporting per-driver, per-team, and per-track custom loading. It includes features to manage LODs (Level of Detail), auto-naming conventions, and fallbacks to default GP4 assets when custom assets are unavailable.

Additionally, it enables using the built-in visor shader in the cockpit mesh and fixes the reversed tyre tread bug when using 3D rims.

See [Usage](#Usage) for more info.

## Getting Started

### Dependencies

* [GP4MemLib](https://github.com/Oggo87/GP4MemLib)
* [IniLib](https://github.com/Oggo87/IniLib)

### Prerequisites

* Grand Prix 4
* GP4 Memory Access by Carl_gpgames (recommended) or
* CheatEngine or
* Any other DLL injection tool

### Installing

#### GP4 Memory Access

Under the *DLLs* tab, add an entry pointing to `IndividualMeshesAndTextures.dll`. Make sure that `IndividualMeshesAndTextures.ini` is located in the same folder as the DLL.

### Usage

All settings are contained in `IndividualMeshesAndTextures.ini` and everything is designed to revert back to default GP4 behaviour, including if `IndividualMeshesAndTextures.ini` itself missing. The `IndividualMeshesAndTextures.ini` file that is included with the release is also pre-configured to mimic GP4's default behaviour.

`IndividualMeshesAndTextures.ini` is divided in 4 logical sections: 

* [Individual Meshes](#Individual-Meshes)
* [LOD Table](#LOD-Table)
* [Settings](#Settings)
* [Assets Settings](#Assets-Settings)

#### Individual Meshes

This section contains entries to enable individual mesh loading for mesh types that are generic by default in GP4: Front Wheels, Rear Wheels, Helmets and Cockpits. Entries can be set to `true` / `false` or `1` / `0`.

Cars are not included in this section, as they are already use individual loading by default and `IndividualMeshesAndTextures.dll`, by design choice, does not allow to set them as generic.

```ini
[IndividualMeshes]
FrontWheels = false
RearWheels = false
Helmets = false
Cockpits = false
```

#### LOD Table
This section allows to specify which LODs (Levels of Detail) will be used by each mesh type. GP4 allows a maximum of 5 LODs per mesh type. `IndividualMeshesAndTextures.dll` will automatically exclude all 0 values at the _end_ of each entry, and will also 0-fill any entries that contain less than 5 values, guaranteeing full compatibility with GP4 internal loading. 

```ini
[LODTable]
FrontWheels	=	0,	1,	2,	3,	0
RearWheels	=	0,	1,	2,	3,	0
Helmets		=	0,	1,	2,	3,	0
Cockpits	=	0,	0,	0,	0,	0
Cars		=	0,	1,	2,	3,	4
```

#### Settings
This section contains two entries:

* [Track Folders](#Track-Folders)
* [Cockpit Visor](#Cockpit-Visor)

```ini
[Settings]
TrackFolders = false
CockpitVisor = false
```

##### Track Folders
This utility setting, in combination with `AutoName` found in each individual asset section (see [Assets Settings](#Assets-Settings)), tells `IndividualMeshesAndTextures.dll` to look into track subfolders for those assets that are have `PerTrack` set to `true`.

Tracks are indexed 1 through 17.

Example - Per-Track Cars with Track Folders enabled

```ini
[Settings]
TrackFolders = true

[Cars]
AutoName = true
PerTrack = true
```

Ferrari's first car (LOD 0) at Monza will be loaded as follows

```
cars\track15\car_ferrari_car1_lod_0.gp4
```

##### Cockpit Visor

This setting allows to enable using GP4's built-in visor shader in the cockpit mesh. In combination with the entries in the ```CockpitVisor``` section, it's possible to specify the ```ObjectName``` to apply the visor shader to, the ```Colour``` (in ```0-255 BGRA``` format),  and ```Transparency``` (a ```float``` value). The Alpha component of the ```Colour``` and the ```Transparency``` value work combinedly to set the visor's transparency.

Example - Cockpit Visor enabled, using object named `Visor`, with a grey colour and 50% transparency

```ini
[Settings]
CockpitVisor = true

[CockpitVisor]
ObjectName = Visor
Colour = 128, 128, 128, 0
Transparency = 0.5
```

#### Assets Settings

These sections are, more specifically:

* Front Wheels
* Rear Wheels
* Helmets
* Cockpits
* Cars
* Collision Mesh
* Helmet Textures
* Cockpit Textures

They all share a common structure, with some minor differences for the two texture sections:

```ini
AutoName = true
LOD0Only = false
PerTeam = false
PerDriver = false
PerTrack = false
FileName = 
Tracks = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
```

##### AutoName
Available for all assets, `AutoName` works in conjunction with `IndividualMeshes`, `PerTeam`, `PerDriver`, `PerTrack` and `TrackFolders`, and allows `IndividualMeshesAndTextures.dll` to automatically determine what file names to use, depending on the chosen options.

`AutoName` follows the convention of original GP4 files, trying to keep it as close as possible to the original file names, including the little differences like `LOD_0` for Cars and Wheels and `00` for Helmets and Cockpits.

See [`PerTrack`](#PerTeam-PerDriver-PerTrack) for some more details, it is recommended to check the GPxPatch debug log for the asset name(s) being loaded.

#### LOD0Only
Available for mesh entries only, overrides the corresponding entry in the [LOD Table](#LOD-Table), forcing to use only LOD 0.

#### PerTeam, PerDriver, PerTrack
Available for all assets, these settings only have an effect when [`AutoName`](#AutoName) is enabled for the corresponding asset.

* `PerTrack`, when not using `TrackFolders`, will append `_track_{track}` at the end of the filename.
* `PerDriver` will combine `{teamname}` with car `1` and `2` for meshes, while using `{driver}` for textures.

See [`FileName`](#FileName) for more details about `{variables}`.

#### FileName
Available for all assets, this setting allows to specify a custom file name pattern for each asset. For **Helmet Textures**, there are two independent settings, one per texture: `FileName1` and `FileName2`.

7 `{variables}` are available to use in the file names, enclosed in curly brackets:

* `{track}` - Available for all assets. Indexed 1 through 17
* `{team}` - Available for collision mesh and all textures, available for meshes only when [`IndividualMeshes`](#Individual-Meshes) is enabled for the corresponding mesh. Indexed 1 through 11.
* `{teamname}` - Available for collision mesh and all textures, available for meshes only when [`IndividualMeshes`](#Individual-Meshes) is enabled for the corresponding mesh.
* `{car}` - Available for all textures, available for meshes only when [`IndividualMeshes`](#Individual-Meshes) is enabled for the corresponding mesh. Indexed 1 and 2 for first and second car of the team.
* `{driver}` - Available for collision mesh and all textures, available for meshes only when [`IndividualMeshes`](#Individual-Meshes) is enabled for the corresponding mesh. Indexed 1 through 12 and 14 through 23 (as for original GP4 helmet textures, following 2001 car numbering)
* `{driverindex}` - Available for collision mesh and all textures, available for meshes only when [`IndividualMeshes`](#Individual-Meshes) is enabled for the corresponding mesh. Indexed 1 through 22.
* `{lod}` - Available for for meshes only, follows the corresponding entry in the [LOD Table](#LOD-Table)

Please note that collision mesh and texture entries need to have the extension explicitly written as either `.tex` or `.tga` for textures and `.ct` for collision mesh

Example - Per-Team Helmet mesh and Per-Track Helmet Textures

```ini
[Helmets]
FileName=car_helmet_{teamname}_0{lod}

[HelmetTextures]
FileName1=driver{driver}_track_{track}_1.tga
FileName1=driver{driver}_track_{track}_2.tga
```

Ferrari's first driver helmet assets at Monza will be loaded as follows

```
cars\car_helmet_ferrari_00.gp4
cars\driver1_track_15_1.tex
cars\driver1_track_15_2.tex
```

Subolders can be included as part of the `FileName` entry. Please note that the directory separator character has to be escaped and written as `\\`.

Example - Per-Track generic Helmet mesh

```ini
[Helmets]
FileName=track{track}\\car_helmet_0{lod}
```

The Helmet mesh at Monza will be loaded as follows

```
cars\track15\car_helmet_00.gp4
```

#### Tracks
Available for all assets, this setting allows some additional control when using per-track assets, either with the [`PerTrack`](#PerTeam-PerDriver-PerTrack) setting or the manual `{track}` variable. Acknowledging that not all tracks might need their own unique asset, with this setting is possible to specify a fall-back track index for each track slot. If an asset isn't found for the track slot, the asset from the specified fall-back track slot will be attempted to be loaded, before reverting back to GP4's default. If less than 17 entries are present, track 1 will be used for the missing track slots.

Example - Per-Team Helmet mesh, with Track Folders enabled and all fall-back tracks set to 1

```ini
[Settings]
TrackFolders = true

[Cars]
AutoName = true
PerTrack = true
Tracks = 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
```

Ferrari's first car (LOD 0) at Monza will be loaded as follows

```
cars\track15\car_ferrari_car1_lod_0.gp4
```

If the asset doesn't exist, the fall-back track slot will be used

```
cars\track1\car_ferrari_car1_lod_0.gp4
```

If the fall-back asset also doesn't exist, GP4's default file will be used

```
cars\car_ferrari_car1_lod_0.gp4
```

## Authors

Diego "Öggo" Noriega

## Version History

* 1.4
    * Update - Added fix for reversed tyre tread bug when using 3D rims

* 1.3
    * Update - Enable using GP4 Visor Shader for cockpit mesh

* 1.2
    * Update - Added collision mesh assets

* 1.1
    * Update - Fall-back track slots

* 1.0
    * First Release

* 0.1
    * Pre-Release
