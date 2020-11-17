# DirectX 12 Playground

This is a small 3D engine created for my education purpose. The main goal of this project is trying to understand contemporary graphic APIs and implementing different rendering techniques.

![engine](img/img.png?raw=true)

## Features
* Forward pipeline
* PBR material (Metal/Roughness Workflow)
* Punctual lights
* Sky box
* GLTF scenes (via [assimp](https://github.com/assimp/assimp))
* Debug UI (via [Dear ImGui](https://github.com/ocornut/imgui))
* Stateless render passes (Forward, SkyBox, ToneMapping)
* ECS instead of Scene Graph (via [entt](https://github.com/skypjack/entt))

## In progress
* Deferred pipeline
* Render graph
* Frustum culling

## Planned
* Sky simulating (Rayleigh Scattering, MieScattering)
* Image-based lighting
* Better memory managment
* Compute passes
* Area lights
* Shadows