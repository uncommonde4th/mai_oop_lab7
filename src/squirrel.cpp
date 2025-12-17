#include "../include/squirrel.hpp"
#include "../include/orc.hpp"
#include "../include/bear.hpp"

Squirrel::Squirrel(int x, int y) : NPC(NpcType::SquirrelType, x, y) {
    step_range = 5;
    kill_range = 5;
}

Squirrel::Squirrel(std::istream &is) : NPC(NpcType::SquirrelType, is) {
    step_range = 5;
    kill_range = 5;
}

void Squirrel::print() {
    std::cout << *this;
}

void Squirrel::save(std::ostream &os) const {
    os << "\n" << static_cast<int>(type) << "\n";
    NPC::save(os);
}

bool Squirrel::accept(std::shared_ptr<NPC> visitor) {
    return visitor->fight(std::dynamic_pointer_cast<Squirrel>(shared_from_this()));
}

// Белки не хотят воевать

bool Squirrel::fight(std::shared_ptr<Squirrel> other) {
    fight_notify(other, false);
    return false;
}

bool Squirrel::fight(std::shared_ptr<Orc> other) {
    fight_notify(other, false);
    return false;
}

bool Squirrel::fight(std::shared_ptr<Bear> other) {
    fight_notify(other, false);
    return false;
}

std::ostream &operator<<(std::ostream &os, Squirrel &squirrel) {
    os << "Белка: " << *static_cast<NPC*>(&squirrel) << "\n";
    return os;
}
