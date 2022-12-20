#include "pch.h"
#include "SharedMemoryDataManager.h"

void SharedMemoryDataManager::init(ClientSharedMemory& shared_memory)
{
    m_client_data = shared_memory.get().find_or_construct<ClientData>(unique_instance)();
    m_client_data->player.agent_id = 5;
}

int SharedMemoryDataManager::update_client_data()
{
    int bytes_written = 0;
    const auto character = GW::Agents::GetCharacter();
    if (character)
    {
        m_client_data->player.agent_id = character->agent_id;
        m_client_data->player.ground = character->ground;
        m_client_data->player.h0060 = character->h0060;

        // The game uses a different coordinate system than what I like.
        // I change it do my prefered left-handed coordinate system.
        m_client_data->player.x = character->x;
        m_client_data->player.y = -character->z;
        m_client_data->player.z = character->y;

        m_client_data->player.health = character->hp;
        m_client_data->player.energy = character->energy;

        m_client_data->instance_info.fps_timer = character->timer;
    }

    const auto char_context = GW::GetCharContext();
    if (char_context)
    {
        m_client_data->instance_info.instance_id = char_context->token1;
    }

    const auto party_context = GW::GetPartyContext();
    if (party_context)
    {
        m_client_data->party.party_id = party_context->player_party->party_id;
    }

    bytes_written += sizeof(ClientData);

    return bytes_written;
}
