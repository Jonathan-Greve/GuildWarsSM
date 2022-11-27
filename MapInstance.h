#pragma once
#include "Party.h"

class MapInstance
{
    // Lock this while checking if a mutex exists in _party_mutexes;
    std::mutex m_mutex;

    // Email addresses of all the connected processes.
    std::unordered_set<std::string> m_connected_client_emails;

    std::unordered_map<PartyId, Party> m_parties;

public:
    const InstanceId id = 0;
    const GW::Constants::MapID map_id = GW::Constants::MapID::None;

    MapInstance() = delete;

    MapInstance(const MapInstance& map_instance)
        : id{map_instance.id}
        , map_id{map_instance.map_id}
    {
        std::scoped_lock lock(m_mutex);
        if (m_parties.size() > 0 || m_connected_client_emails.size() > 0)
            throw "MapInstance object only copy-able when no parties are added.";
    };

    MapInstance(InstanceId id, GW::Constants::MapID map_id)
        : id{id}
        , map_id{map_id}
    {
    }

    void connect(const std::string& email)
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_connected_client_emails.insert(email);
    }
    uint32_t disconnect(const std::string& email)
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_connected_client_emails.erase(email);

        return m_connected_client_emails.size();
    }

    // Get the party with the given id. If it does not exist, create it.
    // Returns a reference to the party object. This object can be shared between
    // multiple clients/processes. The party class protects its own data using mutexes.
    Party& get_or_add_party(PartyId party_id)
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        if (! m_parties.contains(party_id))
        {
            m_parties[party_id] = Party(party_id);
        }

        return m_parties[party_id];
    }
};
