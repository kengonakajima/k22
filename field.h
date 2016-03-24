#ifndef _FIELD_H_
#define _FIELD_H_

#define PIXEL_PER_CELL 24
#define PPC PIXEL_PER_CELL

#define CHUNKSZ 16

typedef enum  {
    FTS_NOT_EYE = 0,
    FTS_IDLE = 1,
    FTS_ACTIVE = 2,
    FTS_MAD = 3,
} FTSTATE;

typedef enum {
    OPENNESS_CLOSED = 0,
    OPENNESS_ALMOST_CLOSED = 1,
    OPENNESS_ALMOST_OPEN = 2,
    OPENNESS_OPEN = 3,
} OPENNESS;

typedef enum {
    GT_SPACE = 0, // subindex == 0
    GT_PANEL = 1, // use subindex
    GT_HATCH = 2, // use subindex
} GROUNDTYPE;
    
class Cell {
public:
    GROUNDTYPE gt;
    int subindex;

    inline bool isWalkable() {
        return gt == GT_PANEL || gt == GT_HATCH;
    }
    inline bool isFlyable() {
        return true;
    }
    inline bool isEnterable(bool flying ) {
        if( flying ) return isFlyable(); else return isWalkable();
    }
};


class Field {
public:
    int width, height;
    Vec2 loc_max;
    Cell *cells;
    Pos2 hatch_pos;

    Field( int w, int h );
    void clear();
    inline Cell *get( int x, int y) {
        if( x<0 || y<0 || x >= width || y >= height ) return NULL;
        return & cells[ x + y * width ];
    }
    inline Cell *get( Pos2 p ) {
        return get( p.x, p.y );
    }
    inline void getCellXY(Cell *c, int *x, int *y ) { 
        int d_addr = ((char*)c) - ((char*) &cells[0] );
        int ind = d_addr / sizeof(Cell);
        assertmsg( ind >= 0 && ind < width*height, "getCellXY: the cell is not in this field?" );
        int mod = d_addr % sizeof(Cell);
        assert(mod==0);
        *x = ind % width;
        *y = ind / width;
    }
    inline Pos2 getCellPos(Cell *c) {
        int x,y;
        getCellXY(c,&x,&y);
        return Pos2(x,y);
    }
    inline Cell *get( Vec2 v ) {
        return get( (int)v.x/PPC, (int)v.y/PPC);
    }
    inline Cell *getDir4( Cell *c, DIR4 d ) {
        int x,y;
        getCellXY( c, &x, &y );
        int dx,dy;
        dirToDXDY( d, &dx, &dy );
        return get( Pos2( x+dx,y+dy) );
    }
    inline Cell *getDia( Vec2 center, float dia ) {
        Vec2 rel = Vec2(0,0).randomize( dia * 1.414 ).normalize( range(0,dia) );
        return get(center+rel);
    }

    Pos2 getRespawnPoint();
    
    //
    static const int WALKABLE_BIT_HIT_RT = 8;
    static const int WALKABLE_BIT_HIT_LT = 4;
    static const int WALKABLE_BIT_HIT_RB = 2;
    static const int WALKABLE_BIT_HIT_LB = 1;
    
    int getEnterableBits( Vec2 at, float sz, bool flying );

    bool isValidPos( Pos2 p );
    void get8(Pos2 center, Cell *out[8] );
    void get4(Pos2 center, Cell *out[4] );
    void getCorner4( Vec2 center, float sz, Cell **lb, Cell **rb, Cell **lt, Cell **rt );
    void getRectCorner(Vec2 center, float sz, Cell *out[4]);

    bool findEnemyAttackTarget( Vec2 center, Vec2 *tgt, float distance, float building_target = 15 );

    void generate();
};



inline Pos2 vec2ToPos2( Vec2 v ) {
    return Pos2( v.x / PPC, v.y / PPC );
}

inline Vec2 pos2ToVec2( Pos2 p ) {
    return Vec2( p.x * PPC, p.y * PPC );
}
inline Vec2 toCellCenter( Vec2 at ) {
    return Vec2( (int)(at.x / PPC) * PPC + PPC/2,
                 (int)(at.y / PPC) * PPC + PPC/2
                 );
}
inline Vec2 toCellCenter( Pos2 p ) {
    return toCellCenter( Vec2( p.x*PPC, p.y*PPC) );
}
inline Vec2 toCellCenter( int x, int y ) {
    return toCellCenter( Vec2(x*PPC,y*PPC) );
}

inline Vec2 randomDirToVec2() {
    int dx,dy;
    dirToDXDY( randomDir(), &dx, &dy );
    return Vec2(dx,dy);
}
inline Vec2 dirToVec2( DIR4 d ) {
    int dx,dy;
    dirToDXDY(d, &dx, &dy );
    return Vec2(dx,dy);
}
inline Pos2 dirToPos2( DIR4 d ) {
    int dx,dy;
    dirToDXDY(d, &dx, &dy );
    return Pos2(dx,dy);
}
inline DIR4 vec2ToDir( Vec2 v ) {
    if( v.x == 0 && v.y == 0 ) return DIR4_NONE;
    float r = atan2( v.y, v.x ); // 0~M_PI, 0~-M_PI
    if( r >= M_PI*-0.25 && r <= M_PI*0.25 ) return DIR4_RIGHT;
    if( r >= M_PI*0.25 && r <= M_PI*0.75 ) return DIR4_UP;
    if( r >= M_PI*0.75 || r <= M_PI*-0.75 ) return DIR4_LEFT;
    if( r <= M_PI*0.25 && r >= M_PI*-0.75 ) return DIR4_DOWN;
    return DIR4_NONE;
}

#endif
