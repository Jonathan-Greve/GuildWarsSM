#pragma once
#include "ClientData.h"

using namespace boost::interprocess;

// Keep all the client data up to date in shared memory. I.e. Position, HP, Energy, etc.
class SharedMemoryDataManager
{
public:
    void init(SharedMemory& shared_memory);

    // Update all data in shared memory and returns the number of bytes written.
    int update_client_data();

private:
    std::unique_ptr<ClientData> m_client_data;
};
