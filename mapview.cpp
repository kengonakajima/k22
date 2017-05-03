
#include "moyai/client.h"
#include "dimension.h"
#include "globals.h"
#include "field.h"
#include "mapview.h"

#include "atlas.h"
#include "conf.h"

////////////////////////////

class Chunk : public Prop2D {
public:
    static const int SZ=8;
    Grid *g;
    int chx,chy;
    Chunk( int chx, int chy ) : Prop2D(),chx(chx),chy(chy) {
        g = new Grid(SZ,SZ);
        addGrid(g);
        g->setDeck(g_base_deck);
        setScl(24);
        setLoc(chx*SZ*24,chy*SZ*24);
        g_field_layer->insertProp(this);
    }
    void loadCell(Field *f) {
        for(int y=0;y<SZ;y++) {
            for(int x=0;x<SZ;x++) {
                Cell *c = f->get(chx*SZ+x,chy*SZ+y);
                g->set(x,y,groundTypeToBaseIndex(c->gt));
            }
        }
    }
    virtual bool prop2DPoll(double dt) {
        return true;
    }
};


////////////////////////////

MapView::MapView(int w, int h ) {
    chw = w/CHUNKSZ;
    chh = h/CHUNKSZ;
    chunks = (Chunk**)MALLOC( sizeof(Chunk*) * chw * chh );
    for(int chy=0;chy<chh;chy++) {
        for(int chx=0;chx<chw;chx++) {
            Chunk *ch = new Chunk(chx,chy);
            chunks[ chunkIndex(chx,chy) ] = ch;
        }
    }
}
MapView::~MapView() {
}
int groundTypeToBaseIndex( GROUNDTYPE gt ) {
    switch(gt) {
    case GT_SPACE: return Grid::GRID_NOT_USED;
    case GT_PANEL: return B_ATLAS_SHIP_PANEL;
    case GT_HATCH: return B_ATLAS_SHIP_HATCH_BASE;
    default:
        assertmsg(false, "invalid gt:%d",gt);
        break;        
    }
}
void MapView::update(Field *f) {
    for(int chy=0;chy<chh;chy++) {
        for(int chx=0;chx<chw;chx++) {
            Chunk *ch = chunks[ chunkIndex(chx,chy) ];
            ch->loadCell(f);
        }
    }
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
        g_bg[i]->setLocSyncMode(LOCSYNCMODE_LINEAR);
        g_bg_layer->insertProp(g_bg[i]);
    }
    g_planet = new Prop2D();
    g_planet->setTexture(g_planet_tex);
    g_planet->setScl( g_planet_tex->getSize() *2);
    g_planet->setLoc(100,100);
    g_planet->setLocSyncMode(LOCSYNCMODE_LINEAR);
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

