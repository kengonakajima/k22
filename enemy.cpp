
#include "moyai/client.h"

#include "dimension.h"
#include "util.h"
#include "atlas.h"
#include "conf.h"
#include "field.h"
#include "char.h"
#include "effect.h"
#include "enemy.h"
#include "pc.h"
#include "globals.h"

////////////////


Enemy::Enemy( Vec2 lc, TileDeck *dk, bool flying ) : Char( CAT_ENEMY, lc, dk, g_char_layer ), v(0,0), hp(1), maxhp(1), flying(flying), timeout(ENEMY_DEFAULT_TIMEOUT), body_size(8), hit_category(CAT_INVAL), enemy_type(ET_INVAL), beam_hits(true) {
    tex_epsilon = 1.0/1024.0/2.0;
    setFragmentShader(g_eye_col_replacer);
    render_children_first = true; // Shadow is always under main sprite
}
Enemy::~Enemy() {
}


bool Enemy::charPoll( double dt ) {
    if( enemyPoll(dt) == false ) {
        return false;
    }

    Vec2 nloc = loc + v * dt;

    // Enemy hits enemy. used only by Builder. Builder can't go on to other Builder.
    if( hit_category != CAT_INVAL ) {
        Char *nearest = getNearestByCategory( loc, hit_category, this );
        if(nearest ) {
            float l = nearest->loc.len(loc);
            if( l < body_size*2 ) {
                Vec2 relv = nearest->loc.to(loc).normalize(PPC*8);
                nloc += relv * dt;
            }
        }
    }
    
    Vec2 finalcand = calcNextLoc( loc, nloc, body_size, flying );
    if( finalcand == loc ) {
        // can't move..
    }
    loc = finalcand;
    
    // no flickering 
    if( timeout > 0 && accum_time > timeout ) {
        onTimeout();
        return false;
    }
    return true;
}
void Enemy::notifyHitBeam( Beam *b, int dmg ) {
    if( dmg <= 0 ) return;
    onBeam(b,dmg);
    bool killed = applyDamage(dmg);
    if(killed) {
        // Destroyed!
    }
}
// Returns true when destroyed
bool Enemy::applyDamage( int dmg ) {
    hp -= dmg ;
    
    if( hp<= 0 ) {
        createExplosion( loc, 2,3 );
        g_kill_sound->play();
        to_clean = true;
        return true;
    } else {
        return false;
    }
}
Enemy *Enemy::getNearestShootable( Vec2 from ) {
    float minl = 9999999999;
    Enemy *out = NULL;
    Char *cur = (Char*) g_char_layer->prop_top;
    while(cur) {
        if( cur->category == CAT_ENEMY ){
            Enemy *e = (Enemy*)cur;
            float l = from.len(e->loc);
            if( l < minl ) {
                minl = l;
                out = e;
            }
        }
        cur = (Char*)cur->next;
    }
    return out;
}


/////////////////////////////


Worm::Worm( Vec2 lc ) : Enemy( lc, g_base_deck, NOT_FLYING ), shooter(false), shoot_at(2), last_hit_at(0), rest_until(0), turn_at(0) {
    hp = maxhp = 2;
    setIndex( B_ATLAS_WORM_NORMAL_BASE );
    setScl( range(PPC,PPC*1.5) );
    enemy_type = ET_WORM;
}
void Worm::onBeam( Beam *b, int dmg ) {
    // knockback
    v = b->v;
    turn_at = accum_time + KNOCKBACK_DELAY;
}
        

bool Worm::enemyPoll( double dt ) {
    // Get stronger when walk on ENHANCER
    Cell *c = g_fld->get(loc);
    if(!c)return false;
    
    if( accum_time < rest_until ) {
        v *= 0;
    } else {
        if( accum_time > turn_at ) {
            Vec2 tgt;
            bool hastgt = false;
            if( g_fld->findEnemyAttackTarget(loc, &tgt, WORM_SHOOT_DISTANCE ) ) {
                v = loc.to(tgt).normalize(PPC*3);
                hastgt = true;
            } else {
                v = Vec2(0,0).randomize(1).normalize(PPC*2);
            }

            turn_at = accum_time + ( hastgt ? 1 : 3 );
        }
    }

    // Shoot
    if( accum_time > shoot_at ) {
        if( shooter && range(0,100) < 20 ) {
            new Bullet( BLT_GOOBALL, loc, g_pc->loc );
            v *= 0;
            shoot_at = accum_time + range(1,3);
        } 
    }
    updateDefaultRightFacedRot(v);

    // Direct damage on touching PC
    if( hit(g_pc,PPC/4) && last_hit_at < accum_time - 0.5 ) {
        last_hit_at = accum_time;
        turn_at = accum_time + KNOCKBACK_DELAY;
        int dmg = 1;
        g_pc->onAttacked(dmg, this);
        v = loc.to(g_pc->loc).normalize( -PPC * 10 );
        //        g_wormbite_sound->play();
    }
    return true;
}


//////////////////////////

Egg::Egg(Vec2 lc ) : Enemy( lc, g_base_deck, NOT_FLYING ), hatch_at(7) {
    setIndex( B_ATLAS_WORM_EGG );
    hp = maxhp = 1;
    setScl(PPC);
    v = Vec2(0,0);
    timeout = 999; // Must hatch before timeout
    setRot( irange(0,M_PI*2));
    enemy_type = ET_EGG;
}

bool Egg::enemyPoll(double dt) {
    v *= 0.95;
    if( accum_time > hatch_at ) {
        new Worm( loc );
        return false;
    }
    return true;
}

/////////////////////////
Fly::Fly( Vec2 lc ) : Enemy( lc, g_base_deck, FLYING ), turn_at(1) {
    hp = maxhp = 1;
    setScl(PPC*1.5);
    v = Vec2(0,0);
    timeout = ENEMY_DEFAULT_TIMEOUT / 2;
    v = Vec2(0,0).randomize(PPC*2);
    enemy_type = ET_FLY;
}
bool Fly::enemyPoll(double dt) {
    setIndex( B_ATLAS_FLY_BASE + ((int)(accum_time * 10) % 2) );
    if(accum_time > turn_at ) {
        turn_at = accum_time + range(1,2);
        Vec2 tgt;
        if( g_fld->findEnemyAttackTarget(loc,&tgt,WORM_SHOOT_DISTANCE)) {
            target = tgt;
        } else {
            target = loc.randomize(PPC*10);
        }
    }
    //
    float vel = PPC * 5;
    Vec2 vv = loc.to(target).normalize(vel);
    v += vv * dt;
    v -= v * dt * 0.5;

    if( loc.len(g_pc->loc) < PPC/2 ) {
        to_clean = true;
        g_pc->onPushed(1,this);        
    }
    return true;
}




/////////////////////

Bullet::Bullet( BULLETTYPE blt, Vec2 at, Vec2 to ) : Enemy( at, g_base_deck, FLYING ), bullet_type(blt), round_vel(0), round_dia(0), hit_beam_size(0), friction(0), rot_speed(0) {
    beam_hits = false;
    clean_at = 20;
    setIndex(0);
    float vel = PPC;
    timeout = 10;
    
    bool to_rot = false;
    bool to_quantize = true;

    switch(blt) {
    case BLT_GOOBALL:
        vel = PPC * range(3,6);
        setIndex( B_ATLAS_GOOBALL_BASE );
        setScl( range( PPC, PPC*2));
        clean_at = 3;
        beam_hits = false;
        break;
    case BLT_SPARIO:
        vel = PPC * 4;
        setIndex( B_ATLAS_SPARIO );
        setScl( PPC*1.5, PPC*1.5 );
        beam_hits = false;
        break;
    case BLT_SIG:
        category = CAT_ENEMY; // This bullet can be destroyed by beam
        to_rot = true;
        hp = 1;
        vel = PPC * 8;
        setIndex( B_ATLAS_SIG );
        setScl( PPC*1.5, PPC*1.5 );
        hit_beam_size = PPC/3;
        g_shoot_sig_sound->play();
        break;
    default:
        assertmsg( false, "invalid bullettype:%d", blt);
    }
    
    v = at.to(to).normalize(vel);

    if( to_quantize ) v = quantizeAngle(v, 32 );

    if( to_rot ) updateDefaultRightFacedRot(v);

}
Bullet::~Bullet() {
}
bool Bullet::enemyPoll( double dt ) {

    if( friction > 0 ) {
        v *= friction;
    }
        
    if( round_vel > 0 ) {
        loc += Vec2( cos( accum_time*round_vel), sin(accum_time*round_vel) ) * round_dia * dt;
    }

    if( rot_speed > 0 ) {
        setRot( accum_time * rot_speed );
    }

    if( loc.len(g_pc->loc) < PPC/3 ) {
        g_pc->onAttacked(1, this);
        return false;
    }

    if( bullet_type == BLT_GOOBALL ) {
        float s = 1 + sin( accum_time * 20 ) * 0.4;
        setScl(s*PPC*2);
    }
    
    return true;
}


void Bullet::shootAt( BULLETTYPE blt, Vec2 from, Vec2 at ) {
    new Bullet(blt,from,at);
}

void Bullet::shootFanAt( BULLETTYPE blt, Vec2 from, Vec2 at, int side_n, float pitch ) {
    Bullet *center = new Bullet(blt, from,at);
    float rad = atan2( center->v.y, center->v.x );
    
    for(int i=0;i<side_n;i++) {
        Vec2 sidev( cos(rad + pitch*(i+1)), sin(rad + pitch*(i+1)) );
        Bullet *b = new Bullet(blt,from, from + sidev.normalize( center->v.len() ) );

        sidev = Vec2( cos(rad - pitch*(i+1)), sin(rad - pitch*(i+1)) );
        b = new Bullet(blt, from, from + sidev.normalize( center->v.len() ) );
    }
}

Beam *Bullet::checkHitBeam() {
    Char *cur = (Char*) g_char_layer->prop_top;
    while(cur) {
        if( cur->category == CAT_BEAM ) {
            if( cur->hit(this,hit_beam_size) ) {
                return (Beam*)cur;                
            }
        }
        cur = (Char*) cur->next;
    }
    return NULL;
}


//////////////


Takwashi::Takwashi( Vec2 lc ) : Enemy( lc, g_base_deck, FLYING ), shoot_start(0), shoot_end(0), last_shoot_at(0), to_shoot(false) {
    setIndex( B_ATLAS_TAKWASHI );
    v = Vec2(0,0);
    hp = maxhp = 25;
    setFragmentShader(g_eye_col_replacer);
    enemy_type = ET_TAKWASHI;
}
bool Takwashi::enemyPoll(double dt) {
    if( shoot_start == 0 ) {
        shoot_start = accum_time + 2;
        shoot_end = accum_time + 4;
    } else {
        if( accum_time > shoot_end ) {
            shoot_start = shoot_end = 0;
        }
    }
    if( accum_time > shoot_start && accum_time < shoot_end ) {
        if( v.len() > 0 ) {
            Vec2 tgt;
            if( (to_shoot = g_fld->findEnemyAttackTarget(loc, &tgt, TAKWASHI_SHOOT_DISTANCE)) ) {            
                shoot_tgt = tgt;
                updateDefaultRightFacedRot(loc.to(tgt));    
            } 
        }
        float intvl = 0.1;
        if( accum_time > last_shoot_at + intvl) {
            last_shoot_at = accum_time;
            if( to_shoot ) Bullet::shootAt( BLT_SIG, loc, shoot_tgt );
        }
        v = Vec2(0,0);
    } else {
        if( v.len() == 0 ) {
            Vec2 tgt;
            if( g_fld->findEnemyAttackTarget( loc, &tgt, TAKWASHI_SHOOT_DISTANCE ) ) {
                float vel = PPC;
                v = loc.to(tgt).normalize(vel);
                updateDefaultRightFacedRot(v);    
            }
        }
    }
    return true;
}



///////////////////////
Girev::Girev( Vec2 lc ) :Enemy( lc, g_girev_deck, FLYING ), shoot_at(0), turn_at(0), repairer_at(3) {
    setIndex(0);
    v = Vec2(0,0);
    hp = maxhp = 500;
    setScl(64,64);
    timeout = ENEMY_DEFAULT_TIMEOUT * 100;
    enemy_type = ET_GIREV;
}

bool Girev::enemyPoll(double dt) {
    if( accum_time > turn_at) {
        float intvl = 2;
        turn_at = accum_time + intvl;
        Vec2 tgt;
        if( g_fld->findEnemyAttackTarget( loc, &tgt, MACHINE_SHOOT_DISTANCE) ) {
            float vel = PPC;
            v = loc.to(tgt).normalize(vel);
            Bullet::shootFanAt( BLT_SPARIO, loc, tgt, 8, M_PI/8.0 );
        }
    }
    if( accum_time > repairer_at ) {
        float intvl = 2;
        repairer_at = accum_time + intvl;
        new Repairer(loc);
    }
    return true;
}
///////////////////////
Repairer::Repairer( Vec2 lc ) : Enemy(lc, g_base_deck, FLYING ), turn_at(0), spark_at(0) {
    setIndex( B_ATLAS_REPAIRER_BASE );
    v = Vec2(0,0);
    hp = maxhp = 20;
    setScl(24*1.5);
    timeout = ENEMY_DEFAULT_TIMEOUT * 2;
    enemy_type = ET_REPAIRER;
}
bool Repairer::enemyPoll(double dt) {
    setIndex( B_ATLAS_REPAIRER_BASE + (((int)(accum_time*30) % 4)));

    if( accum_time > turn_at ) {
        float intvl = 3;
        float vel = PPC * 2;
        turn_at = accum_time + intvl;
        v = Vec2(0,0).randomize(1).normalize(vel);
    }
    return true;    
}

