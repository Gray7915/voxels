### Voxels

## Current Features
 - Infinite world generation
 - Able to have multiple different mesh types (block, mesh, x mesh, ect)
 - Full texture atlas stitched together at start up that works regardless of texture size for any texture
 - Full world collision
 - Block Breaking
 - Vague beginnings of an inventory system
 - Entity Component System
 - Multi threaded generation and meshing

## Roadmap
 - [ ] Implement Trees with exiting looking branches and leaves (not blocky)
 - [ ] An entity factory for easy spawning of mobs, objects, ect
 - [ ] Multiple biomes
 - [ ] Caves (really the entire generation system its very basic right now)
 - [ ] Shadow maps
 - [ ] Sounds
 - [ ] AABB refactor
 - [ ] When breaking an object have an entity of it spawn rather that just appearing in the inventory
 - [ ] Block Json data will solve the face problem
 - [ ] Water
 - [ ] Fish, Animals, ect
 - [ ] Bauble / Accessory system

## Current Issues
 - [ ] Ray casts don't work properly before the player has moved for the first time
 - [ ] Blocks are given the same texture for every face
 - [ ] Loads of vulkan errors if you exit the program via the window rather than via the command line
 - [ ] Can place blocks at head height and get stuck in them (easy aabb fix)
 - [ ] if moving and placing player can be accelerated in the direction rather than either not having the block paced or have the place put on top of it
