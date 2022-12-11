#include "pch.h"
#include "SharedMemoryDataManager.h"

void SharedMemoryDataManager::init(SharedMemory& shared_memory)
{
    m_client_data =
      std::unique_ptr<ClientData>(shared_memory.get().find_or_construct<ClientData>(unique_instance)());
}

int SharedMemoryDataManager::update_client_data()
{
    int bytes_written = 0;
    const auto character = GW::Agents::GetCharacter();
    if (character)
    {
        m_client_data->player.agent_id = character->agent_id;
        m_client_data->player.fps_timer = character->timer;
        m_client_data->player.layer = character->level;

        // The game uses a different coordinate system than what I like.
        // I change it do my prefered left-handed coordinate system.
        m_client_data->player.x = character->x;
        m_client_data->player.y = -character->z;
        m_client_data->player.z = character->y;

        bytes_written += sizeof(ClientData);
    }
    return bytes_written;
}
