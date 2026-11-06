//
// Created by vastrakai on 6/24/2024.
//

#include "MinecraftGame.hpp"
#include <map>
#include <memory>
#include <libhat/Access.hpp>
#include <SDK/OffsetProvider.hpp>
#include <Utils/MemUtils.hpp>

#include "UWP/BedrockPlatformUWP.hpp"
#include "UWP/MainView.hpp"

#include "ClientInstance.hpp"

MinecraftGame* MinecraftGame::getInstance(){
    static auto minecraftgame = reinterpret_cast<MinecraftGame**>(SigManager::MinecraftGame_instance);
    return minecraftgame ? *minecraftgame : nullptr;
}

ClientInstance* MinecraftGame::getPrimaryClientInstance()
{
    return ClientInstance::get();
}

UIProfanityContext* MinecraftGame::getProfanityContext()
{
    return hat::member_at<UIProfanityContext*>(this, OffsetProvider::MinecraftGame_mProfanityContext);
}

bool MinecraftGame::getMouseGrabbed()
{
    return hat::member_at<bool>(this, OffsetProvider::MinecraftGame_mMouseGrabbed);
}

void MinecraftGame::setMouseGrabbed(bool grabbed)
{
    hat::member_at<bool>(this, OffsetProvider::MinecraftGame_mMouseGrabbed) = grabbed;
}

std::string MinecraftGame::getxuid()
{
    return *hat::member_at<std::string*>(this, OffsetProvider::MinecraftGame_mXuid);
}

void MinecraftGame::playUi(const std::string& soundName, float volume, float pitch)
{
    auto ci = ClientInstance::get();
    if (ci)
    {
        ci->playUi(soundName, volume, pitch);
    }
}
