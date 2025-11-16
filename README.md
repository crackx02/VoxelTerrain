## VoxelTerrain

<img width="1920" height="1080" alt="crystalCave5" src="https://github.com/user-attachments/assets/cba920f3-a224-48ab-bd59-2473280fc678" />

<img width="1920" height="1080" alt="crystalCave6" src="https://github.com/user-attachments/assets/3ef7e20b-bb4b-4bc5-8a61-9d446422fb52" />

<img width="1920" height="1080" alt="crystalCave2" src="https://github.com/user-attachments/assets/ee402a7c-91da-4c5e-be6b-a7fadbc251f9" />

[![VoxelTerrain Video 1](http://img.youtube.com/vi/SrMF0JXCT84/0.jpg)](http://www.youtube.com/watch?v=SrMF0JXCT84 "VoxelTerrain 1")

[![VoxelTerrain Video 2](http://img.youtube.com/vi/oaj2dcRYAw4/0.jpg)](http://www.youtube.com/watch?v=oaj2dcRYAw4 "VoxelTerrain 2")

This mod re-enables the hidden, unreleased, disabled voxel terrain system in Scrap Mechanic.  

However, **that is not all!**  
As the "vanilla" implementation was found to be somewhat disappointing (for example, the `terrainSphereModification` function throwing an error instead of allowing terrain to be placed back down!), the mod *also* **immediately and massively expands the system** in various ways by implementing a large custom Lua scripting API.  

This API allows for a lot of modding freedom - terrain can be manipulated in a variety of shapes and ways, such as erasing terrain, 
**placing terrain**, painting ground materials and even **importing and voxelizing 3D models!**

There are also features for restricting the system, e.g. preventing players from digging out an important structure or object.

## Showcase Mod

To showcase the gameplay side of things, a [VoxelTerrain Showcase](https://steamcommunity.com/sharedfiles/filedetails/?id=3575301562) mod has been uploaded to the Steam workshop.  

**The showcase mod mainly focuses on the gameplay**, e.g. drilling your way through cave systems, mining resources, etc.  

**Note that the showcase mod requires this VoxelTerrain DLL to be installed (see [How to Use](#howtouse) section below).**

## How to Use

To use this mod, a DLL injector such as [this one](https://github.com/QuestionableM/SM-DLL-Injector) is **required**.  

After installing an injector, simply add the `VoxelTerrain.dll` mod to your `DLLModules` directory as written in the injector's description.

## Technical Details

### Lua API

This DLL provides a large custom Lua API for the voxel terrain system.  
As described above, this includes, but is not limited to, features to erase or place terrain in various shapes and ways,  
runtime importing and voxelization of 3D models, terrain restriction systems and more.  
**All of these Lua API features are documented in the [ScrapMechanicTools](https://scrapmechanictools.com/customApis/VoxelTerrain/Info) website.**

### API Usage Examples

For examples of how to use the voxel terrain scripting API, **feel free to dig through the scripts and files of the [VoxelTerrain Showcase](https://steamcommunity.com/sharedfiles/filedetails/?id=3575301562) mod!**
