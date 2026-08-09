#include "Engine/Core/LayerStack.h"

#include <algorithm>

namespace Engine
{
    Layer* LayerStack::PushLayer(std::unique_ptr<Layer> layer)
    {
        Layer* observer = layer.get();
        m_Layers.emplace(m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex), std::move(layer));
        ++m_LayerInsertIndex;
        observer->OnAttach();
        return observer;
    }

    Layer* LayerStack::PushOverlay(std::unique_ptr<Layer> overlay)
    {
        Layer* observer = overlay.get();
        m_Layers.emplace_back(std::move(overlay));
        observer->OnAttach();
        return observer;
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        const auto it = std::find_if(
            m_Layers.begin(), m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex),
            [layer](const std::unique_ptr<Layer>& candidate) { return candidate.get() == layer; });

        if (it != m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex))
        {
            (*it)->OnDetach();
            m_Layers.erase(it);
            --m_LayerInsertIndex;
        }
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        const auto it = std::find_if(
            m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex), m_Layers.end(),
            [overlay](const std::unique_ptr<Layer>& candidate) { return candidate.get() == overlay; });

        if (it != m_Layers.end())
        {
            (*it)->OnDetach();
            m_Layers.erase(it);
        }
    }

} // namespace Engine
