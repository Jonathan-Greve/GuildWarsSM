#pragma once
class ConnectedClients
{
public:
    void connect(std::string shared_memory_name)
    {
        std::scoped_lock lock(m_mutex);

        m_connected_shared_memory_names.insert(shared_memory_name);
    }
    void disconnect(std::string shared_memory_name)
    {
        std::scoped_lock lock(m_mutex);

        m_connected_shared_memory_names.erase(shared_memory_name);
    }

private:
    std::mutex m_mutex;
    std::unordered_set<std::string> m_connected_shared_memory_names;
};
