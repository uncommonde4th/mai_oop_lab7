#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <set>
#include <cstring>
#include <math.h>


constexpr size_t MAP_SIZE = 500;
constexpr size_t NPC_COUNT = 50;
constexpr size_t GAME_TIME = 30;

class NPC;

using set_t = std::set<std::shared_ptr<NPC>>;
using npc_set_t = set_t;

enum class NpcType {
    Unidentified = 0,
    OrcType = 1,
    SquirrelType = 2,
    BearType = 3
};

struct Observer {
    virtual void fight(const std::shared_ptr<class NPC> attacker, const std::shared_ptr<class NPC> defender, bool win) = 0;
    virtual ~Observer() = default;
};

struct NPC : public std::enable_shared_from_this<NPC> {
protected:
    NpcType type;
    int x{0};
    int y{0};
    int step_range{0};
    int kill_range{0};
    bool alive{true};
    std::vector<std::shared_ptr<Observer>> observers;

public:
    NPC(NpcType t, int _x, int _y);
    NPC(NpcType t, std::istream &is);

    NpcType get_type() const { return type; }
    std::pair<int, int> position() const { return {x, y}; }
    size_t get_speed() const { return step_range; }
    size_t get_range() const { return kill_range; }
    bool is_alive() const { return alive; }

    void die() { alive = false; }
    void move(const npc_set_t &others);
    int roll_dice();
    double distance(std::shared_ptr<NPC> other) const;

    void subscribe(std::shared_ptr<Observer> observer);
    void fight_notify(const std::shared_ptr<NPC> defender, bool win);
    bool is_close(const std::shared_ptr<NPC> &other, size_t distance) const;
    
    virtual bool accept(std::shared_ptr<NPC> visitor) = 0;

    virtual bool fight(std::shared_ptr<class Orc> other) = 0;
    virtual bool fight(std::shared_ptr<class Squirrel> other) = 0;
    virtual bool fight(std::shared_ptr<class Bear> other) = 0;

    virtual void print() = 0;
    virtual void save(std::ostream &os) const;

    friend std::ostream &operator<<(std::ostream &os, NPC &npc);
};

