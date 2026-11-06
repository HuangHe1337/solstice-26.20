//
// Created by vastrakai on 7/1/2024.
//

#include "Timer.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/RenderEvent.hpp>
#include <Hook/Hooks/MiscHooks/SimulationTickHook.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftGame.hpp>
#include <SDK/Minecraft/MinecraftSim.hpp>
#include <Utils/MemUtils.hpp>

static void applyTimer(float speed)
{
    // 1. Direct simulation tick override (active each tick in Simulation::tick)
    SimulationTickHook::sCustomTimer.store(speed);
    SimulationTickHook::sOverrideTimer.store(speed != 20.0f);

    // 2. Direct update to last active simulation
    if (SimulationTickHook::sLastSim && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(SimulationTickHook::sLastSim))) {
        __try {
            SimulationTickHook::sLastSim->mTimer = speed;
            SimulationTickHook::sLastSim->mTimerMultiplier = speed / 20.f;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // 3. If ClientInstance has a valid MinecraftSim
    auto ci = ClientInstance::get();
    if (ci) {
        auto sim = ci->getMinecraftSim();
        if (sim) {
            sim->setSimTimer(speed);
        }
    }

    // 4. Also directly apply to MinecraftGame's client simulation (0xD98 & 0xDA0 in 1.26.20)
    auto mc = MinecraftGame::getInstance();
    if (mc && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(mc))) {
        __try {
            auto gameSim = *reinterpret_cast<Simulation**>((uintptr_t)mc + 0xD98);
            if (gameSim && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(gameSim))) {
                gameSim->mTimer = speed;
                gameSim->mTimerMultiplier = speed / 20.f;
            }
            auto renderSim = *reinterpret_cast<Simulation**>((uintptr_t)mc + 0xDA0);
            if (renderSim && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(renderSim))) {
                renderSim->mTimer = speed;
                renderSim->mTimerMultiplier = speed / 20.f;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}

void Timer::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Timer::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &Timer::onRenderEvent>(this);
    applyTimer(mSpeed.mValue);
}

void Timer::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Timer::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &Timer::onRenderEvent>(this);
    applyTimer(20.0f);
}

void Timer::onBaseTickEvent(BaseTickEvent& event)
{
    applyTimer(mSpeed.mValue);
}

void Timer::onRenderEvent(RenderEvent& event)
{
    applyTimer(mSpeed.mValue);
}
