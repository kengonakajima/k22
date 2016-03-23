#include "moyai/client.h"

#include "conf.h"
#include "dimension.h"
#include "field.h"
#include "char.h"
#include "enemy.h"
#include "atlas.h"
#include "pc.h"
#include "effect.h"
#include "globals.h"

Char::Char( CATEGORY cat, Vec2 lc, TileDeck *dk, Layer *tgtlayer ) : Prop2D(), category(cat), clean_at(0) {
    setScl(24,24);
    setDeck(dk);
    setLoc(lc);
    tgtlayer->insertProp(this);
}
Char::~Char() {
}

bool Char::prop2DPoll( double dt ) {
    if( charPoll(dt) == false ) return false;
    if( clean_at > 0 && accum_time > clean_at ) return false;
    if( loc.x < 0 ||  loc.x > g_fld->loc_max.x || loc.y < 0 || loc.y > g_fld->loc_max.y  ) {
        return onWorldOut();
    }
    return true;
}
Vec2 Char::calcNextLoc( Vec2 from, Vec2 nextloc, float body_size, bool flying ) {
    Vec2 out = nextloc;
    int hitbits;
    if( (hitbits=g_fld->getEnterableBits(nextloc, body_size, flying )) > 0 ) {
        
        out = from;
        // Try X and Y independently
        Vec2 nxloc( nextloc.x, from.y );
        bool nxok=false, nyok=false;
        if( g_fld->getEnterableBits(nxloc, body_size, flying ) == 0 ) {
            nxok = true;
        }
        Vec2 nyloc( from.x, nextloc.y );
        if( g_fld->getEnterableBits(nyloc, body_size, flying ) == 0 ) {
            nyok = true;
        }

        if( nxok ) out.x = nxloc.x; 
        if( nyok ) out.y = nyloc.y;

        // hit on something.
        onTouchWall( nextloc, hitbits, nxok, nyok );
        
        return out;
    } else {
        return nextloc;
    }
}

void Char::selectByCategory( Vec2 center, float dia, CATEGORY cat, Char *out[], int *outlen ) {
    Prop *hits[1024];
    int hitlen = elementof(hits);
    g_char_layer->selectCenterInside( center,dia, hits, &hitlen );
    int cnt=0;
    for(int i=0;i<hitlen;i++){
        Char*ch = (Char*) hits[i];
        if(ch->category == cat ){
            out[cnt++] = ch;
            if(cnt==*outlen)break;
        }
    }
    *outlen = cnt;
}

Char *Char::getNearestByCategory( Vec2 from, CATEGORY cat, Char *except ) {
    float minl = 9999999999;
    Char *out = NULL;
    Char *cur = (Char*) g_char_layer->prop_top;
    while(cur) {
        if( cur->category == cat && cur != except ) {
            float l = from.len(cur->loc);
            if( l < minl ) {
                minl = l;
                out = cur;
            }
        }
        cur = (Char*)cur->next;
    }
    return out;
}


/////////////////
Beam::Beam( Vec2 lc, Vec2 at, BEAMTYPE beamtype, int base_index ) : Char( CAT_BEAM, lc, g_base_deck, g_char_layer ), type(beamtype) {

    switch(beamtype) {
    case BEAMTYPE_WIDE:
        v = lc.to(at).normalize( BEAM_NORMAL_VEL );
        clean_at = 1.5f;
        setIndex( base_index );
        break;
    case BEAMTYPE_LASER:
    case BEAMTYPE_RIPPLE:        
        assertmsg(false, "not impl");
        break;
    }


    setRot(atan2( v.y, v.x ));
    setColor(WHITE);

    updateIndex();
    tex_epsilon = DEFAULT_TEX_EPS;

    hitsz = 4;
}

void Beam::updateIndex() {
    switch(type) {
    case BEAMTYPE_WIDE:
        break;
    case BEAMTYPE_LASER:
        break;
    case BEAMTYPE_RIPPLE:
        break;
    }
}



void Beam::createSparkEffect() {
    for(int i=0;i<5;i++){
        switch(type) {
        case BEAMTYPE_WIDE:
            {
                Particle *e = new Particle( loc, range(0.5,1),range(0.5,1),0, 0.2,0, index+AU, g_base_deck, true );
                e->v = (v*0.7).randomize(200);
            }
            break;
        case BEAMTYPE_LASER:
        case BEAMTYPE_RIPPLE:
            break;
        }
    }    
}


bool Beam::charPoll( double dt ) {
    loc += v * dt;
    
    updateIndex();

    // Shoot on enemies
    Char *cur = (Char*) g_char_layer->prop_top;
    while(cur) {
        if( cur->isEnemyCategory() ) {
            Enemy *e = (Enemy*) cur;
            if( e->hit(this,PPC/2) && e->beam_hits ) {
                int dmg = 1;
                if( dmg > e->hp ) dmg = e->hp;
                e->notifyHitBeam(this, dmg);
                createSparkEffect();
                //
                to_clean = true; // TODO: implement ripple and laser
            }
        }
        cur = (Char*) cur->next;
    }
    return true;
}


///////////////////////

// div_num : Divide a circle(360deg) by this number
Vec2 quantizeAngle( Vec2 v, int div_num ) {
    float l = v.len();
    float r = atan2(v.y,v.x);
    float unit = M_PI / (float)(div_num/2);    
    float positive_r = M_PI + r + (unit/2.0); // from -PI to PI. (0 ~ 2*M_PI)
    int ir = (int)(positive_r / unit) % div_num;
    float quantized_r = ir * unit - M_PI;

    return Vec2( cos(quantized_r), sin(quantized_r) ) * l;
}
