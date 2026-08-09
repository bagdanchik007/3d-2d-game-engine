#pragma once

#include "Engine/Core/Layer.h"

#include <memory>
#include <vector>

namespace Engine
{
    /// Owns all layers and maintains their relative order.
    ///
    /// Regular layers are inserted before the first overlay; overlays
    /// (typically debug UI) always render/update last and receive events
    /// first. Storage is std::unique_ptr<Layer> so the stack owns layers
    /// via RAII — no manual delete loop in the destructor, unlike the
    /// classic raw-pointer version of this pattern. Push functions return a
    /// non-owning observer pointer so the caller (Application, Sandbox) can
    /// still reference the layer without sharing ownership of it.
    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack() = default;

        LayerStack(const LayerStack&) = delete;
        LayerStack& operator=(const LayerStack&) = delete;

        Layer* PushLayer(std::unique_ptr<Layer> layer);
        Layer* PushOverlay(std::unique_ptr<Layer> overlay);

        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        [[nodiscard]] auto begin() noexcept { return m_Layers.begin(); }
        [[nodiscard]] auto end() noexcept { return m_Layers.end(); }
        [[nodiscard]] auto rbegin() noexcept { return m_Layers.rbegin(); }
        [[nodiscard]] auto rend() noexcept { return m_Layers.rend(); }

        [[nodiscard]] auto begin() const noexcept { return m_Layers.begin(); }
        [[nodiscard]] auto end() const noexcept { return m_Layers.end(); }

        [[nodiscard]] std::size_t Size() const noexcept { return m_Layers.size(); }

    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        std::size_t m_LayerInsertIndex = 0;
    };

} // namespace Engine
