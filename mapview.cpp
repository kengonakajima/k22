
#include "moyai/client.h"
#include "mapview.h"
#include "dimension.h"
#include "field.h"
#include "globals.h"
#include "atlas.h"
#include "conf.h"

MapView::MapView(int w, int h ) : Prop2D() {
    setIndex(-1);
    grid = new Grid(w,h);
    grid->setDeck(g_base_deck);
    setScl(24,24);
    addGrid(grid);
}
MapView::~MapView() {
    if(grid) delete grid;
}
int groundTypeToBaseIndex( GROUNDTYPE gt, int subind ) {
    switch(gt) {
    case GT_SPACE: return Grid::GRID_NOT_USED;
    case GT_PANEL: return B_ATLAS_SHIP_PANEL;
    case GT_HATCH: return B_ATLAS_SHIP_HATCH_BASE + subind;
    default:
        assertmsg(false, "invalid gt:%d",gt);
        break;        
    }
}
void MapView::update(Field *f) {
    for(int y=0;y<f->height;y++) {
        for(int x=0;x<f->width;x++) {
            Cell *c = f->get(x,y);
            assert(c);
            int base_ind = groundTypeToBaseIndex(c->gt,c->subindex);
            if(c->gt == GT_HATCH ) print("HOGE: %d", base_ind);
            grid->set(x,y,base_ind);
        }
    }
}
bool MapView::prop2DPoll(double dt) {
    return true;
}

//////////////////////////

Prop2D *g_bg[2];
Prop2D *g_planet;

void setupSpaceBG() {
    for(int i=0;i<2;i++) {
        g_bg[i] = new Prop2D();
        g_bg[i]->setTexture(g_space_bg_tex);
        g_bg[i]->setScl(SCRW+8,SCRH); // +8 for filling gap
        float dx = SCRW * i;
        g_bg[i]->setLoc(SCRW/2-dx,SCRH/2);
        g_bg_layer->insertProp(g_bg[i]);
    }
    g_planet = new Prop2D();
    g_planet->setTexture(g_planet_tex);
    g_planet->setScl( g_planet_tex->getSize() *2);
    g_planet->setLoc(100,100);
    g_bg_layer->insertProp(g_planet);
}
void pollSpaceBG(double dt) {
    for(int i=0;i<2;i++) {
        if( g_bg[i]->loc.x > SCRW+SCRW/2 ) g_bg[i]->loc.x = -SCRW/2;
        g_bg[i]->loc.x += 6 * dt;
    }
    g_planet->loc.x += 12 * dt;
    if( g_planet->loc.x > SCRW/2 + g_planet->scl.x ) g_planet->loc.x = -SCRW;
}

