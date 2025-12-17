#include "../include/fight_logic.hpp"

std::queue<Battle> battles;

void battle(const Battle& battle)
{
    auto attacker = battle.attacker;
    auto defender = battle.defender;

    if (!attacker->is_alive() || !defender->is_alive()) {
        return;
    }

    if (defender->accept(attacker)) {
        int attack = attacker->roll_dice();
        int defense = defender->roll_dice();

        bool success = (attack > defense);

        if (success) {
            defender->die();
            attacker->fight_notify(defender, true);
        } else {
            attacker->fight_notify(defender, false);
        }
    }
}