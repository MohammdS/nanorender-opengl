#include "microui_atlas.h"

#include "atlas.inl"

const unsigned char* nano_ui_atlas_pixels(void)
{
    return atlas_texture;
}

int nano_ui_atlas_width(void)
{
    return ATLAS_WIDTH;
}

int nano_ui_atlas_height(void)
{
    return ATLAS_HEIGHT;
}

mu_Rect nano_ui_atlas_white_rect(void)
{
    return atlas[ATLAS_WHITE];
}

mu_Rect nano_ui_atlas_icon_rect(int icon)
{
    if (icon < 0 || icon >= MU_ICON_MAX) {
        return atlas[ATLAS_WHITE];
    }
    return atlas[icon];
}

mu_Rect nano_ui_atlas_glyph_rect(unsigned int codepoint)
{
    if (codepoint > 127U) {
        codepoint = 127U;
    }
    return atlas[ATLAS_FONT + codepoint];
}
