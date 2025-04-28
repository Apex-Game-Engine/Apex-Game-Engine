#pragma once

#define APEX_GFX_DEBUG !(APEX_CONFIG_FINAL)
#define GFX_USE_BINDLESS_DESCRIPTORS 1

namespace apex { namespace gfx {

constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;
constexpr size_t MAX_RENDER_THREADS = 6;
constexpr size_t MAX_COMMAND_POOLS_PER_QUEUE = MAX_FRAMES_IN_FLIGHT * MAX_RENDER_THREADS;

} }
