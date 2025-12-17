#pragma once
#include "../include/npc.hpp"

struct Squirrel : public NPC {
    Squirrel(int x, int y);
    Squirrel(std::istream &is);

    void print() override;
    void save(std::ostream &os) const override;

    bool accept(std::shared_ptr<NPC> visitor) override;

    bool fight(std::shared_ptr<Squirrel> other) override;
    bool fight(std::shared_ptr<Orc> other) override;
    bool fight(std::shared_ptr<Bear> other) override;

    friend std::ostream &operator<<(std::ostream &os, Squirrel &squirrel);
};
