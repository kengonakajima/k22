#include "moyai/client.h"
#include "dimension.h"
#include "conf.h"
#include "field.h"
#include "char.h"
#include "pc.h"
#include "atlas.h"
#include "effect.h"
#include "enemy.h"
#include "globals.h"

#include "shipdata.h"

/////////////////////

Field::Field( int w, int h ) : width(w), height(h), loc_max(w*PPC,h*PPC) {
    size_t sz = sizeof(Cell) * w * h;
    cells = (Cell*) MALLOC( sz );
    clear();
}

void Field::clear(){
    memset( cells, 0, width*height*sizeof(Cell) );
    for( int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            Cell *c = get(x,y);
            c->gt = GT_SPACE;
            c->subindex = 0;
        }
    }
    print("field init done");
}



Pos2 Field::getRespawnPoint() {
    return hatch_pos;
}

// Returns 4 bits: RT-LT-RB-LB
int Field::getEnterableBits( Vec2 at, float sz, bool flying ) {
    assertmsg( sz < PPC, "too big" );

    int out = 0;
    Cell *rtc = get( at + Vec2( sz, sz ) );
    Cell *ltc = get( at + Vec2( -sz, sz ) );        
    Cell *rbc = get( at + Vec2( sz, -sz ) );
    Cell *lbc = get( at + Vec2( -sz, -sz ) );

    if( (!rtc) || (!rtc->isEnterable(flying)) ) out += WALKABLE_BIT_HIT_RT;
    if( (!ltc) || (!ltc->isEnterable(flying)) ) out += WALKABLE_BIT_HIT_LT;
    if( (!rbc) || (!rbc->isEnterable(flying)) ) out += WALKABLE_BIT_HIT_RB;
    if( (!lbc) || (!lbc->isEnterable(flying)) ) out += WALKABLE_BIT_HIT_LB;

    return out;
}




bool Field::isValidPos( Pos2 p ) {
    if( p.x <0 || p.y < 0 || p.x >= width || p.y >= height ) return false; else return true;
}

// Get 8 pointers except center
void Field::get8(Pos2 center, Cell *out[8] ) {
    out[0] = get(center.add(0,1)); // T
    out[1] = get(center.add(1,1)); // RT
    out[2] = get(center.add(1,0)); // R
    out[3] = get(center.add(1,-1)); // RD
    out[4] = get(center.add(0,-1)); // D
    out[5] = get(center.add(-1,-1)); // LD
    out[6] = get(center.add(-1,0)); // L
    out[7] = get(center.add(-1,1)); // LT
}
void Field::get4(Pos2 center, Cell *out[4] ) {
    out[0] = get(center.add(0,1)); // T
    out[1] = get(center.add(1,0)); // R
    out[2] = get(center.add(0,-1)); // D
    out[3] = get(center.add(-1,0)); // L
}
void Field::getCorner4( Vec2 center, float sz, Cell **lb, Cell **rb, Cell **lt, Cell **rt ) {
    *lb = get( center + Vec2(-sz,-sz) );
    *rb = get( center + Vec2(sz,-sz) );
    *lt = get( center + Vec2(-sz,sz) );
    *rt = get( center + Vec2(sz,sz) );
}

// Returns true if there is something to shoot at (PC and Player buildings)
bool Field::findEnemyAttackTarget( Vec2 center, Vec2 *tgt, float char_distance, float building_distance ) {
    PC *nearestpc = PC::getNearestPC(center); // TODO: avoid simple scanning using cache
    if(nearestpc && center.len(nearestpc->loc) < char_distance ) {
        *tgt = nearestpc->loc;
        return true;
    }
    return false;
}

GROUNDTYPE charToGT( char ch ) {
    switch(ch) {
    case '.': return GT_SPACE;
    case 'x': return GT_PANEL;
    case 'Z': return GT_HATCH;
    default:
        assertmsg(false, "invalid ship data: ch:%c", ch );
    }
    return GT_SPACE;
}

void Field::generate() {
    globalInitShipData();

    int ship_h = elementof(g_ship_data);
    for(int y=0;y<ship_h;y++) {
        int ship_ind = ship_h-1-y;
        int l = strlen(g_ship_data[ship_ind]);
        for(int x=0;x<l;x++) {
            GROUNDTYPE gt = charToGT( g_ship_data[ship_ind][x] );
            Cell *c = get(x,y);
            if(!c)continue;
            c->gt = gt;
            c->subindex = 0;
            if(gt == GT_HATCH) {
                hatch_pos = Pos2(x,y);
            }
        }
        print("");
    }
}
