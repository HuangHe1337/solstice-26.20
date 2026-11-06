//
// Created by vastrakai on 7/7/2024.
//

#include "BlockESP.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/BlockChangedEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerActionPacket.hpp>
#include <SDK/Minecraft/World/BlockLegacy.hpp>
#include <SDK/Minecraft/World/BlockSource.hpp>
#include <Utils/MiscUtils/BlockUtils.hpp>
#include <Utils/MiscUtils/MathUtils.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>

static std::mutex blockMutex = {};

static Block* safeGetBlock(BlockSource* bs, const glm::ivec3& pos)
{
    __try {
        return bs->getBlock(pos);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

constexpr int REDSTONE_ORE = 73;
constexpr int REDSTONE_ORE_LIT = 74;
constexpr int DIAMOND_ORE = 56;
constexpr int EMERALD_ORE = 129;
constexpr int GOLD_ORE = 14;
constexpr int IRON_ORE = 15;
constexpr int LAPIS_ORE = 21;
constexpr int COAL_ORE = 16;
constexpr int BARRIER = 416;
constexpr int DEEPSLATE_REDSTONE_ORE = 658;
constexpr int DEEPSLATE_LIT_REDSTONE_ORE = 659;
constexpr int DEEPSLATE_DIAMOND_ORE = 660;
constexpr int DEEPSLATE_EMERALD_ORE = 662;
constexpr int DEEPSLATE_GOLD_ORE = 657;
constexpr int DEEPSLATE_IRON_ORE = 656;
constexpr int DEEPSLATE_LAPIS_ORE = 655;
constexpr int DEEPSLATE_COAL_ORE = 661;
constexpr int PORTAL = 90;

constexpr int CHEST = 54;
constexpr int ENDER_CHEST = 130;
constexpr int TRAPPED_CHEST = 146;
constexpr int BARREL = 458;

static const std::unordered_map<int, ImColor> blockColors = {
    { 73, ImColor(1.f, 0.f, 0.f, 1.f) },
    { 74, ImColor(1.f, 0.f, 0.f, 1.f) },
    { 658, ImColor(1.f, 0.f, 0.f, 1.f) },
    { 659, ImColor(1.f, 0.f, 0.f, 1.f) },
    { 56, ImColor(0.f, 1.f, 1.f, 1.f) },
    { 660, ImColor(0.f, 1.f, 1.f, 1.f) },
    { 129, ImColor(0.f, 1.f, 0.f, 1.f) },
    { 662, ImColor(0.f, 1.f, 0.f, 1.f) },
    { 14, ImColor(1.f, 1.f, 0.f, 1.f) },
    { 657, ImColor(1.f, 1.f, 0.f, 1.f) },
    { 15, ImColor(1.f, 0.5f, 0.f, 1.f) },
    { 656, ImColor(1.f, 0.5f, 0.f, 1.f) },
    { 21, ImColor(0.f, 0.f, 1.f, 1.f) },
    { 655, ImColor(0.f, 0.f, 1.f, 1.f) },
    { 16, ImColor(0.2f, 0.2f, 0.2f, 1.f) },
    { 661, ImColor(0.2f, 0.2f, 0.2f, 1.f) },
    { 90, ImColor(0.5f, 0.f, 0.5f, 1.f) },
    { 130, ImColor(0.5f, 0.f, 0.5f, 1.f) },
    { 54, ImColor(255, 165, 0) },
    { 146, ImColor(255, 165, 0) },
    { 458, ImColor(255, 165, 0) }
};

static bool isValidBlock(int id)
{
    return blockColors.contains(id);
}

static ImColor getColorFromId(int id)
{
    auto it = blockColors.find(id);
    if (it != blockColors.end())
    {
        return it->second;
    }
    return ImColor(1.f, 1.f, 1.f, 1.f);
}

void BlockESP::moveToNext()
{
}

void BlockESP::tryProcessSub(bool& processed, ChunkPos currentChunkPos, int subChunkIndex)
{
    processed = true;
}

bool BlockESP::processSub(ChunkPos processChunk, int index)
{
    return true;
}

void BlockESP::reset()
{
    std::lock_guard<std::mutex> lock(blockMutex);

    ClientInstance* ci = ClientInstance::get();
    Actor* player = ci ? ci->getLocalPlayer() : nullptr;
    mSearchStart = NOW;
    mFoundBlocks.clear();
    mStepsCount = 0;
    mSteps = 1;
    mDirectionIndex = 0;
    mSubChunkIndex = 0;
    mCurrentScanY = -9999;
    if (!player || !player->getPos()) return;

    mSearchCenter = ChunkPos(*player->getPos());
    mCurrentChunkPos = mSearchCenter;
}

void BlockESP::onEnable()
{
    gFeatureManager->mDispatcher->listen<RenderEvent, &BlockESP::onRenderEvent, nes::event_priority::VERY_FIRST>(this);
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &BlockESP::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<BlockChangedEvent, &BlockESP::onBlockChangedEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketInEvent, &BlockESP::onPacketInEvent>(this);
    reset();
}

void BlockESP::onDisable()
{
    gFeatureManager->mDispatcher->deafen<RenderEvent, &BlockESP::onRenderEvent>(this);
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &BlockESP::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<BlockChangedEvent, &BlockESP::onBlockChangedEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &BlockESP::onPacketInEvent>(this);
    reset();
}

std::vector<int> BlockESP::getEnabledBlocks()
{
    std::vector<int> enabledBlocks = {};

    if (mEmerald.mValue)
    {
        enabledBlocks.push_back(EMERALD_ORE);
        enabledBlocks.push_back(DEEPSLATE_EMERALD_ORE);
    }
    if (mDiamond.mValue)
    {
        enabledBlocks.push_back(DIAMOND_ORE);
        enabledBlocks.push_back(DEEPSLATE_DIAMOND_ORE);
    }
    if (mGold.mValue)
    {
        enabledBlocks.push_back(GOLD_ORE);
        enabledBlocks.push_back(DEEPSLATE_GOLD_ORE);
    }
    if (mIron.mValue)
    {
        enabledBlocks.push_back(IRON_ORE);
        enabledBlocks.push_back(DEEPSLATE_IRON_ORE);
    }
    if (mCoal.mValue)
    {
        enabledBlocks.push_back(COAL_ORE);
        enabledBlocks.push_back(DEEPSLATE_COAL_ORE);
    }
    if (mRedstone.mValue)
    {
        enabledBlocks.push_back(REDSTONE_ORE);
        enabledBlocks.push_back(REDSTONE_ORE_LIT);
        enabledBlocks.push_back(DEEPSLATE_REDSTONE_ORE);
        enabledBlocks.push_back(DEEPSLATE_LIT_REDSTONE_ORE);
    }
    if (mLapis.mValue)
    {
        enabledBlocks.push_back(LAPIS_ORE);
        enabledBlocks.push_back(DEEPSLATE_LAPIS_ORE);
    }
    if (mPortal.mValue)
    {
        enabledBlocks.push_back(PORTAL);
    }
    if (mChests.mValue)
    {
        enabledBlocks.push_back(CHEST);
        enabledBlocks.push_back(ENDER_CHEST);
        enabledBlocks.push_back(TRAPPED_CHEST);
        enabledBlocks.push_back(BARREL);
    }

    return enabledBlocks;
}

void BlockESP::onBlockChangedEvent(BlockChangedEvent& event)
{
    if (!ClientInstance::get() || !ClientInstance::get()->getLevelRenderer()) {
        reset();
        return;
    }
    std::lock_guard<std::mutex> lock(blockMutex);

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player || !player->getPos()) return;

    BlockPos pos = event.mBlockPos;
    if (glm::distance(glm::vec3(pos.x, pos.y, pos.z), *player->getPos()) > mRadius.mValue) return;

    auto enabledBlocks = getEnabledBlocks();
    if (event.mNewBlock && event.mNewBlock->mLegacy) {
        int id = event.mNewBlock->mLegacy->getBlockId();
        if (id != 0 && isValidBlock(id) && std::ranges::find(enabledBlocks, id) != enabledBlocks.end()) {
            if (!mOnlyExposedOres.mValue || BlockUtils::getExposedFace(glm::ivec3(pos.x, pos.y, pos.z)) != -1) {
                mFoundBlocks[pos] = { event.mNewBlock, AABB(pos, glm::vec3(1.f, 1.f, 1.f)), getColorFromId(id) };
                return;
            }
        }
    }
    mFoundBlocks.erase(pos);
}

void BlockESP::onBaseTickEvent(BaseTickEvent& event)
{
    if (!ClientInstance::get() || !ClientInstance::get()->getLevelRenderer()) {
        reset();
        return;
    }
    std::lock_guard<std::mutex> lock(blockMutex);

    static uint64_t lastUpdate = 0;
    uint64_t freq = static_cast<uint64_t>(mUpdateFrequency.mValue * 50.f);
    uint64_t now = NOW;

    if (lastUpdate + freq > now) return;
    lastUpdate = now;

    auto ci = ClientInstance::get();
    auto player = ci ? ci->getLocalPlayer() : nullptr;
    if (!player) return;
    auto blockSource = ci->getBlockSource();
    if (!blockSource) return;

    glm::vec3* pPos = player->getPos();
    if (!pPos) return;

    BlockPos playerPos(*pPos);
    int radius = std::min(static_cast<int>(mRadius.mValue), 32);
    int buildDepth = static_cast<int>(blockSource->getBuildDepth());
    int buildHeight = static_cast<int>(blockSource->getBuildHeight());
    int minY = std::max(buildDepth, playerPos.y - radius);
    int maxY = std::min(buildHeight, playerPos.y + radius);

    if (mCurrentScanY < minY || mCurrentScanY > maxY) {
        mCurrentScanY = minY;
    }

    auto enabledBlocks = getEnabledBlocks();
    if (enabledBlocks.empty()) {
        mFoundBlocks.clear();
        return;
    }

    // Scan vertical slices per tick safely and incrementally
    int yLayersPerTick = 4;
    for (int layer = 0; layer < yLayersPerTick && mCurrentScanY <= maxY; layer++, mCurrentScanY++) {
        int y = mCurrentScanY;
        int r2 = radius * radius;
        for (int dx = -radius; dx <= radius; dx++) {
            for (int dz = -radius; dz <= radius; dz++) {
                if (dx * dx + dz * dz > r2) continue;
                BlockPos pos(playerPos.x + dx, y, playerPos.z + dz);
                Block* block = safeGetBlock(blockSource, glm::ivec3(pos.x, pos.y, pos.z));
                if (!block || !block->mLegacy) {
                    mFoundBlocks.erase(pos);
                    continue;
                }
                int id = block->mLegacy->getBlockId();
                if (id == 0 || !isValidBlock(id)) {
                    mFoundBlocks.erase(pos);
                    continue;
                }
                if (std::ranges::find(enabledBlocks, id) == enabledBlocks.end()) {
                    mFoundBlocks.erase(pos);
                    continue;
                }
                if (mOnlyExposedOres.mValue && BlockUtils::getExposedFace(glm::ivec3(pos.x, pos.y, pos.z)) == -1) {
                    mFoundBlocks.erase(pos);
                    continue;
                }

                mFoundBlocks[pos] = { block, AABB(pos, glm::vec3(1.f, 1.f, 1.f)), getColorFromId(id) };
            }
        }
    }

    // When full vertical pass finishes, cleanup blocks that are out of radius
    if (mCurrentScanY > maxY) {
        mCurrentScanY = minY;
        float maxDistSq = static_cast<float>((radius + 4) * (radius + 4));
        glm::vec3 center = *pPos;
        for (auto it = mFoundBlocks.begin(); it != mFoundBlocks.end(); ) {
            float dx = static_cast<float>(it->first.x) - center.x;
            float dy = static_cast<float>(it->first.y) - center.y;
            float dz = static_cast<float>(it->first.z) - center.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq > maxDistSq) {
                it = mFoundBlocks.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void BlockESP::onPacketInEvent(PacketInEvent& event)
{
    if (!ClientInstance::get() || !ClientInstance::get()->getLevelRenderer()) {
        reset();
        return;
    }

    if (event.mPacket->getId() == PacketID::ChangeDimension)
    {
        reset();
    }
    else if (event.mPacket->getId() == PacketID::PlayerAction)
    {
        auto packet = event.getPacket<PlayerActionPacket>();
        if (packet->mAction == PlayerActionType::Respawn) reset();
    }
}

void BlockESP::onRenderEvent(RenderEvent& event)
{
    if (!ClientInstance::get() || !ClientInstance::get()->getLevelRenderer()) {
        reset();
        return;
    }

    if (ClientInstance::get()->getMouseGrabbed()) return;

    std::lock_guard<std::mutex> lock(blockMutex);

    auto drawList = ImGui::GetBackgroundDrawList();

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player || !player->getPos())
    {
        reset();
        return;
    }

    glm::vec3 playerPos = *player->getPos();

    if (mRenderCurrentChunk.mValue)
    {
        ChunkPos currentChunkPos(*player->getPos());
        glm::vec3 pos = glm::vec3(currentChunkPos.x * 16.f, playerPos.y - 1.f, currentChunkPos.y * 16.f);

        AABB chunkAABB = AABB(pos, glm::vec3(16.f, 1.f, 16.f));
        std::vector<ImVec2> chunkPoints = MathUtils::getImBoxPoints(chunkAABB);

        if (!chunkPoints.empty()) {
            if (mRenderMode.mValue == BlockRenderMode::Both || mRenderMode.mValue == BlockRenderMode::Outline) {
                drawList->AddPolyline(chunkPoints.data(), chunkPoints.size(), ImColor(1.f, 1.f, 1.f), 0, 2.0f);
            }
            if (mRenderMode.mValue == BlockRenderMode::Both || mRenderMode.mValue == BlockRenderMode::Filled) {
                drawList->AddConvexPolyFilled(chunkPoints.data(), chunkPoints.size(), ImColor(1.f, 1.f, 1.f, 0.25f));
            }
        }
    }

    auto enabled = getEnabledBlocks();

    for (auto& [pos, block] : mFoundBlocks)
    {
        if (glm::distance(glm::vec3(pos.x, pos.y, pos.z), playerPos) > mRadius.mValue) continue;
        if (!block.block || !block.block->mLegacy) continue;
        if (std::ranges::find(enabled, block.block->mLegacy->getBlockId()) == enabled.end()) continue;

        ImColor& color = block.color;
        AABB& blockAABB = block.aabb;
        std::vector<ImVec2> imPoints = MathUtils::getImBoxPoints(blockAABB);
        if (imPoints.empty()) continue;

        if (mRenderMode.mValue == BlockRenderMode::Both || mRenderMode.mValue == BlockRenderMode::Outline) {
            drawList->AddPolyline(imPoints.data(), imPoints.size(), color, 0, 2.0f);
        }
        if (mRenderMode.mValue == BlockRenderMode::Both || mRenderMode.mValue == BlockRenderMode::Filled) {
            drawList->AddConvexPolyFilled(imPoints.data(), imPoints.size(), ImColor(color.Value.x, color.Value.y, color.Value.z, 0.25f));
        }
    }
}
