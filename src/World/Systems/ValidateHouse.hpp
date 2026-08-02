#pragma once

namespace WorldSystems
{
    class ValidateHosuse
    {
        /*
            Method One 
            struct {
                List of chunks the house sits in
                List of voxels making up the house
                Position of the bottom left most voxel
                ID of occupying NPC
            };

            When do we do a house check?
            - When the chunk is loaded in the first half of the day -> one check
            - When the player clicks the to be implemented "check house" button
            - When a block is broken we check if a house is in the chunk the block was broken in. If it is we

            -> I don't like method one, method one bad


            Method Two

            struct room?{
                Room Id
                List of bounding boxes that represent the house
                Set of chunks that when combined the boxes fall in
                bool dirty
            };

            ID of occupying NPC lives in the 8 usage bits of the chair or chair like object in the house. if there are multiple chairs each on holds the NPCs ID. (one npc per house / room)


            When do we do a house check?
            - When the chunk is loaded in the first half of the day -> one check, if the check is accepted and there is an npc that needs to move in they will
            - When the player clicks the to be implemented "check house" button
            - When a block is broken/added we check if any of the list of houses sit at all within the current chunk
              If a house does sit in a given chunk we see if the coordinate of the broken block falls within the bounding box.
              If it does we add a dirty flag to it and update it when either of the first two conditions are met (chunk loaded in morning or check house button).
              
            - If a house is invalid we delete it from the list and the NPC stays around until they are killed by an enemy or the chunk they are in is unloaded.
            
            - Using the bounding boxes of the rooms we can create a bounding box around all of them representing a town? This could lower spawn rates and
              if the npcs are around other npcs that they like prices could go down or up if they aren't happy?

       */
    };
}