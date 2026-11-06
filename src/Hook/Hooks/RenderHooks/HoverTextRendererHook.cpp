//
// Created by Solar on 11/18/2024.
//

#include "HoverTextRendererHook.hpp"

#include <Features/Modules/Visual/Interface.hpp>
#include <SDK/Minecraft/mce.hpp>
#include <Utils/MemUtils.hpp>

std::unique_ptr<Detour> HoverTextRendererHook::mHoverTextRendererRenderDetour;

void HoverTextRendererHook::onHoverTextRendererRender(HoverTextRenderer* _this, void* mRenderContext, ClientInstance* mClient, glm::vec4* mRenderAABB, int mPass)
{
    auto original = mHoverTextRendererRenderDetour->getOriginal<&HoverTextRendererHook::onHoverTextRendererRender>();

    if (!_this || !MemUtils::isValidPtr(reinterpret_cast<uintptr_t>(_this))) {
        return original(_this, mRenderContext, mClient, mRenderAABB, mPass);
    }

    if (HoverTextRender::mTimeDisplayed == 0) {
        HoverTextRender::mTimeDisplayed++;
    }

    __try {
        HoverTextRender::mInfo.mText = _this->mFilteredContent;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        HoverTextRender::mInfo.mText = "";
    }

    HoverTextRender::mInfo.mPos = glm::vec2(ImRenderUtils::getMousePos().x + 3, ImRenderUtils::getMousePos().y + 3);

    if (gFeatureManager && gFeatureManager->mModuleManager)
    {
        auto iface = gFeatureManager->mModuleManager->getModule<Interface>();
        if (iface && iface->mEnabled && iface->mHoveredItem.mValue)
        {
            return;
        }
    }

    original(_this, mRenderContext, mClient, mRenderAABB, mPass);
}

void HoverTextRendererHook::init()
{
    mHoverTextRendererRenderDetour = std::make_unique<Detour>("HoverTextRenderer::render", reinterpret_cast<void*>(SigManager::HoverTextRenderer_render), &HoverTextRendererHook::onHoverTextRendererRender);
}
