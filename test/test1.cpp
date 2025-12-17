#include "gtest/gtest.h"
#include "../include/squirrel.hpp"
#include "../include/orc.hpp"
#include "../include/bear.hpp"
#include "../include/npc.hpp"
#include <memory>

// Утилита для создания NPC без глобальных наблюдателей
std::shared_ptr<NPC> make_npc(NpcType type, int x, int y) {
    switch (type) {
        case NpcType::SquirrelType:
            return std::make_shared<Squirrel>(x, y);
        case NpcType::OrcType:
            return std::make_shared<Orc>(x, y);
        case NpcType::BearType:
            return std::make_shared<Bear>(x, y);
        default:
            return nullptr;
    }
}

// ====================================================================
// Тесты для is_close
// ====================================================================

TEST(NpcTest, IsCloseDistance) {
    auto squirrel = make_npc(NpcType::SquirrelType, 0, 0);
    auto orc = make_npc(NpcType::OrcType, 3, 4);    // расстояние 5
    auto bear = make_npc(NpcType::BearType, 6, 8);  // расстояние 10

    EXPECT_TRUE(squirrel->is_close(orc, 5));
    EXPECT_FALSE(squirrel->is_close(orc, 4));

    EXPECT_TRUE(squirrel->is_close(bear, 10));
    EXPECT_FALSE(squirrel->is_close(bear, 9));

    EXPECT_TRUE(squirrel->is_close(squirrel, 0)); // сам с собой
}

// ====================================================================
// Тесты для логики боя (Visitor Pattern)
// ====================================================================

// Правила боя из вашего кода:
// - Орки убивают медведей и орков (да, орк убивает орка)
// - Белки никого не убивают (пацифисты)
// - Медведи убивают белок

TEST(FightTest, OrcAttacks) {
    auto orc = make_npc(NpcType::OrcType, 0, 0);
    auto another_orc = make_npc(NpcType::OrcType, 0, 0);
    auto bear = make_npc(NpcType::BearType, 0, 0);
    auto squirrel = make_npc(NpcType::SquirrelType, 0, 0);

    // Орк атакует орка -> побеждает (согласно вашему коду)
    EXPECT_TRUE(another_orc->accept(orc));

    // Орк атакует медведя -> побеждает
    EXPECT_TRUE(bear->accept(orc));

    // Орк атакует белку -> проигрывает (белка не убивает, но и не умирает)
    EXPECT_FALSE(squirrel->accept(orc));
}

TEST(FightTest, SquirrelAttacks) {
    auto squirrel = make_npc(NpcType::SquirrelType, 0, 0);
    auto orc = make_npc(NpcType::OrcType, 0, 0);
    auto bear = make_npc(NpcType::BearType, 0, 0);
    auto another_squirrel = make_npc(NpcType::SquirrelType, 0, 0);

    // Белка атакует орка -> ничья (белка не убивает)
    EXPECT_FALSE(orc->accept(squirrel));

    // Белка атакует медведя -> ничья (белка не убивает)
    EXPECT_FALSE(bear->accept(squirrel));

    // Белка атакует белку -> ничья
    EXPECT_FALSE(another_squirrel->accept(squirrel));
}

TEST(FightTest, BearAttacks) {
    auto bear = make_npc(NpcType::BearType, 0, 0);
    auto squirrel = make_npc(NpcType::SquirrelType, 0, 0);
    auto orc = make_npc(NpcType::OrcType, 0, 0);
    auto another_bear = make_npc(NpcType::BearType, 0, 0);

    // Медведь атакует белку -> побеждает
    EXPECT_TRUE(squirrel->accept(bear));

    // Медведь атакует орка -> проигрывает
    EXPECT_FALSE(orc->accept(bear));

    // Медведь атакует медведя -> ничья
    EXPECT_FALSE(another_bear->accept(bear));
}

// ====================================================================
// Тесты для проверки симметричности боев
// ====================================================================

TEST(FightTest, SymmetricBattles) {
    // Проверка, что если А убивает Б, то Б не убивает А
    
    auto orc = make_npc(NpcType::OrcType, 0, 0);
    auto bear = make_npc(NpcType::BearType, 0, 0);
    
    // Орк убивает медведя
    EXPECT_TRUE(bear->accept(orc));
    // Медведь не убивает орка
    EXPECT_FALSE(orc->accept(bear));
    
    auto squirrel = make_npc(NpcType::SquirrelType, 0, 0);
    
    // Медведь убивает белку
    EXPECT_TRUE(squirrel->accept(bear));
    // Белка не убивает медведя
    EXPECT_FALSE(bear->accept(squirrel));
}

// ====================================================================
// Тесты для функции fight (если она у вас есть)
// ====================================================================

// Тестовый наблюдатель для боев
class TestFightObserver : public Observer {
public:
    struct FightRecord {
        std::shared_ptr<NPC> attacker;
        std::shared_ptr<NPC> defender;
        bool attacker_won;
    };
    
    std::vector<FightRecord> fights;
    
    void fight(const std::shared_ptr<NPC> attacker,
                  const std::shared_ptr<NPC> defender,
                  bool attacker_won) override {
        fights.push_back({attacker, defender, attacker_won});
    }
};

TEST(SimulationTest, BasicFightLogic) {
    // Создаем NPC
    auto orc1 = make_npc(NpcType::OrcType, 0, 0);
    auto orc2 = make_npc(NpcType::OrcType, 5, 0);   // расстояние 5
    auto bear = make_npc(NpcType::BearType, 3, 4);  // расстояние 5 от орка1
    auto squirrel = make_npc(NpcType::SquirrelType, 0, 6); // расстояние 6 от орка1

    // Подписываем на тестового наблюдателя
    auto observer = std::make_shared<TestFightObserver>();
    orc1->subscribe(observer);
    orc2->subscribe(observer);
    bear->subscribe(observer);
    squirrel->subscribe(observer);

    // Тестируем отдельные бои
    
    // Орк1 атакует Орка2 -> орк2 погибает
    EXPECT_TRUE(orc2->accept(orc1));
    
    // Орк1 атакует медведя -> медведь погибает
    EXPECT_TRUE(bear->accept(orc1));
    
    // Медведь атакует белку -> белка погибает
    EXPECT_TRUE(squirrel->accept(bear));
    
    // Проверяем записи о боях
    EXPECT_GE(observer->fights.size(), 3);
    
    // Проверяем первую запись (орк vs орк)
    if (observer->fights.size() > 0) {
        auto record = observer->fights[0];
        EXPECT_EQ(record.attacker, orc1);
        // Защитник должен быть orc2 или bear в зависимости от порядка
        EXPECT_TRUE(record.defender == orc2 || record.defender == bear || record.defender == squirrel);
        EXPECT_TRUE(record.attacker_won); // Орк всегда побеждает в своих боях
    }
}

TEST(SimulationTest, SquirrelPacifism) {
    auto squirrel = make_npc(NpcType::SquirrelType, 0, 0);
    auto orc = make_npc(NpcType::OrcType, 0, 0);
    auto bear = make_npc(NpcType::BearType, 0, 0);
    auto another_squirrel = make_npc(NpcType::SquirrelType, 0, 0);
    
    // Проверяем, что белка никого не убивает
    EXPECT_FALSE(orc->accept(squirrel));
    EXPECT_FALSE(bear->accept(squirrel));
    EXPECT_FALSE(another_squirrel->accept(squirrel));
}

TEST(SimulationTest, BearVsOrcRelationship) {
    auto bear = make_npc(NpcType::BearType, 0, 0);
    auto orc = make_npc(NpcType::OrcType, 0, 0);
    
    // Медведь проигрывает орку
    EXPECT_FALSE(orc->accept(bear));
    
    // Орк побеждает медведя
    EXPECT_TRUE(bear->accept(orc));
}

TEST(SimulationTest, PositionAndDistance) {
    auto orc = make_npc(NpcType::OrcType, 0, 0);
    auto bear = make_npc(NpcType::BearType, 10, 0);
    
    // NPC на расстоянии 10 друг от друга
    EXPECT_TRUE(orc->is_close(bear, 10));
    EXPECT_FALSE(orc->is_close(bear, 9));
    
    // Перемещаем медведя ближе
    auto bear_near = make_npc(NpcType::BearType, 5, 0);
    EXPECT_TRUE(orc->is_close(bear_near, 5));
    EXPECT_TRUE(orc->is_close(bear_near, 6));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
