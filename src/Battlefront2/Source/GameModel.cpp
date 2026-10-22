#include <string.h>

#include "GameModel.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"

extern bool sLoadLowDetailModels;

// GLOBAL: SWBF2 0x01ec1234
// SYNTHETIC: SWBF2 0x00403d80
// `dynamic initializer for 'GameModel::_HashTable''

PblHashTable<GameModel, 1024> GameModel::_HashTable;

// GLOBAL: SWBF2 0x01ec1230
int GameModel_HashTableNumEntries;

void GameModel::Reset()
{
    _HashTable.Clear();
    GameModel_HashTableNumEntries = 0;
}

// FUNCTION: SWBF2 0x00533da0
GameModel::GameModel()
{
    for (int i = 0; i < 4; ++i)
    {
        _iLodCost[i] = 0;
    }
    _mModel = 0;
    for (int i = 0; i < 4; ++i)
    {
        _mLod[i] = 0;
    }
}

// FUNCTION: SWBF2 0x00533e00
void GameModel::Render(unsigned char lod, float fAlpha, const PblMatrix* pMatrix, const RedPose* pPose,
                       const RedColor& color, unsigned int uiFlags, RedShadingPose* pShadingPose) const
{
    const RedModel* pModel = _mLod[lod];
    if (!pModel)
    {
        pModel = _mLod[0];
    }
    if (!pModel)
    {
        pModel = _mLod[1];
    }
    if (!pModel)
    {
        pModel = _mLod[2];
    }
    if (!pModel)
    {
        return;
    }

    RedColor renderColor = color;
    if (fAlpha < 1.0f)
    {
        renderColor.a = (unsigned char)(int)((float)color.a * fAlpha);
    }
    pModel->Render(pMatrix, pPose, renderColor, uiFlags, pShadingPose);
}

// FUNCTION: SWBF2 0x00533ea0
void GameModel::PrintDebugInfo(const RedModel* pModel, const PblVector3& vPos)
{
    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    float fDist = (vPos.x - mCamera.m[3][0]) * -mCamera.m[2][0]
                + (vPos.y - mCamera.m[3][1]) * -mCamera.m[2][1]
                + (vPos.z - mCamera.m[3][2]) * -mCamera.m[2][2];
    if (fDist > 0.0f && fDist < 100.0f)
    {
        int iNumVtx = pModel->_uiNumStripVtx;
        if (iNumVtx > 100)
        {
            RedColor color = { 0xff, 0xff, 0xff, 0xff };
            if (iNumVtx > 1000)
            {
                color = RedColor::RED;
            }
            else if (iNumVtx > 500)
            {
                color = RedColor::YELLOW;
            }
            RedDebug::SetPrintfData(color, RedGlyphCacheElement::TEXT_STYLE_SHADOW);
            RedDebug::Printf3D(vPos, "V:%d", iNumVtx);
        }

        int iNumSegments = pModel->_iNumSegments;
        RedColor color;
        if (iNumSegments > 6)
        {
            color = RedColor::RED;
        }
        else if (iNumSegments > 4)
        {
            color = RedColor::YELLOW;
        }
        else
        {
            color = RedColor::WHITE;
        }
        RedDebug::SetPrintfData(color, RedGlyphCacheElement::TEXT_STYLE_SHADOW);
        PblVector3 vText(vPos.x + 0.0f, vPos.y + (fDist * 0.02f + 0.5f), vPos.z + 0.0f);
        RedDebug::Printf3D(vText, "S:%d", iNumSegments);
    }
}

// FUNCTION: SWBF2 0x00534000
bool GameModel::_Read(PblFileChunk& chunk)
{
    char acName[256];
    char acModel[256];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x32444f4c:
            child.ReadString(acModel, 256);
            _mLod[2] = RedModel::_HashTable.Find(PblHash(acModel));
            _iLodCost[2] = child._pFile->Read32();
            if (!_mLod[0] && !_mLod[1])
            {
                _mModel = _mLod[2];
            }
            break;

        case 0x31444f4c:
            child.ReadString(acModel, 256);
            _mLod[1] = RedModel::_HashTable.Find(PblHash(acModel));
            _iLodCost[1] = child._pFile->Read32();
            if (!_mLod[0])
            {
                _mModel = _mLod[1];
            }
            break;

        case 0x30444f4c:
            child.ReadString(acModel, 256);
            _mLod[0] = RedModel::_HashTable.Find(PblHash(acModel));
            _mModel = _mLod[0];
            _iLodCost[0] = child._pFile->Read32();
            break;

        case 0x44574f4c:
            child.ReadString(acModel, 256);
            if (!sLoadLowDetailModels)
            {
                _mLod[3] = RedModel::_HashTable.Find(PblHash(acModel));
                _iLodCost[3] = child._pFile->Read32();
            }
            break;

        case 0x454d414e:
            child.ReadString(acName, 256);
            _uiNameHash = PblHash(acName);
            if (_HashTable.Find(_uiNameHash))
            {
                return false;
            }
            if (_HashTable.Store(_uiNameHash, this))
            {
                ++GameModel_HashTableNumEntries;
            }
            break;

        case 0x4f464e49:
        {
            _iLodType = child._pFile->Read32();
            _fLodBias = child._pFile->ReadFloat();
            mClothCount = child._pFile->Read32();
            unsigned int i;
            for (i = 0; i < mClothCount; ++i)
            {
                child.ReadString(acName, 256);
                mClothList[i] = PblHash(acName);
            }
            for (i = mClothCount; i < 8; ++i)
            {
                mClothList[i] = 0;
            }
            break;
        }
        }
    }

    _strnicmp(acName, "coll_", 5);
    _pSkeleton = (RedSkeleton*)RedSkeleton::Find(_uiNameHash);
    if (_strnicmp(acName, "coll_", 5) == 0)
    {
        _mLodData = NoLodData();
    }
    else
    {
        _mLodData = ModelLodData(_iLodType, _mLod[0], _mLod[1], _mLod[2], _mLod[3]);
    }

    if (!_mLod[0])
    {
        _mLod[0] = _mLod[1] ? _mLod[1] : _mLod[2];
    }
    if (!_mLod[1])
    {
        _mLod[1] = _mLod[2] ? _mLod[2] : _mLod[0];
    }
    if (!_mLod[2])
    {
        _mLod[2] = _mLod[1] ? _mLod[1] : _mLod[0];
    }
    if (!_mLod[0])
    {
        _mLod[0] = _mLod[3];
    }
    if (!_mLod[1])
    {
        _mLod[1] = _mLod[3];
    }
    if (!_mLod[2])
    {
        _mLod[2] = _mLod[3];
    }
    return true;
}

// FUNCTION: SWBF2 0x005343b0
void GameModel::Read(PblFileChunk& chunk)
{
    GameModel* pModel = new GameModel;
    if (!pModel->_Read(chunk))
    {
        delete pModel;
    }
}
