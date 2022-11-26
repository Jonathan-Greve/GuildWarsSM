#pragma once
#include "MapInstance.h"

using namespace boost::interprocess;

// Manages the shared memory resources.
class SharedMemoryManager
{
    std::pair<InstanceId, managed_shared_memory> m_current_instance;

public:
    bool is_connected(InstanceId id)
    {
        if (m_current_instance.first == id)
        {
            return true;
        }

        return false;
    }

    const managed_shared_memory& connect(const MapInstance instance)
    {
        if (m_current_instance.first != instance.id)
        {
            // Create of open shared memory resource where all instances are stored.
            auto instance_segment = managed_shared_memory(open_or_create, "sm_instances", 65536);

            // Construct MapInstance in shared memory. It it already exists it does nothing.
            const auto _ = instance_segment.find_or_construct<MapInstance>(
              std::format("instance_{}", instance.id).c_str())(instance);

            //MapInstance copy(std::move(instance));

            // Store in map for future use.
            m_current_instance = {instance.id, std::move(instance_segment)};
        }
        return m_current_instance.second;
    }

    // Try to remove and delete the shared memory. This checks that this process is the last process
    // with a handle to the shared memory. Returns immediately if id is not in _instances.
    void disconnect(InstanceId id)
    {
        if (m_current_instance.first == id)
        {
            std::string instance_name = std::format("instance_{}", id);
            auto instance = m_current_instance.second.find<MapInstance>(instance_name.c_str());
            if (instance.first != nullptr)
            {
                // Remove ourselves from the instance. Delete instance if we were the last client connected.
                int num_connected_clients = instance.connected_clients().size();
                if (num_connected_clients == 1)
                {
                    // Delete instance
                }
                else
                {
                    // Disconnect
                }
            }
        }
    }
};
