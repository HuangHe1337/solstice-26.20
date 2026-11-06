//
// Created by vastrakai on 7/19/2024.
//

#include "NoFilter.hpp"

#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftGame.hpp>
#include <SDK/Minecraft/UIProfanityContext.hpp>
#include <Utils/MemUtils.hpp>

void NoFilter::onEnable()
{
    auto ci = ClientInstance::get();
    if (!ci || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(ci))) return;
    auto mg = ci->getMinecraftGame();
    if (!mg || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(mg))) return;
    auto pc = mg->getProfanityContext();
    if (!pc || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(pc))) return;

    __try {
        pc->setEnabled(false);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // Ignored
    }
}

void NoFilter::onDisable()
{
    auto ci = ClientInstance::get();
    if (!ci || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(ci))) return;
    auto mg = ci->getMinecraftGame();
    if (!mg || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(mg))) return;
    auto pc = mg->getProfanityContext();
    if (!pc || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(pc))) return;

    __try {
        pc->setEnabled(true);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // Ignored
    }
}
