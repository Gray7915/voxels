#include "World/Systems/ChunkGenerationSystem.hpp"
#include "Util/Types.hpp"
#include "World/Area.hpp"
#include "World/Generation/noise.hpp"

namespace lve
{
    ChunkGenerationSystem::ChunkGenerationSystem(Area &area)
        : area{area}, worldSeed{area.getWorldSeed()}, groundNoise{hashSeed(worldSeed++), 5, 0.5f}, groundNoise2{hashSeed(worldSeed++), 3, 0.5f},
          groundNoise3{hashSeed(worldSeed++), 5, 0.5f},     // mountain shape
          largeGroundNoise{hashSeed(worldSeed++), 4, 0.5f}, // biome selector
          modifierNoise{hashSeed(worldSeed++), 3, 0.5f}, treeNoise{hashSeed(worldSeed++), 1, 0.5}, caveNoise{hashSeed(worldSeed++), 4, 0.5f}, caveNoise2{hashSeed(worldSeed++), 1, 0.5f} {
        // Ridged on mountain shape = natural sharp peaks and ridgelines
        groundNoise3.fnl.SetFractalType(FastNoiseLite::FractalType_Ridged);

        treeNoise.fnl.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        treeNoise.fnl.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
        treeNoise.fnl.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
        treeNoise.fnl.SetFractalType(FastNoiseLite::FractalType_None);

        caveNoise.fnl.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
        caveNoise.fnl.SetDomainWarpAmp(30.0f);
        caveNoise.fnl.SetFractalType(FastNoiseLite::FractalType_None);
        caveNoise2.fnl.SetFractalType(FastNoiseLite::FractalType_None);
    }
    ChunkGenerationSystem::~ChunkGenerationSystem() = default;

    void ChunkGenerationSystem::update() {
        for (auto &[coord, chunk] : area.AllChunks()) {
            if (chunk->chunkState == ChunkState::Unloaded) {
                chunk->chunkState = ChunkState::QueuedForGeneration;
                requestGeneration(coord);
            }
        }

        GenResult result;
        int budget = 4;
        while (budget-- > 0 && genPool.tryGetResult(result)) {
            Chunk *chunk = area.getChunk(result.chunkCoord);
            if (!chunk)
                continue;

            chunk->setVoxelData(std::move(result.data));
            chunk->chunkState = ChunkState::Generated;
            area.markNeighborChunksDirty(result.chunkCoord);
        }
    }

    void ChunkGenerationSystem::requestGeneration(ivec3 coord) {
        genPool.submit({.chunkCoord = coord,
                        .worldSeed = area.getWorldSeed(),
                        .groundNoise = &groundNoise,
                        .groundNoise2 = &groundNoise2,
                        .groundNoise3 = &groundNoise3,
                        .largeGroundNoise = &largeGroundNoise,
                        .modifierNoise = &modifierNoise,
                        .treeNoise = &treeNoise,
                        .caveNoise = &caveNoise,
                        .caveNoise2 = &caveNoise2});
        // std::cout << "world seed in request gen " << area.getWorldSeed() << '\n';
    }
} // namespace lve
