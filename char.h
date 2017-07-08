#ifndef _CHAR_H_
#define _CHAR_H_

#define FLYING true
#define NOT_FLYING false


typedef enum {
    CAT_INVAL,
    CAT_PC,
    CAT_ENEMY,
    CAT_BEAM,
    CAT_PC_PART,
} CATEGORY;



typedef enum {
    PRIO_CHAR = 20, 
} PRIORITY;


class Char : public Prop2D {
public:
    CATEGORY category;
    double clean_at;

    Char( CATEGORY cat, Vec2 lc, TileDeck *dk, Layer *tgtlayer );

    Vec2 calcNextLoc( Vec2 from, Vec2 nextloc, float body_size, bool flying );
    void selectByCategory( Vec2 center, float dia, CATEGORY cat, Char *out[], int *outlen );
    static Char *getNearestByCategory( Vec2 from, CATEGORY cat, Char *except );
    inline bool hit( Prop2D *p, float sz ) {
        return ( loc.x + sz > p->loc.x - sz ) && ( loc.y + sz > p->loc.y - sz ) &&
            ( loc.x - sz < p->loc.x + sz ) && ( loc.y - sz < p->loc.y + sz );
    }
    ~Char();
    
    virtual bool prop2DPoll( double dt );
    virtual bool charPoll( double dt ){ return true; }

    // Rotate a sprite (Pixel art have to face right)
    inline void updateDefaultRightFacedRot( Vec2 v ) {
        float r = atan2(v.y,v.x);
        setRot(r);
    }
    inline bool isEnemyCategory() {
        return (category == CAT_ENEMY);
    }
    virtual bool onWorldOut() {return false;} // REturn false to clean this character, true to keep
};


typedef enum {
    BEAMTYPE_WIDE = 0,
    BEAMTYPE_LASER = 1,
    BEAMTYPE_RIPPLE = 2,
} BEAMTYPE;

class Beam : public Char {
public:
    Vec2 v;
    BEAMTYPE type;
    float hitsz;
    int ene;
    Beam( Vec2 lc, Vec2 at, BEAMTYPE beamt, int base_index );
    virtual bool charPoll( double dt );
    void updateIndex();
    void createSparkEffect();
};

Prop2D *createShadow( Vec2 lc );
Vec2 quantizeAngle( Vec2 v, int div_num );


#endif
