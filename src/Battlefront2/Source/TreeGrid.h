#pragma once

class PblVector3;
class TreeGridObject;
class TreeGridStack;

struct TreeGridCallbackData
{
    TreeGridCallbackData() { mData[0] = -1; }

    int mData[1];
};

typedef bool (*TreeGridCallback)(const TreeGridObject*, TreeGridCallbackData*, float&);

class TreeGrid
{
public:
    struct TreeGridLayerData
    {
        float mSize;
        float mSizePerMeter;
        int mStartIndex;
        int mGridSize;
    };

    TreeGrid(const PblVector3& rMin, const PblVector3& rMax, float fMinSize);
    ~TreeGrid();

    void Add(TreeGridObject* pObj, const PblVector3& rPos, float fSize);

    void Add(TreeGridObject* pObj, const PblVector3& rPos, float fSize, float f2dSize);
    void Remove(TreeGridObject* pObj);
    void Update(TreeGridObject* pObj, const PblVector3& rPos, float fSize);
    void Update(TreeGridObject* pObj, const PblVector3& rPos, float fSize, float f2dSize);
    void CallbackOnRect(const PblVector3& rMin, const PblVector3& rMax,
                        TreeGridCallback pCallback, TreeGridCallbackData* pData);
    void CallbackOnSphere(const PblVector3& rCenter, float fRadius,
                          TreeGridCallback pCallback, TreeGridCallbackData* pData);

    void CallbackOnRay(const PblVector3& rFrom, const PblVector3& rTo,
                       TreeGridCallback pCallback, TreeGridCallbackData* pData);
    void CallbackOnLazyRay(const PblVector3& rFrom, const PblVector3& rTo,
                           TreeGridCallback pCallback, TreeGridCallbackData* pData);
    void CallbackOnFatRay(const PblVector3& rFrom, const PblVector3& rTo, float fRadius,
                          TreeGridCallback pCallback, TreeGridCallbackData* pData);

    bool ProcessCell(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                     float& fDist);
    bool ProcessCellRect(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                         float& fDist, const PblVector3& rMin, const PblVector3& rMax);
    bool ProcessCellRay(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                        float& fDist, const PblVector3& rFrom, const PblVector3& rDir,
                        float fLength);
    bool ProcessCellSphere(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                           float& fDist, const PblVector3& rCenter, float fRadius);
    bool ProcessNeighborhood(int iX, int iZ, int iGridSize, int iStartIndex, int* piExtents,
                             TreeGridCallback pCallback, TreeGridCallbackData* pData,
                             float& fDist, bool bFull);
    void _CallbackOnRay(const PblVector3& rFrom, const PblVector3& rTo, int* piExtents,
                        TreeGridCallback pCallback, TreeGridCallbackData* pData);

    float mMaxSize;
    float mMinSize;
    float mOffset;
    float mSizePerMeter;
    int mDepth;
    int mMaxGridSize;
    int mNumCells;
    TreeGridStack** mGrid;
    TreeGridLayerData* mLayerData;
};
 
