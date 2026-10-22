#pragma once

#include "RedEngineFL/Portals/Portal.h"
#include "RedEngineFL/Util/ext/hashtable.h"

class PblConfig;

namespace pvs
{
class PortalReader
{
public:

    explicit PortalReader(IManager& manager);

    ~PortalReader();

    void Read(IManager& manager, PblConfig& config);

protected:

    void Read(ISector& sector, PblConfig& config, const char* pcName);

    void Read(IPortal& portal, PblConfig& config, IManager& manager, const char* pcName);

    void FindRegisterStatics(ISector& sector);

private:
    IManager& manager_;
    util::HashTable<ISector*, char const*, util::TNewArrayAlloc<ISector*> > namedSectors_;
};
}

 
 
