#ifndef _GLOBALS_H_
#define _GLOBALS_H_



extern TileDeck *g_base_deck;
extern TileDeck *g_girev_deck;

extern Layer *g_bg_layer;
extern Layer *g_char_layer;
extern Layer *g_effect_layer;
extern Layer *g_field_layer;

class Field;
extern Field *g_fld;
extern ColorReplacerShader *g_eye_col_replacer;


extern Sound *g_kill_sound;
extern Sound *g_shoot_sig_sound;
extern Sound *g_shoot_sound;
extern Sound *g_hurt_sound;
extern Sound *g_beamhit_sound;

#define WHITE Color(1,1,1,1)
#define RED Color(1,0,0,1)
#define BLUE Color(0,0,1,1)


extern Texture *g_planet_tex;
extern Texture *g_space_bg_tex;

Vec2 screenPosToWorldLoc( Vec2 scrpos, Camera *cam );
class PC;
PC *getNearestPC(Vec2 from);

extern bool g_enable_single_screen;

#endif
