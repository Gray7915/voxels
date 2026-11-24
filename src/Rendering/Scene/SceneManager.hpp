#pragma once
#include "Rendering/Scene/Scene.hpp"
#include <functional>
#include <memory>

namespace Rendering
{

    class SceneManager {
      public:
        void switchTo(std::unique_ptr<Scene> newScene) { pendingScene = std::move(newScene); }

        void applyPendingSwitch() {
            if (!pendingScene)
                return;
            if (currentScene)
                currentScene->onExit();
            currentScene = std::move(pendingScene);
            currentScene->onEnter();
        }

        Scene *current() { return currentScene.get(); }

      private:
        std::unique_ptr<Scene> currentScene;
        std::unique_ptr<Scene> pendingScene;
    };

} // namespace Rendering