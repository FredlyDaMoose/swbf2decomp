#pragma once

class TreeGrid;
class PblVector3;
class TreeGridObject;
struct TreeGridCallbackData;

class TreeGridCache
{
public:

    static void CallbackOnRect(TreeGrid* pTreeGrid, const PblVector3& rMin, const PblVector3& rMax,
                               bool (*pCallback)(const TreeGridObject*, TreeGridCallbackData*, float&),
                               TreeGridCallbackData* pData);
    static void CallbackOnLazyRay(TreeGrid* pTreeGrid, const PblVector3& rFrom, const PblVector3& rTo,
                                  bool (*pCallback)(const TreeGridObject*, TreeGridCallbackData*, float&),
                                  TreeGridCallbackData* pData);
    static void CallbackOnSphere(TreeGrid* pTreeGrid, const PblVector3& rCenter, float fRadius,
                                 bool (*pCallback)(const TreeGridObject*, TreeGridCallbackData*, float&),
                                 TreeGridCallbackData* pData);
    static void Prime(TreeGrid* pGrid, const PblVector3& vMin, const PblVector3& vMax);
    static void Flush();
};
