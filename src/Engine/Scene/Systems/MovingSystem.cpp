#include "MovingSystem.h"

#include <Types.h>
#include <MathUtils.h>
#include <Scene/Components/MovingComponent.h>
#include <Scene/Components/LocalTransformComponent.h>

#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
   MovingSystem::MovingSystem(SharedPtr<Keyboard> keyboard) : mKeyboard(keyboard)
   {
   }

   MovingSystem::~MovingSystem() = default;

   void MovingSystem::Init(entt::registry *registry)
   {
      const auto &view = registry->view<Components::MovingComponent, Components::LocalTransformComponent>();
      for (auto &&[entity, movingComponent, transformComponent] : view.proxy())
      {
         float32 pitch, yaw, roll;
         Math::ExtractPitchYawRollFromXMMatrix(&pitch, &yaw, &roll, &transformComponent.transform);
         movingComponent.mPitch = pitch;
         movingComponent.mYaw = yaw;
      }
   }

   void MovingSystem::Process(entt::registry *registry, const Timer &timer)
   {
      const auto &view = registry->view<Components::MovingComponent, Components::LocalTransformComponent>();

      const float32 speed = 5 * timer.DeltaTime();
      const float32 rotationSpeed = 1.0f * timer.DeltaTime();

      for (auto &&[entity, movingComponent, transformComponent] : view.proxy())
      {
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
         dx::XMMatrixDecompose(&s, &r, &t, transformComponent.transform);

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
         transformComponent.transform = rotationMatrix * translate;

         registry->replace<Components::LocalTransformComponent>(entity, transformComponent);
      }
   }
} // namespace Engine::Scene::Systems