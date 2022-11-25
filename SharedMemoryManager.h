#pragma once
#include "MapInstance.h"

using namespace boost::interprocess;

// Managed the shared memory resources.
class SharedMemoryManager
{
    std::unordered_map<InstanceId, managed_shared_memory> _instances;

public:
    // Check if the instance has been opened/created and stored in _instances.
    bool IsInstanceAdded(InstanceId id)
    {
        if (! _instances.contains(id))
        {
            return true;
        }

        return false;
    }

    // Open or create the shared memory resources and add it to _instances.
    // Return immediately if already added.
    const managed_shared_memory& GetOrAddInstance(const MapInstance instance)
    {
        if (! _instances.contains(instance.id))
        {
            // Init shared memory resource where all instances are stored.
            auto instance_segment = managed_shared_memory(open_or_create, "sm_instances", 65536);

            // Construct MapInstance in shared memory. It it already exists it does nothing.
            const auto _ = instance_segment.find_or_construct<MapInstance>(
              std::format("instance_{}", instance.id).c_str())(instance);

            //MapInstance copy(std::move(instance));

            // Store in map for future use.
            _instances[instance.id] = std::move(instance_segment);
        }
        return _instances[instance.id];
    }

    // Try to remove and delete the shared memory. This checks that this process is the last process
    // with a handle to the shared memory. Returns immediately if id is not in _instances.
    void TryRemoveInstance(InstanceId id)
    {
        if (_instances.contains(id))
        {
            throw "Not yet implmented";
        }
    }
};
