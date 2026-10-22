#pragma once

#include <stddef.h>

#include "GameCamera.h"
#include "PebbleFL/Common/PblMatrix.h"

// VTABLE: SWBF2 0x007aa43c
class MapCamera : public GameCamera
{
public:
    enum ZoomLevel
    {
        ZOOM_OUT = 0,
        ZOOM_NORMAL = 1,
        ZOOM_IN = 2,
        NUM_ZOOM_LEVELS = 3
    };

    MapCamera(CameraManager* pManager, int iView);
    virtual ~MapCamera() {}
    virtual void Update(float fDeltaTime);
    virtual void SetupCamera(RedCamera* pCamera, float fDeltaTime);
    virtual void SetMatrix(const PblMatrix& rMatrix);
    virtual PblVector3 GetVelocity();
    void SetZoomLevel(ZoomLevel eLevel);

    int GetZoomDir() { return mZoomDir; }
    ZoomLevel GetZoomLevel() { return mZoomLevel; }
    float GetZoomRatio() { return mZoomRatio; }
    void Snap()
    {
        mMoveRatio = 1.0f;
        mZoomRatio = 1.0f;
    }
    static void SetOffset(ZoomLevel eLevel, const PblVector3& vOffset) { sOffset[eLevel] = vOffset; }
    static void SetPitch(ZoomLevel eLevel, float fPitch) { sPitch[eLevel] = fPitch; }

    static PblVector3 sOffset[NUM_ZOOM_LEVELS];
    static float sPitch[NUM_ZOOM_LEVELS];

    PblMatrix mMatrix;
    PblMatrix mTarget;
    PblVector3 mVelocity;
    float mMoveRatio;
    PblVector3 mMoveCurPos;
    PblVector3 mMoveOldPos;
    int mZoomDir;
    ZoomLevel mZoomLevel;
    float mZoomRatio;
    PblVector3 mZoomOldDir;
    PblVector3 mZoomOldPos;
    PblVector3 mZoomNewDir;
    PblVector3 mZoomNewPos;
    float mZoomDist;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
