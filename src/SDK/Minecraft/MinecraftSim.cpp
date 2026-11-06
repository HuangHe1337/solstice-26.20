//
// Created by vastrakai on 6/25/2024.
//

#include "MinecraftSim.hpp"

#include <libhat/Access.hpp>
#include <SDK/OffsetProvider.hpp>
#include <SDK/Minecraft/MinecraftGame.hpp>
#include <Utils/MemUtils.hpp>
#include <Windows.h>

static bool isMemoryWritable(void* ptr, size_t size = sizeof(float))
{
    if (!ptr || reinterpret_cast<uintptr_t>(ptr) < 0x10000) return false;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if ((mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) == 0) return false;
    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    return true;
}

static bool isMemoryReadable(const void* ptr, size_t size = sizeof(float))
{
    if (!ptr || reinterpret_cast<uintptr_t>(ptr) < 0x10000) return false;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    return true;
}

Simulation* MinecraftSim::getGameSim()
{
    if (this && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(this))) {
        __try {
            auto sim = hat::member_at<Simulation*>(this, OffsetProvider::MinecraftSim_mGameSim);
            if (sim && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(sim))) return sim;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return nullptr;
}

Simulation* MinecraftSim::getRenderSim()
{
    if (this && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(this))) {
        __try {
            auto sim = hat::member_at<Simulation*>(this, OffsetProvider::MinecraftSim_mRenderSim);
            if (sim && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(sim))) return sim;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return nullptr;
}

void MinecraftSim::setSimTimer(float timer)
{
    auto applyTimer = [timer](Simulation* sim) {
        if (!sim || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(sim)) || !isMemoryWritable(&sim->mTimer)) return;
        __try {
            sim->mTimer = timer;
            sim->mTimerMultiplier = timer / 20.f;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    };

    if (this && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(this))) {
        __try {
            applyTimer(getGameSim());
            applyTimer(getRenderSim());
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}

void MinecraftSim::setSimSpeed(float speed)
{
    auto applySpeed = [speed](Simulation* sim) {
        if (!sim || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(sim)) || !isMemoryWritable(&sim->mTimerMultiplier)) return;
        __try {
            sim->mTimerMultiplier = speed;
            sim->mTimer = 20.f * speed;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    };

    if (this && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(this))) {
        __try {
            applySpeed(getGameSim());
            applySpeed(getRenderSim());
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}

float MinecraftSim::getSimTimer()
{
    auto game = getGameSim();
    if (game && isMemoryReadable(&game->mTimer))
    {
        __try {
            return game->mTimer;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 20.f;
}

float MinecraftSim::getSimSpeed()
{
    auto game = getGameSim();
    if (game && isMemoryReadable(&game->mTimerMultiplier))
    {
        __try {
            return game->mTimerMultiplier;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 1.f;
}

GameSession* MinecraftSim::getGameSession()
{
    if (!this || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(this))) return nullptr;
    __try {
        auto session = hat::member_at<GameSession*>(this, OffsetProvider::MinecraftSim_mGameSession);
        if (session && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(session))) return session;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return nullptr;
}
