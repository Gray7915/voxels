#include "World/Generation/ChunkGenWorkerPool.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "Util/Types.hpp"
namespace lve
{
    ChunkGenWorkerPool::ChunkGenWorkerPool(size_t threadCount) {
        for (size_t i = 0; i < threadCount; i++)
            workers.emplace_back([this] {
                workerLoop();
            });
    }

    ChunkGenWorkerPool::~ChunkGenWorkerPool() {
        running = false;
        jobQueue.shutdown();
        for (auto &t : workers)
            if (t.joinable())
                t.join();
    }

    void ChunkGenWorkerPool::workerLoop() {
        while (running) {
            GenJob job;
            if (!jobQueue.wait_and_pop(job))
                break;
            VoxelData data = generateTerrain(job);
            resultQueue.push({job.chunkCoord, std::move(data)});
        }
    }

    VoxelData ChunkGenWorkerPool::generateTerrain(GenJob job) {
        VoxelData data{};
        data.allocate();
        runTerrainPass(job, data);
        runFloraPass(job, data);
        return data;
    }

    float ChunkGenWorkerPool::getNoiseValue(GenJob job, ivec3 coord) {
        vec2 pos = vec2(coord.x, coord.z);

        // Large scale biome selector
        float large = job.largeGroundNoise->sample(pos * 0.0015f, job.worldSeed);
        float mountainMask = (large + 1.0f) * 0.5f;

        // Sharper transition: narrow band between plains and mountains
        mountainMask = glm::smoothstep(0.52f, 0.72f, mountainMask);

        // Pow curve makes mountains rarer but taller once they start appearing
        mountainMask = glm::pow(mountainMask, 1.8f);

        float medium = job.groundNoise->sample(pos * 0.005f, job.worldSeed);
        float detail = job.groundNoise2->sample(pos * 0.018f, job.worldSeed);
        float mountain = job.groundNoise3->sample(pos * 0.002f, job.worldSeed);

        // Remap all to [0, 1]
        float mediumN = (medium + 1.0f) * 0.5f;
        float detailN = (detail + 1.0f) * 0.5f;
        float mountainN = (mountain + 1.0f) * 0.5f;

        float baseHeight = VoxelData::HEIGHT * 0.35f;

        // Plains: gentle rolling, detail adds small bumps
        float plainsHeight = mediumN * 18.0f + detailN * 3.0f;

        // Mountains: dedicated mountain noise drives the large shape,
        // medium adds ridge variation, detail adds rocky surface texture
        float rawMountain = glm::pow(mountainN, 2.2f); // power curve = sharp peaks, flat valleys
        float mountainHeight = rawMountain * 110.0f + mediumN * 22.0f + detailN * 5.0f;

        float terrainHeight = baseHeight + glm::mix(plainsHeight, mountainHeight, mountainMask);
        return terrainHeight;
    }

    bool ChunkGenWorkerPool::isTreePosition(GenJob job, ivec3 coord) {
        vec2 pos = vec2(coord.x, coord.z);

        auto sampleCell = [&](vec2 p) {
            float c = job.treeNoise->sample(p * 0.2f, job.worldSeed);
            c = (c + 1.0f) * 0.5f;
            return 1.0f - c;
        };

        float here = sampleCell(pos);

        if (here < 0.7f)
            return false; // below minimum threshold entirely

        // Must be the local maximum in a small neighbourhood
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                if (dx == 0 && dz == 0)
                    continue;
                if (sampleCell(pos + vec2(dx, dz)) >= here)
                    return false; // a neighbour is equal or higher, not the peak
            }
        }

        // Reject on mountains
        float large = job.largeGroundNoise->sample(pos * 0.0013f, job.worldSeed);
        float mountainMask = (large + 1.0f) * 0.5f;
        mountainMask = glm::smoothstep(0.52f, 0.72f, mountainMask);
        mountainMask = glm::pow(mountainMask, 1.8f);
        if (mountainMask > 0.4f)
            return false;

        return true;
    }

    float ChunkGenWorkerPool::getDensity(GenJob job, ivec3 worldCoord, float surfaceHeight) {
        float distFromSurface = surfaceHeight - (float)worldCoord.y;

        if (distFromSurface < 0.0f)
            return distFromSurface;

        vec3 pos = vec3(worldCoord);

        // Squash Y so tunnels are wider than they are tall
        vec3 scaledPos = vec3(pos.x, pos.y * 2.5f, pos.z);

        float caveA = glm::abs(job.caveNoise->sample3d(scaledPos * 0.04f, job.worldSeed));
        float caveB = glm::abs(job.caveNoise2->sample3d(scaledPos * 0.04f, job.worldSeed));
        float tunnels = glm::min(caveA, caveB) - 0.05f;

        float pocketNoise = glm::abs(job.caveNoise->sample3d(scaledPos * 0.007f, job.worldSeed));
        float pockets = pocketNoise - 0.10f;

        float density3d = glm::min(tunnels, pockets);

        float surfaceGradient = glm::clamp(distFromSurface / 12.0f, 0.0f, 1.0f);
        float bottomGradient = glm::clamp((float)worldCoord.y / 12.0f, 0.0f, 1.0f);
        float gradient = glm::max(1.0f - surfaceGradient, 0.0f) + glm::max(1.0f - bottomGradient, 0.0f);

        float caveWeight = glm::smoothstep(0.0f, 20.0f, distFromSurface);

        return gradient + density3d * caveWeight;
    }

    void ChunkGenWorkerPool::runTerrainPass(GenJob job, VoxelData &data) {
        ivec3 chunkCoord = job.chunkCoord;
        ivec2 worldOrigin = glm::ivec2(chunkCoord.x, chunkCoord.z) * VoxelData::WIDTH;

        int surfaceHeights[VoxelData::WIDTH][VoxelData::DEPTH];
        for (int x = 0; x < VoxelData::WIDTH; x++)
            for (int z = 0; z < VoxelData::DEPTH; z++) {
                vec2 worldPos = vec2(x + worldOrigin.x, z + worldOrigin.y);
                surfaceHeights[x][z] = glm::clamp((int)getNoiseValue(job, ivec3(worldPos.x, 0, worldPos.y)), 0, VoxelData::HEIGHT - 1);
            }

        for (int x = 0; x < VoxelData::WIDTH; x++)
            for (int z = 0; z < VoxelData::DEPTH; z++) {
                vec2 worldPos = vec2(x + worldOrigin.x, z + worldOrigin.y);
                int surfaceHeight = surfaceHeights[x][z];
                for (int y = 0; y < VoxelData::HEIGHT; y++) {
                    float density = getDensity(job, ivec3(worldPos.x, y, worldPos.y), (float)surfaceHeight);
                    if (density < 0.0f) {
                        data.set(x, y, z, BlockRegistry::Get().GetBlockIDByName("air"));
                    } else {
                        int depth = surfaceHeight - y;
                        if (depth == 0)
                            data.set(x, y, z, BlockRegistry::Get().GetBlockIDByName("grass"));
                        else if (depth < 4)
                            data.set(x, y, z, BlockRegistry::Get().GetBlockIDByName("dirt"));
                        else
                            data.set(x, y, z, BlockRegistry::Get().GetBlockIDByName("stone"));
                    }
                }
            }
    }

    void ChunkGenWorkerPool::runCavePass(GenJob job, VoxelData &data) {
        /*
        caves already in get density thingie, this is for when i have a full proper cave pass
        */
    }

    void ChunkGenWorkerPool::runFloraPass(GenJob job, VoxelData &data) {
        ivec3 chunkCoord = job.chunkCoord;
        ivec2 worldOrigin = glm::ivec2(chunkCoord.x, chunkCoord.z) * VoxelData::WIDTH;

        // Sample a ring around this chunk so border trees get their
        // leaves/trunk written into us even if the neighbour isn't loaded
        constexpr int BORDER = 4; // max tree radius

        for (int x = -BORDER; x < VoxelData::WIDTH + BORDER; x++)
            for (int z = -BORDER; z < VoxelData::DEPTH + BORDER; z++) {
                vec2 worldPos = vec2(x + worldOrigin.x, z + worldOrigin.y);

                if (!isTreePosition(job, ivec3(worldPos.x, 0, worldPos.y)))
                    continue;

                // Find surface height at this world position (no chunk needed)
                int surfaceY = (int)getNoiseValue(job, ivec3(worldPos.x, 0, worldPos.y));
                surfaceY = glm::clamp(surfaceY, 0, VoxelData::HEIGHT - 1);

                // Place tree — writes only clamp to THIS chunk's bounds
                placeTree(data, x, surfaceY + 1, z);
            }
    }

    void ChunkGenWorkerPool::placeTree(VoxelData &data, int localX, int y, int localZ) {
        auto setBlock = [&](int bx, int by, int bz, BlockId block) {
            // Clamp to this chunk only — neighbour chunks handle their own writes
            if (bx < 0 || bx >= VoxelData::WIDTH)
                return;
            if (bz < 0 || bz >= VoxelData::DEPTH)
                return;
            if (by < 0 || by >= VoxelData::HEIGHT)
                return;
            data.set(bx, by, bz, block);
        };

        for (int i = 0; i < 5; i++)
            setBlock(localX, y + i, localZ, BlockRegistry::Get().GetBlockIDByName("oak_logs"));

        for (int dx = -2; dx <= 2; dx++)
            for (int dz = -2; dz <= 2; dz++)
                for (int dy = 3; dy <= 5; dy++)
                    setBlock(localX + dx, y + dy, localZ + dz, BlockRegistry::Get().GetBlockIDByName("oak_leaves"));
    }

    void ChunkGenWorkerPool::setIfInBounds(VoxelData &data, int x, int y, int z, BlockId block) {
        if (x < 0 || x >= VoxelData::WIDTH)
            return;
        if (z < 0 || z >= VoxelData::DEPTH)
            return;
        if (y < 0 || y >= VoxelData::HEIGHT)
            return;
        data.set(x, y, z, block);
    }
} // namespace lve