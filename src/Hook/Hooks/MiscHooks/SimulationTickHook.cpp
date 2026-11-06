#include "SimulationTickHook.hpp"

#include <SDK/Minecraft/MinecraftSim.hpp>
#include <SDK/SigManager.hpp>
#include <Utils/MemUtils.hpp>
#include <Windows.h>

std::unique_ptr<Detour> SimulationTickHook::mDetour;

void SimulationTickHook::onSimulationTick(Simulation* sim, float a2)
{
    auto oFunc = mDetour->getOriginal<&onSimulationTick>();

    if (sim && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(sim))) {
        sLastSim = sim;
        if (sOverrideTimer.load()) {
            float speed = sCustomTimer.load();
            sim->mTimer = speed;
            sim->mTimerMultiplier = speed / 20.0f;
        } else if (sim->mTimer != 20.0f) {
            sim->mTimer = 20.0f;
            sim->mTimerMultiplier = 1.0f;
        }
    }

    oFunc(sim, a2);

    if (sim && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(sim))) {
        if (sOverrideTimer.load()) {
            float speed = sCustomTimer.load();
            sim->mTimer = speed;
            sim->mTimerMultiplier = speed / 20.0f;
        }
    }
}

void SimulationTickHook::init()
{
    if (SigManager::Simulation_tick) {
        mDetour = std::make_unique<Detour>("Simulation::tick", reinterpret_cast<void*>(SigManager::Simulation_tick), &SimulationTickHook::onSimulationTick);
    }
}
