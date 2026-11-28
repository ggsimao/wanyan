#pragma once

enum class TextureType {
    DIFFUSE,
    SPECULAR,
    LDR_ATTACHMENT,
    HDR_ATTACHMENT,
    NORMAL,
    DEPTH,  // parallax depth map: white is deepest
    LUMINANCE,
};
