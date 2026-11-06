//
// Created by vastrakai on 6/25/2024.
//

#include "GameSession.hpp"

#include <libhat.hpp>
#include <SDK/OffsetProvider.hpp>
#include <Utils/MemUtils.hpp>
#include <Windows.h>

void* GameSession::getEventCallback()
{
    if (!this || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(this))) return nullptr;
    __try {
        auto cb = hat::member_at<void*>(this, OffsetProvider::GameSession_mEventCallback);
        if (cb && MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(cb))) return cb;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return nullptr;
}
