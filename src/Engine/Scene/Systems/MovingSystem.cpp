#include "MovingSystem.h"

#include <Types.h>
#include <MathUtils.h>
#include <Scene/SceneRegistry.h>
#include <Scene/Components/MovingComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/RelationshipComponent.h>

#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
   MovingSystem::MovingSystem(SharedPtr<Keyboard> keyboard) : mKeyboard(keyboard)
   {
   }

   MovingSystem::~MovingSystem() = default;

   void MovingSystem::Process(SceneRegistry* scene, const Timer &timer)
   {
      auto& registry = scene->GetRegistry();
      const auto &view = registry.view<
          Components::MovingComponent,
          Components::WorldTransformComponent,
          Components::RelationshipComponent>();

      const float32 speed = 5 * timer.DeltaTime();
      const float32 rotationSpeed = 1.0f * timer.DeltaTime();

      for (auto &&[entity, movingComponent, worldTransform, relationship] : view.each())
      {
          {
              float32 pitch, yaw, roll;
              Math::ExtractPitchYawRollFromXMMatrix(&pitch, &yaw, &roll, &worldTransform.transform);
              movingComponent.mPitch = pitch;
              movingComponent.mYaw = yaw; 
          }

         float32 xTranslate = 0;
         float32 zTranslate = 0;
         if (mKeyboard->IsKeyPressed(KeyCode::Key::Up))
         {
             movingComponent.Rotate(0.0f, -rotationSpeed);
         }
         else if (mKeyboard->IsKeyPressed(KeyCode::Key::Down))
         {
             movingComponent.Rotate(0.0f, +rotationSpeed);
         }

         if (mKeyboard->IsKeyPressed(KeyCode::Key::Left))
         {
              movingComponent.Rotate(-rotationSpeed, 0.0f);
         }
         else if (mKeyboard->IsKeyPressed(KeyCode::Key::Right))
         {
             movingComponent.Rotate(+rotationSpeed, 0.0f);
         }

         if (mKeyboard->IsKeyPressed(KeyCode::Key::W))
         {
            zTranslate = +speed;
         }
         else if (mKeyboard->IsKeyPressed(KeyCode::Key::S))
         {
            zTranslate = -speed;
         }

         if (mKeyboard->IsKeyPressed(KeyCode::Key::D))
         {
            xTranslate = +speed;
         }
         else if (mKeyboard->IsKeyPressed(KeyCode::Key::A))
         {
            xTranslate = -speed;
         }

         dx::XMVECTOR s;
         dx::XMVECTOR r;
         dx::XMVECTOR t;
         dx::XMMatrixDecompose(&s, &r, &t, worldTransform.transform);

         using namespace dx;

         auto pitchYawRotation = dx::XMQuaternionRotationRollPitchYaw(
            movingComponent.mPitch, 
            movingComponent.mYaw, 
            0.0f);

         auto rotationMatrix = dx::XMMatrixRotationQuaternion(pitchYawRotation);

         auto positionOffset = DirectX::XMVector3Transform(
                dx::XMVectorSet(xTranslate, 0.0f, zTranslate, 0.0f),
                rotationMatrix);

         auto translate = dx::XMMatrixTranslationFromVector(t + positionOffset);
         auto localTransform = rotationMatrix * translate;

         if (relationship.parent != entt::null)
         {
             const auto& parentWorldTransform = registry.get<Components::WorldTransformComponent>(relationship.parent);
             dx::XMVECTOR D;
             auto inverseParentWorldTransform = dx::XMMatrixInverse(&D, parentWorldTransform.transform);
             localTransform = localTransform * inverseParentWorldTransform;
         }

         registry.replace<Components::LocalTransformComponent>(entity, localTransform);
      }
   }
} // namespace Engine::Scene::Systems