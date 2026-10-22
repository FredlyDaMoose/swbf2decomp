#pragma once

#include <stddef.h>

#include "GameCamera.h"
#include "PebbleFL/Common/PblMatrix.h"

class GUIInputs;
class ProcessedInputs;

// VTABLE: SWBF2 0x0079f304
class FreeCamera : public GameCamera
{
public:
    FreeCamera(CameraManager* pManager, int iView);
    virtual ~FreeCamera() {}
    virtual void Update(float fDeltaTime);
    virtual void SetupCamera(RedCamera* pCamera, float fDeltaTime);
    virtual void SetupIconVisibility(int iView);
    virtual void SetMatrix(const PblMatrix& rMatrix);
    virtual PblVector3 GetVelocity();

    PblMatrix GetMatrix() const;

    static bool sQuakeControl;
    static bool sStop;
    static bool sInvertAxis;
    static const float sMoveScale;
    static const float sTurnScale;
    static const float sHeightScale;

private:
     

    void UpdateControlSpeed(const GUIInputs& rInputs);
    void UpdateOrientation(const ProcessedInputs& rInputs, float fDeltaTime);
    void UpdateTranslation(const ProcessedInputs& rInputs, float fDeltaTime);

public:

    PblMatrix mMatrix;
    float mPitch;
    float mYaw;
    float mMultiplier;
    float mTurnMultiplier;
    bool mButtonFasterDown;
    bool mButtonSlowerDown;
    bool mTurnButtonFasterDown;
    bool mTurnButtonSlowerDown;
    PblVector3 mVelocity;
};
 
 
 
 
 
 
 
 
