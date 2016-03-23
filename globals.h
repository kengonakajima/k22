#ifndef _GLOBALS_H_
#define _GLOBALS_H_



extern TileDeck *g_base_deck;
extern TileDeck *g_girev_deck;

extern Layer *g_char_layer;
extern Layer *g_effect_layer;

class Field;
extern Field *g_fld;
extern ColorReplacerShader *g_eye_col_replacer;


extern Sound *g_kill_sound;
extern Sound *g_shoot_sig_sound;
extern Sound *g_shoot_sound;

#define WHITE Color(1,1,1,1)
#define RED Color(1,0,0,1)
#define BLUE Color(0,0,1,1)

// to be removed
class PC;
extern PC *g_pc;

#endif
