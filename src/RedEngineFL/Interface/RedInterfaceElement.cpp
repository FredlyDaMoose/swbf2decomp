#include "RedInterfaceElement.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedGroupElement.h"
#include "PebbleFL/Common/PblMath.h"

// FUNCTION: SWBF2 0x006c0250
void RedInterfaceElement::Rotate(float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    float oldX[3], oldY[3];
    for (int i = 0; i != 3; ++i)
    {
        oldX[i] = m_mLocal.m[0][i];
        oldY[i] = m_mLocal.m[1][i];
    }
    for (int i = 0; i != 3; ++i)
    {
        m_mLocal.m[0][i] = oldX[i] * fCos + oldY[i] * fSin;
        m_mLocal.m[1][i] = oldX[i] * -fSin + oldY[i] * fCos;
    }
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal.m[1][3] = 0.0f;
}

// FUNCTION: SWBF2 0x006c0370
void RedInterfaceElement::Rotate(float fX, float fY, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    float oldX[3], oldY[3];
    for (int i = 0; i != 3; ++i)
    {
        oldX[i] = m_mLocal.m[0][i];
        oldY[i] = m_mLocal.m[1][i];
    }
    for (int i = 0; i != 3; ++i)
    {
        m_mLocal.m[0][i] = oldX[i] * fCos + oldY[i] * fSin;
        m_mLocal.m[1][i] = oldX[i] * -fSin + oldY[i] * fCos;
        m_mLocal.m[3][i] = m_mLocal.m[3][i] + oldX[i] * fX + oldY[i] * fY;
    }
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal.m[1][3] = 0.0f;
    m_mLocal.m[3][3] = 1.0f;
}

// GLOBAL: SWBF2 0x008fe488
unsigned int RedInterfaceElement::s_uiUnnamedCRC;

// GLOBAL: SWBF2 0x008fe498
int RedInterfaceElement::sAllObjectsListNumEntries;

// GLOBAL: SWBF2 0x008fe49c
// SYNTHETIC: SWBF2 0x00407a10
// `dynamic initializer for 'RedInterfaceElement::sAllObjectsList''

PblHashTable<RedInterfaceElement, 32768> RedInterfaceElement::sAllObjectsList;

// FUNCTION: SWBF2 0x006bfeb0
RedInterfaceElement::RedInterfaceElement(const char* pcName)
{
    m_zorder = 128;
    m_bEnabled = true;

    m_groupNode._pList = 0;
    m_groupNode._pNext = 0;
    m_groupNode._pPrev = 0;
    m_groupNode._pObject = 0;

    m_color.b = 0xFF;
    m_color.g = 0xFF;
    m_color.r = 0xFF;
    m_color.a = 0xFF;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            m_mLocal.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    m_bInAllObjectList = false;
    m_uiFlags = 0;

    m_pHotSpot = 0;

    SetName(pcName);
}

// FUNCTION: SWBF2 0x006bff90
RedInterfaceElement::RedInterfaceElement()
{
    m_zorder = 128;
    m_bEnabled = true;

    m_groupNode._pList = 0;
    m_groupNode._pNext = 0;
    m_groupNode._pPrev = 0;
    m_groupNode._pObject = 0;

    m_color.b = 0xFF;
    m_color.g = 0xFF;
    m_color.r = 0xFF;
    m_color.a = 0xFF;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            m_mLocal.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    m_crcName = s_uiUnnamedCRC++;

    m_bInAllObjectList = false;
    m_uiFlags = 0;

    m_pHotSpot = 0;
}

// FUNCTION: SWBF2 0x006c0070
RedInterfaceElement::~RedInterfaceElement()
{
    if (m_bInAllObjectList)
    {
        m_bInAllObjectList = false;
        if (sAllObjectsList.Remove(m_crcName))
        {
            --sAllObjectsListNumEntries;
        }
    }

    if (m_groupNode._pList)
    {
        m_groupNode._pList->Remove(&m_groupNode);
    }

    delete m_pHotSpot;

    SetName(0);
}

// FUNCTION: SWBF2 0x006c0170
void RedInterfaceElement::BringToFront()
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;
    m_zorder = 0;
    if (!pList)
    {
        return;
    }
    pList->Remove(&m_groupNode);
    pList->AddLast(&m_groupNode, this);
}

// FUNCTION: SWBF2 0x006c0590
void RedInterfaceElement::SendToBack()
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;
    m_zorder = 0xFF;
    if (!pList)
    {
        return;
    }
    pList->Remove(&m_groupNode);
    pList->AddFirst(&m_groupNode, this);
}

// FUNCTION: SWBF2 0x006c0750
void RedInterfaceElement::SetMatrixPosScale(float fX, float fY, float fScaleX,
                                            float fScaleY)
{
    m_mLocal.m[0][0] = fScaleX;
    m_mLocal.m[0][1] = 0.0f;
    m_mLocal.m[0][2] = 0.0f;
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal.m[1][0] = 0.0f;
    m_mLocal.m[1][1] = fScaleY;
    m_mLocal.m[1][2] = 0.0f;
    m_mLocal.m[1][3] = 0.0f;
    m_mLocal.m[2][0] = 0.0f;
    m_mLocal.m[2][1] = 0.0f;
    m_mLocal.m[2][2] = 1.0f;
    m_mLocal.m[2][3] = 0.0f;
    m_mLocal.m[3][0] = fX;
    m_mLocal.m[3][1] = fY;
    m_mLocal.m[3][2] = 0.0f;
    m_mLocal.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x006c0510
void RedInterfaceElement::Scale(float fScaleX, float fScaleY)
{
    m_mLocal.m[0][0] *= fScaleX;
    m_mLocal.m[0][1] *= fScaleX;
    m_mLocal.m[0][2] *= fScaleX;
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal.m[1][0] *= fScaleY;
    m_mLocal.m[1][1] *= fScaleY;
    m_mLocal.m[1][2] *= fScaleY;
    m_mLocal.m[1][3] = 0.0f;
}

// FUNCTION: SWBF2 0x006c07e0
void RedInterfaceElement::SetName(const char* pcName)
{
    m_crcName = PblHash(pcName);
}

// FUNCTION: SWBF2 0x006c0880
void RedInterfaceElement::SetZOrder(unsigned char uZOrder)
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;

    m_zorder = uZOrder;
    if (!pList)
    {
        return;
    }

    pList->Remove(&m_groupNode);

    PblList<RedInterfaceElement>::Node* pAt = pList->GetFirst();
    while (pAt->_pObject != pList->_head._pObject && uZOrder < pAt->_pObject->m_zorder)
    {
        pAt = pAt->_pNext;
    }

    pList->AddBefore(pAt, &m_groupNode, this);
}

// FUNCTION: SWBF2 0x006c0820
void RedInterfaceElement::SetPosition(float fX, float fY)
{
    const float fZ = m_mLocal.m[3][2];

    m_mLocal.m[3][0] = fX;
    m_mLocal.m[3][1] = fY;
    m_mLocal.m[3][2] = fZ;
    m_mLocal.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x006c0850
void RedInterfaceElement::SetPosition(float fX, float fY, float fZ)
{
    m_mLocal.m[3][0] = fX;
    m_mLocal.m[3][1] = fY;
    m_mLocal.m[3][2] = fZ;
    m_mLocal.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x006c0800
void RedInterfaceElement::SetPosition(const PblVector3& vPos)
{
    SetPosition(vPos.x, vPos.y, vPos.z);
}

// FUNCTION: SWBF2 0x006c0210
RedGroupElement* RedInterfaceElement::GetGroup() const
{
    PblList<RedInterfaceElement>* pList = m_groupNode._pList;
    if (pList)
    {
        return (RedGroupElement*)((char*)pList - offsetof(RedGroupElement, m_children));
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c0130
void RedInterfaceElement::AddToSearchList(const char* pcName)
{
    SetName(pcName);
    if (sAllObjectsList.Store(m_crcName, this))
    {
        ++sAllObjectsListNumEntries;
    }
    m_bInAllObjectList = true;
}

// FUNCTION: SWBF2 0x006c01d0
RedInterfaceElement* RedInterfaceElement::Find(unsigned int uiNameHash)
{
    return sAllObjectsList.Find(uiNameHash);
}

// FUNCTION: SWBF2 0x006c01f0
RedInterfaceElement* RedInterfaceElement::Find(const char* pcName)
{
    return Find((unsigned int)PblHash(pcName));
}

// FUNCTION: SWBF2 0x006c0220
void RedInterfaceElement::Reset()
{
    sAllObjectsList.Clear();
    sAllObjectsListNumEntries = 0;
    s_uiUnnamedCRC = 0;
}

// FUNCTION: SWBF2 0x006c05f0
void RedInterfaceElement::SetMatrix(const PblMatrix& matrix)
{
    for (int i = 0; i < 4; i++)
    {
        m_mLocal.m[i][0] = matrix.m[i][0];
        m_mLocal.m[i][1] = matrix.m[i][1];
        m_mLocal.m[i][2] = matrix.m[i][2];
        m_mLocal.m[i][3] = 0.0f;
    }
    m_mLocal.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x006c0680
void RedInterfaceElement::SetMatrixPosRotScale(float fX, float fY, float fRotation, float fScaleX,
                                               float fScaleY)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fRotation);
    m_mLocal.m[0][0] = fCos * fScaleX;
    m_mLocal.m[0][1] = fSin * fScaleX;
    m_mLocal.m[0][2] = 0.0f;
    m_mLocal.m[0][3] = 0.0f;
    m_mLocal.m[1][0] = -(fSin * fScaleY);
    m_mLocal.m[1][1] = fCos * fScaleY;
    m_mLocal.m[1][2] = 0.0f;
    m_mLocal.m[1][3] = 0.0f;
    m_mLocal.m[2][0] = 0.0f;
    m_mLocal.m[2][1] = 0.0f;
    m_mLocal.m[2][2] = 1.0f;
    m_mLocal.m[2][3] = 0.0f;
    m_mLocal.m[3][0] = fX;
    m_mLocal.m[3][1] = fY;
    m_mLocal.m[3][2] = 0.0f;
    m_mLocal.m[3][3] = 1.0f;
}
