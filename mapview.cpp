
#include "moyai/client.h"
#include "mapview.h"
#include "dimension.h"
#include "field.h"
#include "globals.h"
#include "atlas.h"

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
