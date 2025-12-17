#pragma once
#include "../include/npc.hpp"

struct Bear : public NPC
{
    Bear(int x, int y);
    Bear(std::istream &is);

    void print() override;
    void save(std::ostream &os) const override;

    bool accept(std::shared_ptr <NPC> visitor) override;

    bool fight(std::shared_ptr<Bear> other) override;
    bool fight(std::shared_ptr<Squirrel> other) override;
    bool fight(std::shared_ptr<Orc> other) override;

    friend std::ostream &operator<<(std::ostream &os, Bear &bear);
};
