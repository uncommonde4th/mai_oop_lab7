#include "../include/npc.hpp"

#include <algorithm>


NPC::NPC(NpcType t, int _x, int _y) : type(t), x(_x), y(_y) {}

NPC::NPC(NpcType t, std::istream &is) : type(t) {
    is >> x;
    is >> y;
}

void NPC::move(const set_t &others)
{
    if (!is_alive()) return;

    std::shared_ptr<NPC> nearest = nullptr;
    double min_dist = 100000;

    // поиск ближайшего живого NPC
    for (const auto& other: others) {
        if (other->is_alive() && (other.get() != this)) {
            double dist = distance(other);
            if (nearest == nullptr || dist < min_dist) {
                nearest = other;
                min_dist = dist;
            }
        }
    }

    int new_x = x;
    int new_y = y;

    if (nearest != nullptr) {
        // движение к ближайшему NPC
        double dx = (double)nearest->x - x;
        double dy = (double)nearest->y - y;
        double angle = std::atan2(dy, dx);

        new_x += (int)(std::cos(angle) * step_range);
        new_y += (int)(std::sin(angle) * step_range);
    } else {
        // случайное движение
        new_x += rand() % (2 * step_range + 1) - step_range;
        new_y += rand() % (2 * step_range + 1) - step_range;
    }

    // ограничение координат границами карты
    new_x = std::max(0, std::min(new_x, (int)MAP_SIZE - 1));
    new_y = std::max(0, std::min(new_y, (int)MAP_SIZE - 1));

    x = new_x;
    y = new_y;
}

int NPC::roll_dice()
{
    return std::rand() % 6;
}

double NPC::distance(std::shared_ptr<NPC> other) const
{
    return std::sqrt(std::pow(x - other->x, 2) + std::pow(y - other->y, 2));
}

void NPC::subscribe(std::shared_ptr<Observer> observer) {
    observers.push_back(observer);
}

void NPC::fight_notify(const std::shared_ptr<NPC> defender, bool win) {
    for (auto &observer : observers) {
        observer->fight(shared_from_this(), defender, win);
    }
}

bool NPC::is_close(const std::shared_ptr<NPC> &other, size_t distance) const {
    int dx = x - other->x;
    int dy = y - other->y;
    return (dx * dx + dy * dy) <= (distance * distance);
}

void NPC::save(std::ostream &os) const {
    os << x << " " << y << " ";
}

std::ostream &operator<<(std::ostream &os, NPC &npc) {
    os << "{x:" << npc.x << ", y:" << npc.y << "} ";
    return os;
}
