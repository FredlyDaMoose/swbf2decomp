#include "BoundaryManager.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/RedPath.h"

#include "SoundFL/source/win/sndengine.h"
#include "PebbleFL/Common/PblTime.h"
#include "FLInputManager.h"
#include "NetGame.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatUtil.h"
#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "BootDisplay.h"
#include "CaptureDisplay.h"
#include "Cloth.h"
#include "CollisionMesh.h"
#include "EntityHologram.h"
#include "GameLoop.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "LockOnManager.h"
#include "LuaScript.h"
#include "MessageDisplay.h"
#include "MuzzleFlash.h"
#include "OrdnanceGrapplingHook.h"
#include "OrdnanceTowCable.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterObject.h"
#include "PatrolPath.h"
#include "PauseMenu.h"
#include "ScreenTransition.h"
#include "SkyDome.h"
#include "SpectatorDisplay.h"
#include "StatIconDisplay.h"
#include "Tracker.h"
#include "VehicleSpawn.h"
#include "Battlefront2/Platform/Pc/pcGrassPatch.h"
#include "Battlefront2/Platform/Pc/pcLeafPatch.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Interface/RedGlyphCacheElement.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Util/ext/IAllocator.h"
#include "RedEngineFL/Util/ext/hashtable.h"
#include "PebbleFL/Common/PblRegion.h"
#include "CollisionManager.h"
#include "CommandLine.h"
#include "GameMovie.h"
#include "ProceduralAnimation.h"
#include "ShellLoop.h"
#include "RedEngineFL/Interface/RedShadowBlob.h"

// FUNCTION: SWBF2 0x0044e150
void BoundaryManager::PlayOutOfBoundsVoiceOver(int iTeam)
{
    mOutOfBounds[iTeam - 1].Play(0, 0, false, true);
}

// FUNCTION: SWBF2 0x0044de70
void BoundaryManager::Create()
{
    sInstance = new BoundaryManager;
}

// FUNCTION: SWBF2 0x0044df10
BoundaryManager::BoundaryManager()
    : mBoundaries(0)
{
}

// FUNCTION: SWBF2 0x0044df40
BoundaryManager::~BoundaryManager()
{
    while (mBoundaries)
    {
        Boundary* pNext = mBoundaries->mNext;
        delete mBoundaries;
        mBoundaries = pNext;
    }
}

// FUNCTION: SWBF2 0x0044dfc0
void BoundaryManager::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk data = chunk.ReadNextChild();
        const unsigned int uHash = data._pFile->Read32();
        data._pFile->Read8();

        if (uHash == 0x54eacca7)
        {
            PblFileChunk scope = chunk.ReadNextChild();
            while (scope.HasMoreChildren())
            {

                data = scope.ReadNextChild();
                const unsigned int uItemHash = data._pFile->Read32();
                data._pFile->Read8();

                if (uItemHash == 0x84874d36)
                {
                    AddPath(data._pFile->Read32());
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x0044e0a0
void BoundaryManager::AddPath(unsigned int uPathID)
{
    const RedPath* pPath = RedPath::Find(uPathID);
    if (pPath)
    {
        Boundary* pBoundary = new Boundary(pPath);
        pBoundary->mNext = mBoundaries;
        mBoundaries = pBoundary;
    }
}

// FUNCTION: SWBF2 0x0044e110
bool BoundaryManager::IsOutOfBounds(const PblVector3& vPos) const
{
    if (!mBoundaries)
    {
        return false;
    }
    for (Boundary* pBoundary = mBoundaries; pBoundary; pBoundary = pBoundary->mNext)
    {
        if (pBoundary->IsInBounds(vPos))
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0044e190
Boundary::Boundary(const RedPath* pPath)
    : mNext(0)
{
    mCount = pPath->GetNumPoints();
    mEdges = new Edge[mCount];
    AddEdge(0, pPath->GetPointPosition(mCount - 1), pPath->GetPointPosition(0));

    PblVector3 vPrev = pPath->GetPointPosition(0);
    for (int i = 1; i < mCount; ++i)
    {
        const PblVector3& vPoint = pPath->GetPointPosition(i);
        AddEdge(i, vPrev, vPoint);
        vPrev = vPoint;
    }
}

// FUNCTION: SWBF2 0x0044e250
Boundary::~Boundary()
{
    delete[] mEdges;
}

// FUNCTION: SWBF2 0x0044e260
bool Boundary::IsInBounds(const PblVector3& vPos) const
{
    for (int i = 0; i < mCount; ++i)
    {
        const Edge& edge = mEdges[i];
        if ((vPos.z - edge.z) * edge.nz + (vPos.x - edge.x) * edge.nx > 0.0f)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0044e2c0
void Boundary::AddEdge(int i, const PblVector3& vStart, const PblVector3& vEnd)
{
    mEdges[i].x = vStart.x;
    mEdges[i].z = vStart.z;
    mEdges[i].nx = -(vEnd.z - vStart.z);
    mEdges[i].nz = vEnd.x - vStart.x;
}

// FUNCTION: SWBF2 0x0044dee0
void BoundaryManager::Destroy()
{
    delete sInstance;
    sInstance = 0;
}

// FUNCTION: SWBF2 0x0044e170
void BoundaryManager::SetOutOfBoundsVoiceOverID(int iTeam, const char* pName)
{
    mOutOfBounds[iTeam - 1].SetID(pName, false, false);
}

// GLOBAL: SWBF2 0x01e30320
BoundaryManager* BoundaryManager::sInstance;
