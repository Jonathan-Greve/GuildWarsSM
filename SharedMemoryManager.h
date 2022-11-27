#pragma once
#include "MapInstance.h"

using namespace boost::interprocess;

// Manages the shared memory resources.
class SharedMemoryManager
{
    managed_shared_memory m_shared_memory;
    std::pair<InstanceId, std::shared_ptr<MapInstance>> m_current_instance;

public:
    bool is_connected(InstanceId id)
    {
        if (m_current_instance.first == id)
        {
            return true;
        }

        return false;
    }

    std::shared_ptr<MapInstance> connect(const MapInstance instance, std::string email)
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
            m_shared_memory = std::move(instance_segment);
        }

        std::string instance_name = std::format("instance_{}", instance.id);
        auto curr_instance = m_shared_memory.find_or_construct<MapInstance>(instance_name.c_str())(instance);

        if (curr_instance != nullptr)
        {
            curr_instance->connect(email);

            m_current_instance.first = instance.id;
            m_current_instance.second.reset(curr_instance);

            return m_current_instance.second;
        }
        else
        {
            throw "Couldn't construct or find instance in shared memory.";
        }
    }

    // Try to remove and delete the shared memory. This checks that this process is the last process
    // with a handle to the shared memory. Returns immediately if id is not in _instances.
    void disconnect(InstanceId id, std::string email)
    {
        if (m_current_instance.first == id)
        {
            if (m_current_instance.second != nullptr)
            {
                // Remove ourselves from the instance. Delete instance if we were the last client connected.
                int num_connected_clients = m_current_instance.second->disconnect(email);
                if (num_connected_clients == 0)
                {
                    // Delete instance
                    std::string instance_name = std::format("instance_{}", id);
                    m_shared_memory.destroy<MapInstance>(instance_name.c_str());
                }
            }
        }
    }
};
