# Kiwi Engine Demo  
This project is a combination of two separate projects: a game engine (called the Kiwi Engine) and a game/tech demo made using the engine. Both the demo and the engine were made over a period of 8 months, or around 500 hours, as a university capstone project.  
  
This tech demo focuses mainly on procedural generation of a voxel world and uses the engine to quickly create the terrain meshes and for communication between the player and the terrain and a few simple NPCs. Like the engine, the demo game was made from scratch in C++ using Visual Studio and includes no external libraries except for the STL library and the engine itself.  
  
## Requirements 
To run the demo, a minimum screen resolution of 1600x900 is required, along with a DirectX 11 capable GPU.  
  
## Controls  
Esc: Free the mouse  
WASD: Movement  
Space: Jump  
L-Shift: Run
Tab: Toggle between no-clip (free camera) and regular mode  
C: Toggle between block-edit and combat modes  
Right-Mouse: Fire projectile (combat mode), or remove voxel (block-edit mode)
  
## Building the demo
To build the demo, the static library and source files of Engine, as well as the DirectX 11 SDK, must be linked. The engine can be found [here](https://github.com/nicoawalker/Kiwi-Engine-2016 "Kiwi Engine 2016").  
To do this in Visual Studio 2015, simply add the paths to the include and library directories in the solution's "VC++ Directories" property.  
