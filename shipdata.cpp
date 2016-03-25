#include <assert.h>

#include "conf.h"
#include "shipdata.h"

const char *g_ship_data[FIELD_H];

void globalInitShipData() {
    int i=0;
    //                0         1         2         3         4         5         6
    //                0123456789012345678901234567890123456789012345678901234567890123
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";    
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".....................xxxx..............................";
    g_ship_data[i++] = ".....................xx................................";
    g_ship_data[i++] = ".....................xx................................";
    g_ship_data[i++] = "........xxxxxxxx....xxx...xxx..........................";
    g_ship_data[i++] = ".....xxxxxxxxxxxxxxxxxxxxxx............................";
    g_ship_data[i++] = "...xxxxxxxZxxxxxxx.....xxxxx...........................";
    g_ship_data[i++] = ".....xxxxxxxxxxxxxxxxxxxxxx............................";
    g_ship_data[i++] = "........xxxxxxxx....xxx...xxx..........................";
    g_ship_data[i++] = ".....................xx................................";
    g_ship_data[i++] = ".....................xx................................";
    g_ship_data[i++] = ".....................xxxx..............................";    
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";    
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";
    g_ship_data[i++] = ".......................................................";
    //                0123456789012345678901234567890123456789012345678901234567890123    
    assert(i==FIELD_H);    
}

