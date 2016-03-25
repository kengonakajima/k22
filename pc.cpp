
#include "moyai/client.h"

#include "dimension.h"
#include "char.h"
#include "pc.h"
#include "atlas.h"
#include "field.h"
#include "enemy.h"
#include "globals.h"
#include "conf.h"

PC::PC(Vec2 lc) : Char(CAT_PC, lc, g_base_deck, g_char_layer ), ideal_v(0,0), dir(DIR4_DOWN), body_size(8), shoot_v(0,0), last_shoot_at(0), knockback_until(0), knockback_v(0,0), equip_prop(NULL), died_at(0), recalled_at(0), invincible_until(0) {
    tex_epsilon=0;
    priority = PRIO_CHAR;


    face_prop = new Prop2D();
    face_prop->setScl(24);
    face_prop->setDeck( g_base_deck );
    face_prop->setIndex( face_base_index = B_ATLAS_PC_FACE_FRONT);
    addChild( face_prop );

    body_prop = new Prop2D();
    body_prop->setScl(24);
    body_prop->setDeck( g_base_deck );
    body_prop->setIndex( body_base_index = B_ATLAS_PC_BODY_BASE );
    addChild( body_prop );
    
    hair_prop = new Prop2D();
    hair_prop->setScl(24);
    hair_prop->setDeck( g_base_deck );
    hair_prop->setIndex( hair_base_index = B_ATLAS_PC_HAIR_FRONT );
    addChild(hair_prop);

    equip_prop = new Prop2D();
    equip_prop->setScl(24);
    equip_prop->setDeck( g_base_deck );
    equip_prop->setIndex( B_ATLAS_PC_EQUIPMENT_BEAMGUN_BASE );
    addChild( equip_prop );

    // no main sprite
    setIndex(-1);

    body_prop->priority = 1;
    face_prop->priority = 2;
    hair_prop->priority = 3;
    equip_prop->priority = 4; // Changes depending on direction of PC
    
}

bool PC::charPoll( double dt ) {
    
    if( pcPoll(dt) == false ) return false;
    
    //
    DIR4 d = ideal_v.toDir();
    //    print("pc:%f %f ind:%d d:%d", ideal_v.x, ideal_v.y, index, d  );

    if( d != DIR4_NONE ) {
        dir = d;
        if( (dir == DIR4_RIGHT || dir == DIR4_LEFT) && ideal_v.y < 0 ) dir = DIR4_DOWN;
    }
    DIR4 shootd = shoot_v.toDir();
    if( shootd != DIR4_NONE ) {
        dir = shootd;
    }

    if( died_at == 0 && recalled_at == 0 ) {
        setIndex(-1);
        bool walking = (d != DIR4_NONE);
        body_prop->setXFlip( false );
        body_prop->setYFlip( false );
        face_prop->setXFlip( false );
        hair_prop->setXFlip( false );        
        int base_index = body_base_index;
        
        int anim_d = ( (int)(accum_time*12) % 2);
        switch(dir) {
        case DIR4_UP:
            if(walking) {
                body_prop->setIndex(base_index + 6 + anim_d);
            } else {
                body_prop->setIndex(base_index+5);
            }
            hair_prop->setIndex( hair_base_index + 2 );
            face_prop->setIndex(-1);
            body_prop->setXFlip(false);
            equip_prop->priority = 0;            
            break;
        case DIR4_DOWN:
            if(walking) {
                body_prop->setIndex(base_index+1+anim_d);
            } else {
                body_prop->setIndex(base_index+0);
            }
            hair_prop->setIndex( hair_base_index+0);
            face_prop->setIndex( face_base_index+0);
            body_prop->setXFlip(false);
            equip_prop->priority = 4;
            break;
        case DIR4_RIGHT:
            if(walking) {
                body_prop->setIndex(base_index+3+anim_d);
            } else {
                body_prop->setIndex(base_index+3);
            }
            hair_prop->setIndex( hair_base_index+1);
            hair_prop->setXFlip(true);
            face_prop->setIndex( face_base_index+1);
            face_prop->setXFlip(true);            
            body_prop->setXFlip(true);
            equip_prop->priority = 4;            
            break;
        case DIR4_LEFT:
            if(walking) {
                body_prop->setIndex(base_index+3+anim_d);
            } else {
                body_prop->setIndex(base_index+3);
            }
            hair_prop->setIndex( hair_base_index+1);
            hair_prop->setXFlip(false);
            face_prop->setIndex( face_base_index+1);
            face_prop->setXFlip(false);                        
            body_prop->setXFlip(false);
            equip_prop->priority = 0;            
            break;
        default:
            assert(false);
        }
        
        //
        float vel = PC_MAX_WALK_SPEED;
        Vec2 nextloc;
        if( accum_time < knockback_until ) {
            nextloc = loc + knockback_v * dt;
        } else {
            nextloc = loc + ideal_v * dt * vel;
        }
        Vec2 prevloc = loc;
        loc = calcNextLoc( loc, nextloc, body_size, NOT_FLYING );

        // To avoid stuck in blocks or water
        if( loc == prevloc ) {
            if( ideal_v.len() > 0 ) {
                // Try to touch wall (Assume ideal_v has length of 1)
                Vec2 finalepsloc = loc;
                for(int i=1;i<PPC/2;i++) {
                    Vec2 epsnextloc = loc + ideal_v.normalize(1*i);                    
                    Vec2 epsloc = calcNextLoc( loc, epsnextloc, body_size, NOT_FLYING );
                    if( epsloc != loc ) {
                        finalepsloc = epsloc;
                    }
                }
                if( finalepsloc != loc ) {
                    // OK could go!
                    loc = finalepsloc;                    
                } else {
                    // Try opposit direction
                    Vec2 revnextloc = loc + ideal_v * dt * vel * -1;
                    Vec2 revloc = calcNextLoc( loc, revnextloc, body_size, NOT_FLYING );
                    if( revloc == loc ) {
                        // Can't go to opposite direction. Stucked!

                        // Try center position of the nearest cell.
                        Vec2 centerloc = toCellCenter(loc);
                        Vec2 centerloctest = calcNextLoc( loc, centerloc, body_size, NOT_FLYING );
                        if( centerloctest != loc ) {
                            loc = centerloctest;
                        } 
                    }
                }
            }
        }


        // Draw equipment
        {
            int eqind = 0;
            bool xfl = false;
            Vec2 eqv = calcEquipPosition(dir);
            
            switch(dir) {
            case DIR4_DOWN:
                eqind = B_ATLAS_PC_EQUIPMENT_BEAMGUN_BASE;
                render_children_first = false;
                break;
            case DIR4_UP:
                eqind = B_ATLAS_PC_EQUIPMENT_BEAMGUN_BASE+2;
                render_children_first = true;
                break;
            case DIR4_RIGHT:
                eqind = B_ATLAS_PC_EQUIPMENT_BEAMGUN_BASE+1;
                render_children_first = false;
                break;
            case DIR4_LEFT:
                eqind = B_ATLAS_PC_EQUIPMENT_BEAMGUN_BASE+1;
                xfl = true;
                render_children_first = true;
                break;
            
            default:break;
            }
            equip_prop->setXFlip( xfl );
            equip_prop->setIndex( eqind );        
            equip_prop->setVisible(true);
            equip_prop->setLoc( loc + eqv );
        }
        body_prop->setUVRot(false);
        face_prop->setUVRot(false);
        hair_prop->setUVRot(false);
        body_prop->setColor( Color(1,1,1,1));
        face_prop->setColor( Color(1,1,1,1));
        hair_prop->setColor( Color(1,1,1,1));
    } else if( died_at > 0 && accum_time > died_at && accum_time < died_at + RESPAWN_DELAY_SEC ) {
        // Die effect
        body_prop->setIndex( body_base_index+3);
        face_prop->setIndex( face_base_index+1);
        hair_prop->setIndex( hair_base_index+1);
        body_prop->setUVRot(true);
        face_prop->setUVRot(true);
        hair_prop->setUVRot(true);
        equip_prop->setVisible(false);
        body_prop->setColor( Color(1,0.4,0.4,1) );
        face_prop->setColor( Color(1,0.4,0.4,1) );
        hair_prop->setColor( Color(1,0.4,0.4,1) );
    } else {
        if( died_at > 0 ) {
            respawn();            
        }
    }

    if( invincible_until > accum_time ) {
        bool vis = (int)(accum_time*10)%2;
        setVisible(vis);
    } else {
        setVisible(true);
    }
    

    // Adjust position
    body_prop->loc = loc;
    face_prop->loc = loc;
    hair_prop->loc = loc;

    // try to shoot
    tryShoot();
    
    return true;
}
Vec2 PC::getHandLocalLoc(Vec2 direction) {
    Vec2 handv;
    switch(direction.toDir() ) {
    case DIR4_DOWN:
        handv.x = -4;
        handv.y = -8;
        break;
    case DIR4_UP:
        handv.x = 4;
        handv.y = 8;
        break;
    case DIR4_LEFT:
        handv.x = -8;
        handv.y = -6;
        break;
    case DIR4_RIGHT:
        handv.x = 8;
        handv.y = -6;
        break;
    default:
        break;
    }
    return handv;
}

void PC::tryShoot() {
    if( shoot_v.len() == 0 ) return;
    if( last_shoot_at > accum_time - getShootIntervalSec() ) return;
    last_shoot_at = accum_time;
    Vec2 handv = getHandLocalLoc(shoot_v);
    g_shoot_sound->play();
    new Beam( loc + handv, loc + shoot_v + handv, BEAMTYPE_WIDE, B_ATLAS_WIDE_BEAM );
}

Vec2 PC::calcEquipPosition( DIR4 d ) {
    switch(d) {
    case DIR4_DOWN: return Vec2(-5,-7);
    case DIR4_UP: return Vec2(6,-4); 
    case DIR4_RIGHT: return Vec2(6,-6);
    case DIR4_LEFT: return Vec2(-6,-6);
    case DIR4_NONE: return Vec2(0,0);
    default: assert(false);
    }
    return Vec2(0,0);
}





void PC::onAttacked( int dmg, Enemy *e ) {
    if(e) {
        knockback_until = accum_time + 0.2;
        knockback_v = e->loc.to(loc).normalize(PPC*4);
    }

    if( died_at == 0 ) died_at = accum_time;
    g_hurt_sound->play();
}


void PC::respawn() {
    Pos2 p = g_fld->getRespawnPoint();
    loc = toCellCenter(p);
    died_at = 0;
    invincible_until = accum_time + 2;
}

PC *PC::getNearestPC( Vec2 from ) {
    Char *ch = Char::getNearestByCategory( from, CAT_PC, NULL );
    return (PC*)ch;
}

float PC::getShootIntervalSec() {
    return 0.2f;
}

void PC::onDelete() {
}




