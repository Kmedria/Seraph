#pragma once
#include "Renderer/Renderer.h"
#include "Engine/Entity/Entity.h"
#include "Engine/MemoryArena/MemoryArena.h"
#include "GameDetails.h"

struct Seraph {
    Renderer::Context rendererContext;
    MemoryArena memoryArena;

    Seraph() {}

    ~Seraph() {
        Timer(engineCleanup, "Seraph Engine Cleanup");
    }
} seraph;
