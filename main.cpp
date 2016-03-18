#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "client.h"


MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_main_layer;
Texture *g_base_atlas;
TileDeck *g_base_deck;
Camera *g_camera;
ColorReplacerShader *g_replacer_shader;
SoundSystem *g_sound_system;

int g_last_render_cnt ;

GLFWwindow *g_window;

static const int SCRW=1024, SCRH=768;




void gameUpdate(void) {
    static double last_print_at = 0;
    static int frame_counter = 0;
    static double last_poll_at = now();

    double t = now();
    double dt = t - last_poll_at;
    
    frame_counter ++;
    
    int cnt;
    cnt = g_moyai_client->poll(dt);

    if(last_print_at == 0){
        last_print_at = t;
    } else if( last_print_at < t-1 ){
        fprintf(stderr,"FPS:%d prop:%d render:%d\n", frame_counter, cnt, g_last_render_cnt  );
        frame_counter = 0;
        last_print_at = t;
    }

    // replace white to random color
    g_replacer_shader->setColor( Color(0xF7E26B), Color( range(0,1),range(0,1),range(0,1),1), 0.02 );

    if( glfwGetKey( g_window, 'Q') ) {
        print("Q pressed");
        exit(0);
    }
    
    glfwPollEvents();        
        
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

    // shader
    g_replacer_shader = new ColorReplacerShader();
    if( !g_replacer_shader->init() ){
        print("can't initialize shader");
        exit(1);
    }

    g_moyai_client = new MoyaiClient(g_window);

    g_viewport = new Viewport();
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    g_viewport->setSize(SCRW*retina,SCRH*retina); // set actual framebuffer size to output
    g_viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

    g_main_layer = new Layer();
    g_moyai_client->insertLayer(g_main_layer);
    g_main_layer->setViewport(g_viewport);

    g_base_atlas = new Texture();
    g_base_atlas->load("./assets/base.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(32,32,8,8 );
    
    g_camera = new Camera();
    g_camera->setLoc(0,0);

    g_main_layer->setCamera(g_camera);

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
