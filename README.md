# Voxels

## Current Features
 - Infinite world generation
 - Able to have multiple different mesh types (block, mesh, x mesh, ect)
 - Full texture atlas stitched together at start up that works regardless of texture size for any texture
 - Full world collision
 - Block Breaking
 - Vague beginnings of an inventory system
 - Entity Component System
 - Multi threaded generation and meshing
 - Ambient Occlusion
 - Fences that changes what segments are in use based on their surroundings hooray

## Roadmap
### Engine Architecture
 - [ ] Do i really need my event bus? - investigate different implementations
 - [x] Block Json data will solve the face problem
 - [ ] Finish implementing "special blocks" like doors and actuall reading / setting the rotation and other data in the voxel
 - [ ] Add segment logic for things like fences
 - [ ] An entity factory for easy spawning of mobs, objects, ect
 - [ ] Frustrum culling
 - [ ] Greedy meshing
 - [ ] Serialization (i am slightly dredding this one)
### World
 - [ ] Implement Trees with exciting looking branches and leaves (not blocky)
 - [ ] Multiple biomes
 - [ ] Caves (really the entire generation system its very basic right now)
 - [ ] Water
 - [ ] Strcuture gen
### Visual / Ambiance (Vibe i guess?)
 - [ ] Shadow maps
 - [ ] Ray traced shadows maybe
 - [ ] Sounds
 - [ ] View fog
 - [ ] God rays
 - [ ] Day night cycle 
### Playability
 - [ ] When breaking an object have an entity of it spawn rather that just appearing in the inventory
 - [ ] Step up onto slabs ect
 - [ ] Figure out the crafting system (probably not shaped)
 - [ ] Proper UI
 - [ ] Homescreen play button, create world and switch between worlds after serialization
 - [ ] Survival health damage ect
 - [ ] Fish, Animals, ect
 - [ ] Bauble / Accessory system

## Current Issues
 - [x] Ray casts don't work properly before the player has moved for the first time
 - [x] Blocks are given the same texture for every face
 - [ ] Loads of vulkan errors if you exit the program via the window rather than via the command line
 - [ ] Can place blocks at head height and get stuck in them
 - [ ] if moving and placing player can be accelerated in the direction rather than either not having the block paced or have the place put on top of it
 - [x] Face culling when they shouldn't be (fences) (fixed i think)
 - [ ] If using VK_Immediate_mode movement slows significantly
 - [ ] Hit boxes don't entirely represent the box position. Mostly due to 0.5 to center something (e.g fence) on a block
