//
// Created by alteik on 04/10/2024.
//

#include "AirSpeed.hpp"
#include <SDK/SigManager.hpp>
#include <windows.h>

void AirSpeed::onInit() {
    uintptr_t addr = SigManager::GetSpeedInAirWithSprint;
    if (!addr) return;

    // Check if 1.26.20 format: starts with 56 48 83 EC 20
    if (*reinterpret_cast<uint8_t*>(addr) == 0x56 && *reinterpret_cast<uint8_t*>(addr + 1) == 0x48) {
        mSpeedInAirWithoutSprintPtr = reinterpret_cast<float*>(addr + 0x0F);
        int32_t rel = *reinterpret_cast<int32_t*>(addr + 0x2A);
        mSpeedInAirWithSprintPtr = reinterpret_cast<float*>(addr + 0x2E + rel);
        mSpeedInAirWithSprintValue = 1.3f;
        mSpeedInAirWithoutSprintValue = 0.020f;

        DWORD oldProtect;
        VirtualProtect(reinterpret_cast<void*>(addr), 0x40, PAGE_EXECUTE_READWRITE, &oldProtect);
        VirtualProtect(mSpeedInAirWithSprintPtr, sizeof(float), PAGE_READWRITE, &oldProtect);
        return;
    }

    int32_t rel; 
    memcpy(&rel, (void*)(addr + 0x13), 4);
    uintptr_t tb = (addr + 0x17) + rel;

    mSpeedInAirWithoutSprintPtr = reinterpret_cast<float*>(tb + 4);
    mSpeedInAirWithSprintPtr = reinterpret_cast<float*>(tb + 0);
}

void AirSpeed::onEnable() {
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &AirSpeed::onBaseTickEvent>(this);
}

void AirSpeed::onDisable() {
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &AirSpeed::onBaseTickEvent>(this);
    if (mSpeedInAirWithoutSprintPtr) MemUtils::Write((uintptr_t) mSpeedInAirWithoutSprintPtr, mSpeedInAirWithoutSprintValue);
    if (mSpeedInAirWithSprintPtr) MemUtils::Write((uintptr_t) mSpeedInAirWithSprintPtr, mSpeedInAirWithSprintValue);
}

void AirSpeed::onBaseTickEvent(BaseTickEvent &event) {
    if (mSpeedInAirWithoutSprintPtr) MemUtils::Write((uintptr_t) mSpeedInAirWithoutSprintPtr, mSpeedInAirWithoutSprintValue * mNoSprintSpeed.mValue);
    if (mSpeedInAirWithSprintPtr) MemUtils::Write((uintptr_t) mSpeedInAirWithSprintPtr, mSpeedInAirWithSprintValue * mSprintSpeed.mValue);
}
