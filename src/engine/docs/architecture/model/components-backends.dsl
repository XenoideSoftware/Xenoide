glLegacy = component "GL Legacy" "GL 1.x-2.x compatibility profile via Glaze. Display list optimization for sampler/pipeline state." "Tier: Legacy" {
    tags "Component" "TierLegacy"
}

gl4 = component "GL4" "OpenGL 4.x core profile via Glaze. Real sampler objects, VAOs, FBOs." "Tier: Modern" {
    tags "Component" "TierModern"
}

gles2 = component "GLES2" "OpenGL ES 2.0. Per-texture glTexParameter (no sampler objects)." "Tier: Modern" {
    tags "Component" "TierModern"
}

gles3 = component "GLES3" "OpenGL ES 3.0. Real sampler objects via GL_OES_sampler_objects." "Tier: Modern" {
    tags "Component" "TierModern"
}

d3d11 = component "D3D11" "Direct3D 11 native backend. ID3D11SamplerState." "Tier: Modern" {
    tags "Component" "TierModern"
}

metal = component "Metal" "Apple Metal backend. MTLSamplerState." "Tier: Modern" {
    tags "Component" "TierModern"
}

vulkan = component "Vulkan" "Vulkan backend — compute, geometry, tessellation." "Tier: Extended" {
    tags "Component" "TierExtended"
}

n64Rdp = component "N64 RDP" "libdragon rdpq — combiner equation, TMEM 4KB budget, rdpq display list." "Tier: Microcode" {
    tags "Component" "TierMicrocode"
}

gcnGx = component "GCN GX" "libogc GX — HW T&L, up to 16 TEV stages, GX display list." "Tier: TEV" {
    tags "Component" "TierTEV"
}

ps3Gcm = component "PS3 GCM" "PSL1GHT libGCM — RSX GPU, Cg shaders, GCM FIFO ring." "Tier: Legacy/Modern" {
    tags "Component" "TierModern"
}

softBuiltin = component "SoftBuiltin" "Built-in tile rasterizer — software fallback." "Tier: Legacy" {
    tags "Component" "TierLegacy"
}

softMesa = component "SoftMesa" "Mesa3D reference software renderer." "Tier: Extended" {
    tags "Component" "TierExtended"
}

nullBackend = component "Null" "No-op backend for testing and CI." "No rendering" {
    tags "Component"
}
