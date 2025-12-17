#include "npc.hpp"
#include <queue>

struct Battle{
    std::shared_ptr<NPC> attacker;
    std::shared_ptr<NPC> defender;
};

extern std::queue<Battle> battles;

void battle(const Battle& battle);