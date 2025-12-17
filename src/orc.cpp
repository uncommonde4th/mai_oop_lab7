#include "../include/squirrel.hpp"
#include "../include/orc.hpp"
#include "../include/bear.hpp"

Orc::Orc(int x, int y) : NPC(NpcType::OrcType, x, y) {
    step_range = 20;
    kill_range = 10;
}

Orc::Orc(std::istream &is) : NPC(NpcType::OrcType, is) {
    step_range = 20;
    kill_range = 10;
}


void Orc::print() {
    std::cout << *this;
}

void Orc::save(std::ostream &os) const {
    os << "\n" << static_cast<int>(type) << "\n";
    NPC::save(os);
}

bool Orc::accept(std::shared_ptr<NPC> visitor) {
    return visitor->fight(std::dynamic_pointer_cast<Orc>(shared_from_this()));
}

// Орки убивают медведей и орков

bool Orc::fight(std::shared_ptr<Orc> other) {
    fight_notify(other, true);
    return true;
}

bool Orc::fight(std::shared_ptr<Squirrel> other) {
    fight_notify(other, false);
    return false;
}

bool Orc::fight(std::shared_ptr<Bear> other) {
    fight_notify(other, true);
    return true;
}

std::ostream &operator<<(std::ostream &os, Orc &orc) {
    os << "Орк: " << *static_cast<NPC*>(&orc) << "\n";
    return os;
}
