//export module camera;
//
//import Coord;
//
//static __int64 cameraX = 0;
//static __int64 cameraY = 0;
//static float zoomScale = 1.0;
//static Coord* target = nullptr;
//static bool fakeChase = false;
//
//export void cameraSetTarget(Coord* input) { target = input; }
//
//export void cameraFakeChaseOn() { fakeChase = true; }
//export void cameraFakeChaseOff() { fakeChase = false; }
//
//export void cameraChaseTarget()
//{
//    cameraX = target->getX();
//    cameraY = target->getY();
//
//    if (fakeChase)
//    {
//        cameraX += target->getIntegerFakeX();
//        cameraY += target->getIntegerFakeY();
//    }
//}
//
//export void setCamera(__int64 inputX, __int64 inputY)
//{
//    cameraX = inputX;
//    cameraY = inputY;
//}
//export __int64 getCameraX() { return cameraX; }
//export __int64 getCameraY() { return cameraY; }
//export float getZoomScale() { return zoomScale; }
//
