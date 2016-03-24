#ifndef _MAPVIEW_H_
#define _MAPVIEW_H_

class Field;

class MapView : public Prop2D {
public:
    Grid *grid;
    MapView(int w, int h);
    ~MapView();
    void update(Field*f);
    virtual bool prop2DPoll(double dt);
};

#endif

