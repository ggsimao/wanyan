#pragma once

enum class TextureType {
    DIFFUSE,
    SPECULAR,
    ATTACHMENT,
    NORMAL,
    DEPTH,  // parallax depth map: white is deepest
};
