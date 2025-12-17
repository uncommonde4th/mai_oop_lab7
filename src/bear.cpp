#include "../include/bear.hpp"
#include "../include/squirrel.hpp"
#include "../include/orc.hpp"

Bear::Bear(int x, int y) : NPC(NpcType::BearType, x, y) {
    step_range = 5;
    kill_range = 10;
}

Bear::Bear(std::istream &is) : NPC(NpcType::BearType, is) {
    step_range = 5;
    kill_range = 10;
}

void Bear::print() {
    std::cout << *this;
}

void Bear::save(std::ostream &os) const {
    os << "\n" << static_cast<int>(type) << "\n";
    NPC::save(os);
}

bool Bear::accept(std::shared_ptr<NPC> visitor) {
    return visitor->fight(std::dynamic_pointer_cast<Bear>(shared_from_this()));
}

// Медведи убивают белок

bool Bear::fight(std::shared_ptr<Bear> other) {
    fight_notify(other, false);
    return false;
}

bool Bear::fight(std::shared_ptr<Squirrel> other) {
    fight_notify(other, true);
    return true;
}

bool Bear::fight(std::shared_ptr<Orc> other) {
    fight_notify(other, false);
    return false;
}

std::ostream &operator<<(std::ostream &os, Bear &bear) {
    os << "Медведь: " << *static_cast<NPC *>(&bear) << std::endl;
    return os;
}
