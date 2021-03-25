#pragma once

#include <Scene/SceneForwards.h>

#include <map>
#include <string>
#include <future>
#include <memory>
#include <vector>

#include <imgui/imgui.h>
#include <UI/ImGuiEx.h>

namespace Engine::Scene
{
    struct SceneLoadingInfo
    {
        bool loadScene;
        std::map<std::string, std::string> scenes;
        std::string scenePath;

        std::future<Loader::SceneDto> sceneFuture;

        void DrawSelector()
        {
            ImGui::Begin("Scene selector");
            {
                std::vector<std::string> keys;
                std::vector<std::string> values;
                keys.reserve(scenes.size());
                values.reserve(scenes.size());
                for (auto &i : scenes)
                {
                    keys.push_back(i.first);
                    values.push_back(i.second);
                }
                auto it = std::find(values.begin(), values.end(), scenePath);
                auto index = std::distance(values.begin(), it);

                static int item_current = static_cast<int>(index);
                ImGui::Combo("Scenes", &item_current, keys);

                auto& selected = scenes[keys[item_current]];

                if (scenePath != selected)
                {
                    if (ImGui::Button("Load"))
                    {
                        scenePath = selected;
                        loadScene = true;
                    }
                }
            }
            ImGui::End();
        }
    };
}