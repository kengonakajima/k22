#ifndef _MAPVIEW_H_
#define _MAPVIEW_H_

class Field;

class Chunk;
class MapView {
public:
    Chunk **chunks;
    int chw,chh;
    MapView(int w, int h);
    ~MapView();
    int chunkIndex(int chx, int chy) { return chx + chy*chw;}
    void update(Field*f);
};

void pollSpaceBG(double dt);
void setupSpaceBG();
int groundTypeToBaseIndex( GROUNDTYPE gt ) ;

#endif

