#ifndef _DIMENSION_H_
#define _DIMENSION_H_

class Pos2 {
 public:
    int x;
    int y;
    inline Pos2() : x(0),y(0){}
    inline Pos2(int _x,int _y): x(_x), y(_y) {}
    inline Pos2(const Pos2& p ) : x(p.x), y(p.y){}
    inline Pos2(Vec2 v){ x = v.x; y = v.y; }
    inline void clear() { x=y=0; }
    inline void add(Pos2 d){ this->x += d.x; this->y += d.y; }
    inline Pos2 right(){ return Pos2( this->x+1, this->y ); }
    inline Pos2 left(){ return Pos2( this->x-1, this->y ); }
    inline Pos2 down(){ return Pos2( this->x, this->y-1 ); }
    inline Pos2 up(){ return Pos2( this->x, this->y+1 ); }
    inline Pos2 add(int dx,int dy) { return Pos2(this->x+dx,this->y+dy); }
    inline Pos2 to( Pos2 tgt ) { return Pos2( tgt.x - this->x, tgt.y - this->y ); }
    inline bool equals(Pos2 &p){ return ( this->x == p.x && this->y == p.y ); }
    inline bool valid(){ return ( this->x >= 0 && this->y >= 0 ); }
    static inline Pos2 getInvalid(){ return Pos2(-1,-1); }
    static inline Pos2 fromDir( DIR4 d ) {
        switch(d){
        case DIR4_UP: return Pos2( 0, 1 ); 
        case DIR4_RIGHT: return Pos2(1, 0 );
        case DIR4_DOWN: return Pos2(0,-1);
        case DIR4_LEFT: return Pos2(-1,0);
        default:
            assert(false);
            return Pos2(0,0);
        }
    }
    inline float len(Pos2 to){
        return ::len( x,y,to.x,to.y );
    }
    inline Pos2 operator+=(Pos2 arg) { return Pos2( this->x += arg.x, this->y += arg.y); }
    inline Pos2 operator-=(Pos2 arg) { return Pos2( this->x -= arg.x, this->y -= arg.y); }    
    inline bool operator==(Pos2 arg) { return ( this->x == arg.x && this->y == arg.y ); }
    inline bool operator!=(Pos2 arg) { return ( this->x != arg.x || this->y != arg.y ); }
    inline Pos2 operator+(Pos2 arg) { return Pos2( x+arg.x, y+arg.y ); }
    inline Pos2 operator-(Pos2 arg) { return Pos2( x-arg.x, y-arg.y ); }    
    inline bool nextTo(Pos2 tgt) {
        int dx = tgt.x - x, dy = tgt.y - y;
        if( dx==0 ) {
            if( dy == 1 || dy == -1 ) return true;
        } else if( dx == 1 || dx == -1 ) {
            if( dy == 0 ) return true;
        }
        return false;
    }
    inline Pos2 randomize( int r ) { return Pos2( x + irange(-r,r+1), y + irange(-r,r+1) ); }
};



class PosSet {
public:
    Pos2 *ary;
    int nmax, nused;
    PosSet(int maxnum) : nused(0) {
        nmax = maxnum;
        ary = (Pos2*)MALLOC( sizeof(Pos2) * nmax );
        for(int i=0;i<maxnum;i++) ary[i].clear();
    }
    ~PosSet() {
        FREE(ary);
    }
    // true if added
    bool add(Pos2 p) {
        for(int i=0;i<nmax;i++) {
            if( ary[i] == p ) return false;
        }
        if( nused >= nmax ) return false; // full
        ary[nused] = p;
        nused++;
        return true;
    }
    bool find(Pos2 p) {
        for(int i=0;i<nmax;i++) {
            if( ary[i] == p ) return true;
        }
        return false;
    }
  
  
};

#endif
