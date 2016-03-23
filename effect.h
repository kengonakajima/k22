#ifndef _EFFECT_H_
#define _EFFECT_H_



class Particle : public Prop2D {
public:
    Vec2 v;
    float start_scl, max_scl, fin_scl;
    float attack_dur, sustain_dur;
    float base_scl;
    float gravity_effect;
    float friction;
    bool xflipping;
    bool flickering;
    int rand_inds[4];
    int rand_inds_used;

    float sway_x_vel, sway_x_width;
    float sway_xscl_vel;

    Vec2 goal;
    bool die_nearby_goal;
    
    Particle( Vec2 lc, float start_scl, float max_scl, float fin_scl, float attack_dur, float sustain_dur, int atlas_index, TileDeck *deck, float g );
    
    virtual bool prop2DPoll(double dt);
};


Particle *createExplosion(Vec2 loc, float scl, float dur_rate, bool local=true ); 


void stopEffect();


#endif
