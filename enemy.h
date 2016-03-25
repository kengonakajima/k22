#ifndef _ENEMY_H_
#define _ENEMY_H_

class PC;

typedef enum {
    ET_INVAL = -1,
    ET_FLY = 1,
    ET_TAKWASHI = 2,
    ET_MINE = 3,
    ET_REPAIRER = 4,
    ET_GIREV = 6,
    ET_WORM = 7,
    ET_EGG = 8,
} ENEMYTYPE;

class Enemy : public Char {
public:
    Vec2 v;    
    int hp,maxhp; 
    bool flying;

    double timeout;

    float body_size;
    CATEGORY hit_category;
    Vec2 target;
    ENEMYTYPE enemy_type;
    bool beam_hits;
    
    Enemy( Vec2 lc, TileDeck *dk, bool flying );
    ~Enemy();
    virtual bool charPoll( double dt );
    virtual bool enemyPoll( double dt ) { return true; }

    virtual void notifyHitBeam( Beam *b, int dmg );
    virtual void onBeam( Beam *b, int dmg ) {}
    virtual void onKill() {}
    virtual void onTimeout() {}
    virtual bool onWorldOut() { return false; };
    
    bool applyDamage( int dmg );
    
    inline bool hit( Prop2D *p, float sz ) {
        return ( loc.x + sz > p->loc.x - sz ) && ( loc.y + sz > p->loc.y - sz ) &&
            ( loc.x - sz < p->loc.x + sz ) && ( loc.y - sz < p->loc.y + sz );        
    }

    static Enemy *getNearestShootable( Vec2 from );

};



#define KNOCKBACK_DELAY 0.2

class Worm : public Enemy {
public:
    bool shooter;
    double shoot_at;    
    double last_hit_at;
    double rest_until;
    double turn_at;
    
    Worm( Vec2 lc );
    virtual bool enemyPoll( double dt );
    virtual void onBeam( Beam *b, int dmg );
};
class Egg : public Enemy {
public:
    double hatch_at;
    Egg(Vec2 lc);
    virtual bool enemyPoll(double dt);
};

class Fly : public Enemy {
public:
    double turn_at;
    Fly( Vec2 lc );
    virtual bool enemyPoll(double dt);
    virtual void onKill();    
};

typedef enum {
    BLT_INVAL=0,
    BLT_GOOBALL = 1,
    BLT_SPARIO,
    BLT_SIG,
} BULLETTYPE;

class Bullet : public Enemy {
public:
    BULLETTYPE bullet_type;
    float round_vel;
    float round_dia;    
    float hit_beam_size;
    float friction;
    float rot_speed;
    
    virtual bool enemyPoll( double dt );

    Bullet( BULLETTYPE blt, Vec2 at, Vec2 to );
    ~Bullet();
    static void shootAt( BULLETTYPE blt, Vec2 from, Vec2 at );
    static void shootFanAt( BULLETTYPE blt, Vec2 from, Vec2 at, int side_n, float pitch );
    Beam *checkHitBeam();
};


class Takwashi : public Enemy {
public:
    // Stop for some time and shoot fast. By this it makes a weakpoint.
    double shoot_start, shoot_end, last_shoot_at;
    Vec2 shoot_tgt;
    bool to_shoot;
    Takwashi( Vec2 lc );
    virtual bool enemyPoll(double dt);
};


class Repairer : public Enemy {
public:
    double turn_at;
    double spark_at;
    Repairer( Vec2 lc );
    virtual bool enemyPoll(double dt);
};

class Chaser : public Enemy {
public:
    double reset_target_at;
    double shoot_at;
    float thres;
    double accel_at;
    Chaser( Vec2 lc, int client_id=0, int internal_id=0 );
    virtual bool enemyPoll(double dt);
};

class Builder : public Enemy {
public:
    double turn_at;
    bool building;
    DIR build_dir;
    int build_cnt;
    double build_after_at;
    Builder( Vec2 lc, int client_id=0, int internal_id=0 );
    virtual bool enemyPoll(double dt);
    virtual void onTouchWall( Vec2 nextloc, int hitbits, bool nxok, bool nyok );    
};


class Girev : public Enemy {
public:
    double shoot_at;
    double turn_at;
    double repairer_at;
    Girev( Vec2 lc );
    virtual bool enemyPoll(double dt);
};




#endif
