//
// Created by vastrakai on 7/18/2024.
//

#include "SafeWalk.hpp"

#include <Features/Events/BaseTickEvent.hpp>

// Clean hook at function entry of SneakMovementSystem::_tickSneakMovementSystem (0x1497D73E0)
// Calling convention: __fastcall (rcx=Actor*, rdx=..., r8=..., r9=SneakMovementComponent*)
// If r9 != nullptr, sets [r9 + 55h] = 1, sets eax = 1, and returns cleanly.
// 4D 85 C9             test r9, r9
// 74 05                jz +5 -> mov eax, 1
// 41 C6 41 55 01       mov byte ptr [r9+55h], 1
// B8 01 00 00 00       mov eax, 1
// C3                   ret
static const unsigned char PatchBytes[16] = {
    0x4D, 0x85, 0xC9,
    0x74, 0x05,
    0x41, 0xC6, 0x41, 0x55, 0x01,
    0xB8, 0x01, 0x00, 0x00, 0x00,
    0xC3
};

static uintptr_t getFuncAdr() {
    static uintptr_t adr = 0;
    if (adr) return adr;
    if (SigManager::SneakMovementSystem_tickSneakMovementSystem_func) {
        adr = SigManager::SneakMovementSystem_tickSneakMovementSystem_func;
    } else if (SigManager::SneakMovementSystem_tickSneakMovementSystem) {
        adr = SigManager::SneakMovementSystem_tickSneakMovementSystem - 0xEC;
    }
    return adr;
}

static void patchSafeWalk(bool v) {
    static bool current = false;
    uintptr_t funcAdr = getFuncAdr();
    if (!funcAdr || current == v) {
        return;
    }

    if (v) {
        if (!SafeWalk::mHasOriginalData) {
            MemUtils::ReadBytes((void*)funcAdr, SafeWalk::mOriginalData, sizeof(SafeWalk::mOriginalData));
            SafeWalk::mHasOriginalData = true;
        }
        MemUtils::writeBytes(funcAdr, const_cast<unsigned char*>(PatchBytes), sizeof(PatchBytes));
        current = true;
    }
    else {
        if (SafeWalk::mHasOriginalData) {
            MemUtils::writeBytes(funcAdr, SafeWalk::mOriginalData, sizeof(SafeWalk::mOriginalData));
        }
        current = false;
    }
}

void SafeWalk::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &SafeWalk::onBaseTickEvent>(this);
}

void SafeWalk::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &SafeWalk::onBaseTickEvent>(this);
    patchSafeWalk(false);
}

void SafeWalk::onBaseTickEvent(BaseTickEvent& event)
{
    auto player = event.mActor;
    if (!player) {
        patchSafeWalk(false);
        return;
    }

    if (player->isOnGround()) {
        patchSafeWalk(mEnabled);
    }
    else {
        patchSafeWalk(false);
    }
}
