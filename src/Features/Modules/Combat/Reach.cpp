//
// Created by alteik on 04/09/2024.
//

#include "Reach.hpp"
#include <SDK/SigManager.hpp>
#include <Utils/MemUtils.hpp>
#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <windows.h>

void Reach::onInit() {
    if (SigManager::Reach) {
        uintptr_t func = SigManager::Reach;
        int32_t disp = *reinterpret_cast<int32_t*>(func + 3);
        mCombatPtr = reinterpret_cast<float*>(func + 7 + disp);
        DWORD oldProtect;
        VirtualProtect(mCombatPtr, sizeof(float), PAGE_READWRITE, &oldProtect);
    }

    if (SigManager::BlockReach) {
        uintptr_t addr2 = SigManager::BlockReach;
        int32_t offset2 = *reinterpret_cast<int32_t*>(addr2 + 4);
        mBlockReachPtr = reinterpret_cast<float*>(addr2 + 8 + offset2);
        DWORD oldProtect;
        VirtualProtect(mBlockReachPtr, sizeof(float), PAGE_READWRITE, &oldProtect);
    }
}

void Reach::onEnable() {
    if (!mCombatPtr && SigManager::Reach) {
        onInit();
    }

    if (mCombatPtr) {
        *mCombatPtr = mCombatReach.mValue;
    }
    if (mBlockReachPtr) {
        *mBlockReachPtr = mBlockReach.mValue;
    }

    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Reach::onBaseTickEvent>(this);
}

void Reach::onDisable() {
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Reach::onBaseTickEvent>(this);

    if (mCombatPtr) {
        *mCombatPtr = 3.0f;
    }
    if (mBlockReachPtr) {
        *mBlockReachPtr = 5.7f;
    }
}

void Reach::onBaseTickEvent(class BaseTickEvent &event) {
    if (mCombatPtr) {
        *mCombatPtr = mCombatReach.mValue;
    }
    if (mBlockReachPtr) {
        *mBlockReachPtr = mBlockReach.mValue;
    }
}
