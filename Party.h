#pragma once
class Party
{
public:
    PartyId party_id = 0;

    Party() = default;
    Party(PartyId party_id)
        : party_id{party_id}
    {
    }
};
