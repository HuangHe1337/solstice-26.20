#pragma once

#include <Hook/Hook.hpp>
#include <Hook/HookManager.hpp>
#include <atomic>

class Simulation;

class SimulationTickHook : public Hook {
public:
    SimulationTickHook() : Hook() {
        mName = "Simulation::tick";
    }

    static std::unique_ptr<Detour> mDetour;
    static inline std::atomic<float> sCustomTimer{20.0f};
    static inline std::atomic<bool> sOverrideTimer{false};
    static inline Simulation* sLastSim = nullptr;

    static void onSimulationTick(Simulation* sim, float a2);
    void init() override;
};
