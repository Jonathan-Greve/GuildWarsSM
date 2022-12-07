#pragma once

using namespace boost::interprocess;

class SharedMemory
{
public:
    SharedMemory() = default;
    ~SharedMemory();

    void init(std::string shared_memory_name);

    managed_shared_memory& get() { return m_managed_shared_memory; }

private:
    std::string m_name;
    managed_shared_memory m_managed_shared_memory;
    managed_shared_memory m_connections_managed_shared_memory;
};
