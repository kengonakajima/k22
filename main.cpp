#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "client.h"
#include "dimension.h"
#include "field.h"
#include "char.h"
#include "pc.h"
#include "mapview.h"
#include "enemy.h"
#include "conf.h"

MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_char_layer;
Layer *g_effect_layer;
Layer *g_field_layer;
Texture *g_base_atlas;
TileDeck *g_base_deck;
TileDeck *g_girev_deck;
Camera *g_camera;
ColorReplacerShader *g_eye_col_replacer;
SoundSystem *g_sound_system;
Sound *g_shoot_sig_sound;
Sound *g_kill_sound;
Sound *g_shoot_sound;
int g_last_render_cnt ;


Mouse *g_mouse;
Keyboard *g_keyboard;
Pad *g_pad;

GLFWwindow *g_window;



Field *g_fld;
MapView *g_mapview;

PC *g_pc; // to be removed

Vec2 getRandomPos( DIR4 d ) {
    switch(d) {
    case  DIR4_UP: return Vec2( range(0,SCRW), SCRH+PPC);
    case  DIR4_DOWN: return Vec2( range(0,SCRW), 0-PPC);
    case  DIR4_RIGHT: return Vec2(SCRW+PPC, range(0,SCRH) );
    case  DIR4_LEFT: return Vec2(0-PPC, range(0,SCRH) );
    default:
        assert(false);
    }
}

/////////////
void debugKeyPressed( int key ) {
    switch(key) {
    case 'T':
        print("t");
        new Fly( g_pc->loc + Vec2(100,100) ); //getRandomPos(DIR4_UP) );
        break;
    }
}
void keyboardCallback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    g_keyboard->update( key, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
    if(action) {
        debugKeyPressed(key);
    }
}
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods ) {
    g_mouse->updateButton( button, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void cursorPosCallback( GLFWwindow *window, double x, double y ) {
    g_mouse->updateCursorPosition( x,y);
}

// Assuming camera is not moving (always 0,0 in center of the screen)
Vec2 screenPosToWorldLoc( Vec2 scrpos ) {
    return Vec2( scrpos.x - SCRW/2, (scrpos.y - SCRH/2)*-1 ) + g_camera->loc;
}

//////

void gameUpdate(void) {
    static double last_print_at = 0;
    static int frame_counter = 0;
    static double last_poll_at = now();

    double t = now();
    double dt = t - last_poll_at;
    
    frame_counter ++;

    glfwPollEvents();
    g_pad->readKeyboard(g_keyboard);
    
    int cnt = g_moyai_client->poll(dt);

    if(last_print_at == 0){
        last_print_at = t;
    } else if( last_print_at < t-1 ){
        fprintf(stderr,"FPS:%d prop:%d render:%d\n", frame_counter, cnt, g_last_render_cnt  );
        frame_counter = 0;
        last_print_at = t;
    }

    // replace white to random color
    g_eye_col_replacer->setColor( Color(0xF7E26B), Color( range(0,1),range(0,1),range(0,1),1), 0.02 );

    if( glfwGetKey( g_window, 'Q') ) {
        print("Q pressed");
        exit(0);
    }

    // TODO: implement multiplayer
    Vec2 ctl_move;
    g_pad->getVec(&ctl_move);
    Vec2 cursor_pos = g_mouse->getCursorPos();
    Vec2 cursor_wloc = screenPosToWorldLoc(cursor_pos);
    Vec2 shootdir = cursor_wloc - g_pc->loc;
    Vec2 ctl_shoot = shootdir.normalize(1.0f);
    if( !g_mouse->getButton(0) ) ctl_shoot*=0;
    g_pc->ideal_v = ctl_move;
    g_pc->shoot_v = ctl_shoot;
        
    last_poll_at = t;
}


void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}


void gameInit() {
    print("program start");

#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    

    g_sound_system = new SoundSystem();
    g_shoot_sound = g_sound_system->newSound( "sounds/shoot.wav", 0.5, false );          // PC shooting sound option 0.        
    g_shoot_sig_sound = g_sound_system->newSound( "sounds/shoot_sig.wav" ); // Enemy shoots a fast and small missile SIG.
    g_kill_sound = g_sound_system->newSound( "sounds/machine_explo.wav", 0.3, false ); // PC shoot and destroy enemy machines.
    
    // glfw
    if( !glfwInit() ) {
        print("can't init glfw");
        exit(1);
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( SCRW, SCRH, "demo2d", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(1); // vsync
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);


    g_moyai_client = new MoyaiClient(g_window);

    g_viewport = new Viewport();
    g_viewport->setSize(SCRW,SCRH); // set actual framebuffer size to output
    g_viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

    g_field_layer = new Layer();
    g_field_layer->setViewport(g_viewport);
    g_moyai_client->insertLayer(g_field_layer);
    g_char_layer = new Layer();
    g_moyai_client->insertLayer(g_char_layer);
    g_char_layer->setViewport(g_viewport);    
    g_effect_layer = new Layer();
    g_moyai_client->insertLayer(g_effect_layer);
    g_effect_layer->setViewport(g_viewport);
    g_base_atlas = new Texture();
    g_base_atlas->load("./images/k22base1024.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(32,42,24,24 );

    Texture *girevtex = new Texture();
    girevtex->load( "./images/girev64.png");
    g_girev_deck = new TileDeck();
    g_girev_deck->setTexture(girevtex);
    g_girev_deck->setSize(1,1,64,64);
    
    g_camera = new Camera();
    g_camera->setLoc(SCRW/2,SCRH/2);

    g_char_layer->setCamera(g_camera);
    g_effect_layer->setCamera(g_camera);
    g_field_layer->setCamera(g_camera);

    // Eye colors
    g_eye_col_replacer = new ColorReplacerShader();
    if(!g_eye_col_replacer->init()) {
        print("can't initialize shader");
        exit(0);
    }

    g_fld = new Field(FIELD_W,FIELD_H);
    g_fld->generate();
    g_mapview = new MapView(FIELD_W,FIELD_H);
    g_mapview->update(g_fld);
    g_field_layer->insertProp(g_mapview);
    g_mapview->setLoc(0,0);

    g_pc = new PC( Vec2(0,0) );
    g_pc->respawn();

    // input
    g_keyboard = new Keyboard();
    glfwSetKeyCallback( g_window, keyboardCallback );
    g_pad = new Pad();
    g_mouse = new Mouse();
    glfwSetMouseButtonCallback( g_window, mouseButtonCallback );
    glfwSetCursorPosCallback( g_window, cursorPosCallback );
    
}


void gameRender() {
        g_last_render_cnt = g_moyai_client->render();
}
void gameFinish() {
    glfwTerminate();
}



#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE)        
int main(int argc, char **argv )
{
    gameInit();
    while( !glfwWindowShouldClose(g_window) ){
        gameUpdate();
        gameRender();

    }
    gameFinish();
    print("program finished");
    return 0;
}
#endif
