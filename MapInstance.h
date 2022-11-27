#pragma once
#include "Party.h"

class MapInstance
{
    // Lock this while checking if a mutex exists in _party_mutexes;
    std::mutex mutex_find_mutex;

    // Email addresses of all the connected processes.
    std::unordered_set<std::string> _connected_client_emails;

    std::unordered_map<PartyId, Party> _parties;
    std::unordered_map<PartyId, std::mutex> _party_mutexes;

public:
    const InstanceId id = 0;
    const GW::Constants::MapID map_id = GW::Constants::MapID::None;

    MapInstance() = delete;

    MapInstance(const MapInstance& map_instance)
        : id{map_instance.id}
        , map_id{map_instance.map_id}
    {
        std::scoped_lock lock(mutex_find_mutex);
        if (_parties.size() > 0 || _party_mutexes.size() > 0)
            throw "MapInstance object only copy-able when no parties are added.";
    };

    MapInstance(InstanceId id, GW::Constants::MapID map_id)
        : id{id}
        , map_id{map_id}
    {
    }

    void connect(std::string email) { }
    uint32_t disconnect(std::string email) { return 0; }

    // Get the party with the given id. If it does not exist, create it.
    // Secure with mutex.
    Party& get_or_add_party(PartyId party_id)
    {
        std::scoped_lock<std::mutex> lock(mutex_find_mutex);
        if (_party_mutexes.contains(party_id))
        {
            std::scoped_lock<std::mutex> lock(_party_mutexes[party_id]);
            if (! _parties.contains(party_id))
            {
                _parties[party_id] = Party(party_id);
            }
        }

        return _parties[party_id];
    }
};
