#pragma once

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
    void TryAddInstance(InstanceId id)
    {
        if (! _instances.contains(id))
        {
            auto instance = managed_shared_memory(open_or_create, std::to_string(id).c_str(), 65536);
            _instances[id] = std::move(instance);
        }
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
