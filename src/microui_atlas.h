#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <microui.h>

const unsigned char* nano_ui_atlas_pixels(void);
int nano_ui_atlas_width(void);
int nano_ui_atlas_height(void);
mu_Rect nano_ui_atlas_white_rect(void);
mu_Rect nano_ui_atlas_icon_rect(int icon);
mu_Rect nano_ui_atlas_glyph_rect(unsigned int codepoint);

#ifdef __cplusplus
}
#endif
