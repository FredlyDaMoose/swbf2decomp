#pragma once

#include <stddef.h>

#include "Damageable.h"
#include "GameSound.h"
#include "Weapon.h"

class Combo;
class ExplosionClass;
class PblFileChunk;
class PblMatrix;
class RedModel;

// VTABLE: SWBF2 0x007b1534
class WeaponMeleeClass : public WeaponClass
{
public:
    WeaponMeleeClass(unsigned int uHash);
    virtual ~WeaponMeleeClass();

    static void Init();

    WeaponMeleeClass(WeaponMeleeClass* aParent, unsigned int aId);

    static void PostLoad();

    static void ReadCombo(PblFileChunk& Chunk);

    struct DamageEdge
    {
        unsigned int uiAttachToWeapon;
        unsigned int uiEdgeHP;
        PblVector3 vOffsetPos;
        PblVector3 vOffsetDir;
        float fLength;
        float fWidth;
        unsigned int uiTextureHash;
        unsigned int uiGlowTextureHash;
        RedColor colorTrail;
    };

    void GetDamageEdgeRay(unsigned int uiEdge, unsigned int& uiAttach,
                          PblVector3& vBegin, PblVector3& vEnd) const;

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    bool IsLightSaber() const;
    void PlayTurnOnSound() const;
    void PlayTurnOffSound() const;

    void Render(float fTurnOn, const PblMatrix& mMatrix, const class RedPose* pPose, const RedColor& color,
                unsigned int uiFlags, bool bArg) const;

    static PblHash rttiHashWeaponMeleeClass;
    static char rttiNameWeaponMeleeClass[];

    Combo* m_pComboODF;
    Combo* m_pCombo;
    ExplosionClass* mImpactExplosion;
    unsigned int m_uiHitEffects[5];
    short m_uiNumHitEffects;
    GameSound m_HitSound;
    unsigned int m_uiDeflectEffect;
    GameSound m_DeflectSound;
    GameSound m_OnSound;
    GameSound m_TurnOnSound;
    GameSound m_TurnOffSound;
    float m_fTurnOnDuration;
    DamageDesc mDamage;
    unsigned int m_uiNumDamageEdges;
    DamageEdge* m_aDamageEdge;
    RedModel* m_apWeaponModel[3];
    unsigned int m_auiHardpoint[4];
    unsigned int m_uiNumWeapons;

    int m_eDeflectMatrixMap;
    PblMatrix* m_amDeflectMatrix;

protected:

    int _AttachDamageEdge(const RedModel* pAttachToModel, unsigned int uiAttachModelToHP,
                          unsigned int uiAttachToModelHP);

    DamageEdge* _CreateDamageEdges(unsigned int uiNumEdges);

    Combo* _CreateODFCombo(unsigned int uiComboId);
    void _InitDeflectMatrices(int eMap);
};
 
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
