#include "Engine/Core/LayerStack.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

using namespace Engine;

namespace
{
    /// Test double recording every lifecycle call it receives, so tests can
    /// assert on *order* of calls across multiple layers, not just whether
    /// a single layer's own state changed.
    class RecordingLayer final : public Layer
    {
    public:
        RecordingLayer(std::string name, std::vector<std::string>& log)
            : Layer(std::move(name)), m_Log(log)
        {
        }

        void OnAttach() override { m_Log.get().push_back(GetName() + ":Attach"); }
        void OnDetach() override { m_Log.get().push_back(GetName() + ":Detach"); }
        void OnUpdate(Timestep) override { m_Log.get().push_back(GetName() + ":Update"); }

    private:
        std::reference_wrapper<std::vector<std::string>> m_Log;
    };
}

TEST_CASE("PushLayer calls OnAttach immediately", "[layerstack]")
{
    std::vector<std::string> log;
    LayerStack stack;

    stack.PushLayer(std::make_unique<RecordingLayer>("A", log));

    REQUIRE(log == std::vector<std::string>{"A:Attach"});
}

TEST_CASE("Regular layers stay ordered before overlays regardless of push order", "[layerstack]")
{
    std::vector<std::string> log;
    LayerStack stack;

    stack.PushOverlay(std::make_unique<RecordingLayer>("Overlay1", log));
    stack.PushLayer(std::make_unique<RecordingLayer>("Layer1", log));
    stack.PushLayer(std::make_unique<RecordingLayer>("Layer2", log));

    log.clear(); // ignore Attach noise from the pushes above, test ordering only

    for (auto& layer : stack)
    {
        layer->OnUpdate(Timestep(0.016f));
    }

    REQUIRE(log == std::vector<std::string>{"Layer1:Update", "Layer2:Update", "Overlay1:Update"});
}

TEST_CASE("PopLayer calls OnDetach and removes the layer from iteration", "[layerstack]")
{
    std::vector<std::string> log;
    LayerStack stack;

    Layer* layerA = stack.PushLayer(std::make_unique<RecordingLayer>("A", log));
    stack.PushLayer(std::make_unique<RecordingLayer>("B", log));

    stack.PopLayer(layerA);

    REQUIRE(stack.Size() == 1);
    REQUIRE(std::find(log.begin(), log.end(), "A:Detach") != log.end());
}
