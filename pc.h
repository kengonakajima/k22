#ifndef _PC_H_
#define _PC_H_

class Debris;
class Enemy;


class PC : public Char {
public:
    Vec2 ideal_v; // 1 per PPC
    DIR4 dir;
    float body_size;
    Vec2 shoot_v;
    double last_shoot_at;

    double knockback_until;
    Vec2 knockback_v;

    // no main sprite.
    Prop2D *equip_prop; // Draw order depends on move direction.
    Prop2D *body_prop; // 
    Prop2D *face_prop; //
    Prop2D *hair_prop; //

    double died_at;
    double recalled_at;
    
    // Customize look and sounds 
    int hair_base_index, face_base_index, body_base_index;

    double invincible_until;
    
    PC( Vec2 lc );
    virtual bool charPoll( double dt );

    virtual void onPushed( int dmg, Enemy *e ) {};
    virtual bool pcPoll( double dt ) { return true; }

    
    void onAttacked(int dmg, Enemy *e );

    static PC *getNearestPC( Vec2 from );

    float getShootIntervalSec();
    void respawn();
    
    static Vec2 calcEquipPosition(DIR4 d);

    virtual void onDelete();

    void tryShoot();
    Vec2 getHandLocalLoc( Vec2 direction );
};


#endif
