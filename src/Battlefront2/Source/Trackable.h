#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"

class GameObject;
class PblMatrix;
class Trackable;
class Character;

class CameraTrackSetting
{
public:
    CameraTrackSetting();

    CameraTrackSetting(const CameraTrackSetting& other);

    bool SetProperty(unsigned int uProperty, const char* pszValue);

    PblVector3 CalcEyePoint(const PblMatrix& rMatrix);
    PblVector3 CalcCenterPoint(const PblMatrix& rMatrix);
    PblVector3 CalcCameraSafePoint(const PblMatrix& rMatrix);
    PblVector3 CalcCameraPoint(const PblMatrix& rMatrix, const PblMatrix& rRotation);
    float GetCameraTiltValue();
    void SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix, const PblMatrix& rTarget,
                                            const PblVector3& vAimDir);
    void SetupCameraTrackMatrixHUDTargeting(const CameraTrackSetting& rOther, float fBlend,
                                            PblMatrix& rMatrix, const PblMatrix& rTarget,
                                            const PblVector3& vAimDir);
    void SetupCameraTrackMatrix(Trackable* pTrackable, PblMatrix& rMatrix, float fDeltaTime,
                                const PblMatrix& rTarget, const PblVector3& vAimDir,
                                float fMoveTensionScale, float fAimTensionScale,
                                const GameObject* pIgnore);

    void SetupCameraTrackMatrix(Trackable* pTrackable, const CameraTrackSetting& rOther, float fBlend,
                                PblMatrix& rMatrix, float fDeltaTime, const PblMatrix& rTarget,
                                const PblVector3& vAimDir, float fMoveTensionScale,
                                float fAimTensionScale, const GameObject* pIgnore);
    void SetupAmbientShift(PblMatrix& rMatrix, PblVector3& vShift, PblVector3& vTarget,
                           PblVector3& vSpeed, float fMaxSpeed, float fDeltaTime) const;
    void SetupAmbientShiftComponent(float& rShift, float& rTarget, float& rSpeed, float fMaxSpeed,
                                    float fDeltaTime) const;

    PblVector3 mCameraEyePointOffset;
    PblVector3 mCameraTrackCenter;
    PblVector3 mCameraTrackOffset;
    PblVector3 mCameraSafePoint;
    float mCameraMoveTensionL;
    float mCameraMoveTensionR;
    float mCameraMoveTensionU;
    float mCameraMoveTensionD;
    float mCameraMoveTensionF;
    float mCameraMoveTensionB;
    float mCameraAimTension;
    float mCameraTiltValue;
};
 
 
 
 
 
 
 
 
 
 
 
 
 

class Controllable;
class GameObject;
class Tracker;

class Trackable
{
public:

    Trackable() : whichCornerToCastForCamera(0), mTracker(0)
    {
        for (int i = 0; i < 5; ++i)
        {
            minCameraCastDist[i] = 3.402823466e+38F;
        }
    }
    virtual ~Trackable();
    virtual void SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix);
    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual void SetupCameraIconVisibility(unsigned int& rMask);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix);
    virtual GameObject* GetGameObject();
    virtual const GameObject* GetGameObject() const;
    virtual Controllable* GetControllable();
    virtual const Controllable* GetControllable() const;
    virtual CameraTrackSetting* GetCameraTrackSettings();
    virtual bool RenderTrackable(unsigned int uView) const;
    virtual bool IsFirstPersonAvailablePS2() const;
    virtual void SetFirstPersonView(int iView, bool bFirstPerson);
    virtual bool IsForcedThirdPerson() const;
    virtual bool IsForcedFirstPerson() const;

    void UpdateFirstThirdPerson(int iPlayer);

    void Track(int iPlayer);

    bool CheckFirstPerson(int iPlayer) const;
    void SwitchControlView(int iPlayer);

    void TrackDeath(int iPlayer, Character* pKiller);

    short whichCornerToCastForCamera;
    float minCameraCastDist[5];

    // FUNCTION: SWBF2 0x0044e320
    Tracker* GetTracker() const { return mTracker; }

    void SetTracker(Tracker* pTracker);

    Tracker* mTracker;
};
 
 
