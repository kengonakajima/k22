﻿#include <stdio.h>
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
Layer *g_bg_layer;
Layer *g_char_layer;
Layer *g_effect_layer;
Layer *g_field_layer;
Texture *g_base_atlas;
Texture *g_space_bg_tex;
Texture *g_planet_tex;
TileDeck *g_base_deck;
TileDeck *g_girev_deck;
Camera *g_camera;
ColorReplacerShader *g_eye_col_replacer;
SoundSystem *g_sound_system;
Sound *g_shoot_sig_sound;
Sound *g_kill_sound;
Sound *g_shoot_sound;
Sound *g_hurt_sound;
Sound *g_beamhit_sound;
int g_last_render_cnt ;



GLFWwindow *g_window;

bool g_enable_network = true;

RemoteHead *g_rh;
#define HEADLESS_SERVER_PORT 22222


Field *g_fld;
MapView *g_mapview;

ObjectPool<PC> g_pc_cl_pool; // client idから検索

PC *addPC( Client *cl ) {
    PC *pc = g_pc_cl_pool.get(cl->id);
    if(pc) {
        assertmsg(false, "can't add a pc twice. id:%d",cl->id);
    }
    pc = new PC(cl);
    pc->respawn();
    g_pc_cl_pool.set(cl->id,pc);
    print("added a client[%d] to pc pool", cl->id );
    return pc;
}
// use client id is 0
void addLocalPC( PC *to_add ) {
    to_add->respawn();
    g_pc_cl_pool.set(0,to_add);
}
void delPC( PC *pc ) {
    PC *found = g_pc_cl_pool.get( pc->cl->id );
    if(!found) {
        assertmsg( false, "can't find a clid:%d in pc pool", pc->cl->id );
    }
    g_pc_cl_pool.del(pc->cl->id);
    delete pc;
}
PC *getPC(Client *cl) {
    return g_pc_cl_pool.get(cl->id);
}
PC *getLocalPC() {
    for( std::unordered_map<unsigned int,PC*>::iterator it = g_pc_cl_pool.idmap.begin(); it != g_pc_cl_pool.idmap.end(); ++it ) {
        PC *pc = it->second;
        if(pc->cl == 0) {
            return pc;            
        }
    }
    return NULL;
}
PC *getNearestPC(Vec2 from) {
    PC *out=0;
    float distance = 9999999999999;
    for( std::unordered_map<unsigned int,PC*>::iterator it = g_pc_cl_pool.idmap.begin(); it != g_pc_cl_pool.idmap.end(); ++it ) {
        PC *pc = it->second;
        float d = pc->loc.len(from);
        if(d<distance) {
            distance = d;
            out = pc;            
        }
    }
    return out;
}
Vec2 getRandomEdgePos( DIR4 d ) {
    switch(d) {
    case  DIR4_UP: return Vec2( range(0,SCRW), SCRH+PPC);
    case  DIR4_DOWN: return Vec2( range(0,SCRW), 0-PPC);
    case  DIR4_RIGHT: return Vec2(SCRW+PPC, range(0,SCRH) );
    case  DIR4_LEFT: return Vec2(0-PPC, range(0,SCRH) );
    default:
        assert(false);
    }
}

void pollPopper(double dt) {
    static double popper_accum_time=0;
    popper_accum_time += dt;
    static double last_pop_at = 0;
    if( popper_accum_time > last_pop_at + 5 ) {
        last_pop_at = popper_accum_time;
        Vec2 at = getRandomEdgePos( randomDir() );
        int t = irange( 0,100);
        if( t < 5 ) {
            new Girev(at);            
        } else if( t%5==0 ){
            new Takwashi(at);
        } else if( t%2==0) {
            DIR4 d = randomDir();
            for(int n=0;n<7;n++){
                Vec2 at = getRandomEdgePos(d);
                new Fly(at);
            }
        }
    }
}

/////////////
void debugKeyPressed( PC *pc, int key ) {    
    Vec2 at = pc->loc + Vec2(100,100); //getRandomPos(DIR4_UP) );
    switch(key) {
    case 'I':
        new Egg(at);        
        break;
    case 'U':
        new Girev(at);
        break;
    case 'Y':
        new Takwashi(at);
        break;
    case 'T':
        new Fly(at);
        break;
    }
}
void keyboardCallback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    PC *pc = getLocalPC();
    if(!pc) return;
    pc->keyboard->update( key, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
    if(action) {
        debugKeyPressed(pc,key);
    }
}
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods ) {
    PC *pc = getLocalPC();
    if(!pc)return;
    pc->mouse->updateButton( button, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void cursorPosCallback( GLFWwindow *window, double x, double y ) {
    PC *pc = getLocalPC();
    if(!pc)return;
    pc->mouse->updateCursorPosition( x,y);
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

    pollSpaceBG(dt);
    pollPopper(dt);
            
    last_poll_at = t;
}


void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}
void onConnectCallback( RemoteHead *rh, Client *cl ) {
    print("onConnectCallback: clid:%d",cl->id);
    addPC(cl);
}
void onRemoteKeyboardCallback( Client *cl, int kc, int act, int modshift, int modctrl, int modalt ) {
    PC *pc = getPC(cl);
    if(pc) pc->keyboard->update(kc,act,modshift,modctrl,modalt);
}
void onRemoteMouseButtonCallback( Client *cl, int btn, int act, int modshift, int modctrl, int modalt ) {
    PC *pc = getPC(cl);
    pc->mouse->updateButton( btn, act, modshift, modctrl, modalt );
}
void onRemoteMouseCursorCallback( Client *cl, int x, int y ) {
    PC *pc = getPC(cl);
    pc->mouse->updateCursorPosition(x,y);
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
    g_shoot_sig_sound = g_sound_system->newSound( "sounds/shoot_sig.wav", 0.3, false ); // Enemy shoots a fast and small missile SIG.
    g_kill_sound = g_sound_system->newSound( "sounds/machine_explo.wav", 0.3, false ); // PC shoot and destroy enemy machines.
    g_hurt_sound = g_sound_system->newSound( "sounds/hurt.wav", 0.5, false ); // PC got hurt
    g_beamhit_sound = g_sound_system->newSound( "sounds/beamhithard.wav", 0.5, false ); 
    
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


    g_moyai_client = new MoyaiClient(g_window,SCRW,SCRH);

    g_viewport = new Viewport();
    g_viewport->setSize(SCRW,SCRH); // set actual framebuffer size to output
    g_viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

    g_field_layer = new Layer();
    g_field_layer->setViewport(g_viewport);
    g_bg_layer = new Layer();
    g_bg_layer->setViewport(g_viewport);
    g_char_layer = new Layer();
    g_char_layer->setViewport(g_viewport);    
    g_effect_layer = new Layer();
    
    g_moyai_client->insertLayer(g_bg_layer);
    g_moyai_client->insertLayer(g_field_layer);    
    g_moyai_client->insertLayer(g_char_layer);
    g_moyai_client->insertLayer(g_effect_layer);    
    
    g_effect_layer->setViewport(g_viewport);
    g_base_atlas = new Texture();
    g_base_atlas->load("./images/k22base1024.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(32,42,24,24 );

    g_space_bg_tex = new Texture();
    g_space_bg_tex->load("./images/spacebg.png");
    g_planet_tex = new Texture();
    g_planet_tex->load("./images/kepler22b_blended.png");
    
    
    Texture *girevtex = new Texture();
    girevtex->load( "./images/girev64.png");
    g_girev_deck = new TileDeck();
    g_girev_deck->setTexture(girevtex);
    g_girev_deck->setSize(1,1,64,64);
    
    g_camera = new Camera();
    g_camera->setLoc(SCRW/2,SCRH/2);

    g_bg_layer->setCamera(g_camera);    
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


    // input
    glfwSetKeyCallback( g_window, keyboardCallback );
    glfwSetMouseButtonCallback( g_window, mouseButtonCallback );
    glfwSetCursorPosCallback( g_window, cursorPosCallback );

    setupSpaceBG();

    // network

    if( g_enable_network ) { 
        Moyai::globalInitNetwork();
        g_rh = new RemoteHead();
        if( g_rh->startServer(HEADLESS_SERVER_PORT) == false ) {
            print("headless server: can't start server. port:%d", HEADLESS_SERVER_PORT );
            exit(1);
        }
        g_moyai_client->setRemoteHead(g_rh);
        g_rh->setTargetMoyaiClient(g_moyai_client);
        g_sound_system->setRemoteHead(g_rh);
        g_rh->setTargetSoundSystem(g_sound_system);
        g_rh->setOnConnectCallback(onConnectCallback);
        g_rh->setOnKeyboardCallback(onRemoteKeyboardCallback);
        g_rh->setOnMouseButtonCallback(onRemoteMouseButtonCallback);
        g_rh->setOnMouseCursorCallback(onRemoteMouseCursorCallback);
    }

    // game
    PC *local_pc = new PC(NULL);
    addLocalPC(local_pc);
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
