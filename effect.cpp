
#include "moyai/client.h"

#include "effect.h"
#include "atlas.h"
#include "conf.h"
#include "dimension.h"
#include "globals.h"


Particle::Particle( Vec2 lc, float start_scl, float max_scl, float fin_scl, float attack_dur, float sustain_dur, int atlas_index, TileDeck *deck , float g ) : Prop2D(), start_scl(start_scl), max_scl(max_scl), fin_scl(fin_scl), attack_dur(attack_dur), sustain_dur(sustain_dur), gravity_effect(g), friction(0), xflipping(false), flickering(false), rand_inds_used(0), sway_x_vel(0), sway_x_width(0), sway_xscl_vel(0), die_nearby_goal(false) {
    setLoc(lc);
    setDeck(deck);
    setIndex(atlas_index);

    setScl( start_scl, start_scl );
    base_scl = 24;
    g_effect_layer->insertProp(this);
}

bool Particle::prop2DPoll(double dt){
    //        print("expl: scl:%f %d %f", scl.x, id , accumTime );
    float rate = 1;
    if( accum_time < attack_dur ){
        rate = start_scl + ( accum_time / attack_dur ) * ( max_scl - start_scl );
    } else if( accum_time < attack_dur + sustain_dur ){
        float t = accum_time - attack_dur;
        rate = max_scl + ( t / sustain_dur ) * ( fin_scl - max_scl );
    }
    scl.y = scl.x = rate * base_scl;
    if( accum_time > ( attack_dur + sustain_dur ) ){
        return false;
    }
    v.y = v.y - gravity_effect * dt;

    if( friction != 0 ){
        v = v.friction(friction * dt);
    }
    loc += v * dt;
    if( xflipping ) setXFlip( irange(0,2) );
    if( flickering ) setVisible( irange(0,2) );
    if( rand_inds_used > 0 ) {
        setIndex( rand_inds[ irange(0,rand_inds_used) ] );
    }
    if( sway_x_vel > 0 ) {
        float d = cos(accum_time * sway_x_vel) * sway_x_width * dt;
        loc.x += d;
    }
    if( sway_xscl_vel > 0 ) {
        Vec2 s = scl;
        setScl( absolute( s.x * cos(accum_time * sway_xscl_vel) ), s.y );
    }
    if( die_nearby_goal && loc.len(goal) < 8 ) return false;
    return true;
}



Particle *createExplosion(Vec2 loc, float scl, float dur_rate ){
    Particle *e = new Particle(loc,1*scl,2*scl,0, 0.05 * dur_rate,0.1*dur_rate, B_ATLAS_EXPLOSION, g_base_deck, 0 );
    return e;
}



