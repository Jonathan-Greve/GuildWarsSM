#pragma once

using namespace boost::interprocess;

class SharedMemory
{
public:
    SharedMemory() = default;
    ~SharedMemory();

    void init(std::string shared_memory_name);

private:
    std::string m_name;
    managed_shared_memory m_managed_shared_memory;
};
