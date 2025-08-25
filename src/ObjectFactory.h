#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include "Mesh.h"
#include <vector>

class ObjectFactory {
public:
    static Mesh createCube();
    static Mesh createPlane();
    static Mesh createSphere(int sectorCount, int stackCount);
};

#endif
