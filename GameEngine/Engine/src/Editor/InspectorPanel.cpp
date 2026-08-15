#include "Engine/Editor/InspectorPanel.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cstring>

namespace Engine::Editor
{
    void InspectorPanel::OnImGuiRender(ECS::Entity selectedEntity)
    {
        ImGui::Begin("Inspector");

        if (m_Context != nullptr && !selectedEntity.IsNull() && m_Context->GetRegistry().IsValid(selectedEntity))
        {
            DrawTagComponent(selectedEntity);
            ImGui::Separator();
            DrawTransformComponent(selectedEntity);
        }
        else
        {
            ImGui::TextDisabled("No entity selected");
        }

        ImGui::End();
    }

    void InspectorPanel::DrawTagComponent(ECS::Entity entity) const
    {
        auto& tag = m_Context->GetRegistry().GetComponent<Scene::TagComponent>(entity);

        // ImGui::InputText needs a raw char buffer, not std::string
        // directly - copying into a fixed-size stack buffer each frame is
        // the standard, simple approach for editor text fields, at the
        // (here, irrelevant) cost of a hard length cap. A growable
        // approach exists (std::string::data() + resize tricks against
        // ImGuiInputTextFlags_CallbackResize) but isn't worth the added
        // complexity for entity names, which are short by convention
        // everywhere else in this engine already.
        std::array<char, 256> buffer{};
        const std::size_t copyLength = std::min(tag.Name.size(), buffer.size() - 1);
        std::memcpy(buffer.data(), tag.Name.data(), copyLength);
        buffer[copyLength] = '\0';

        if (ImGui::InputText("Name", buffer.data(), buffer.size()))
        {
            tag.Name = buffer.data();
        }
    }

    void InspectorPanel::DrawTransformComponent(ECS::Entity entity) const
    {
        if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            return;
        }

        auto& transform = m_Context->GetRegistry().GetComponent<Scene::TransformComponent>(entity);

        ImGui::DragFloat3("Position", &transform.Position.x, 0.1f);

        // Rotation is edited as axis + angle (degrees), not Euler angles:
        // Euler angles need a chosen rotation order and are prone to
        // gimbal lock and "which convention" ambiguity (see the doc
        // comment on Quaternion::ToAxisAngle, M12). Axis+angle has neither
        // problem and is the direct, unambiguous inverse of
        // Quaternion::FromAxisAngle already used everywhere else in this
        // engine that constructs a rotation.
        auto [axis, angleRadians] = transform.Rotation.ToAxisAngle();
        float angleDegrees = Math::Degrees(angleRadians);
        bool rotationChanged = false;
        rotationChanged |= ImGui::DragFloat3("Rotation Axis", &axis.x, 0.01f);
        rotationChanged |= ImGui::DragFloat("Rotation Angle", &angleDegrees, 0.5f);
        if (rotationChanged)
        {
            transform.Rotation = Math::Quaternion::FromAxisAngle(axis, Math::Radians(angleDegrees));
        }

        ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
    }

} // namespace Engine::Editor
