#include "../include/npc.hpp"
#include "../include/squirrel.hpp"
#include "../include/bear.hpp"
#include "../include/orc.hpp"
#include "../include/fight_logic.hpp"

#include <ctime>
#include <thread>
#include <atomic>
#include <array>
#include <fstream>
#include <mutex>
#include <shared_mutex>

std::mutex cout_mutex;
std::mutex battle_mutex;
std::mutex combat_queue_mutex;
std::shared_mutex npc_mutex;

std::queue<Battle> combat_queue;


class TextObserver : public Observer {
public:
    void fight(const std::shared_ptr<NPC> attacker, const std::shared_ptr<NPC> defender, bool win) override {
        if (win) {
            std::cout << "\n";
            attacker->print();
            std::cout << " --> ";
            defender->print();
        }
    }
};

class FileObserver : public Observer {
private:
    std::ofstream log_file;

public:
    FileObserver(const std::string& filename) {
        log_file.open(filename, std::ios_base::app);
    }

    void fight(const std::shared_ptr<NPC> attacker, const std::shared_ptr<NPC> defender, bool win) override {
        if (win && log_file.is_open()) {
            log_file << *attacker << " убивает " << *defender << "\n";
        }
    }
};

// Глобальные наблюдатели
std::shared_ptr<TextObserver> text_observer = std::make_shared<TextObserver>();
std::shared_ptr<FileObserver> file_observer = std::make_shared<FileObserver>("log.txt");

// Фабрика для создания NPC из потока или по типу
std::shared_ptr<NPC> create_npc(std::istream &is) {
    std::shared_ptr<NPC> result;
    int type_code;
    if (is >> type_code) {
        switch (static_cast<NpcType>(type_code)) {
            case NpcType::SquirrelType:
                result = std::make_shared<Squirrel>(is);
                break;
            case NpcType::BearType:
                result = std::make_shared<Bear>(is);
                break;
            case NpcType::OrcType:
                result = std::make_shared<Orc>(is);
                break;
            default:
                break;
        }
    }

    if (result) {
        result->subscribe(text_observer);
        result->subscribe(file_observer);
    }

    return result;
}

std::shared_ptr<NPC> create_npc(NpcType type, int x, int y) {
    std::shared_ptr<NPC> result;

    switch (type) {
        case NpcType::SquirrelType:
            result = std::make_shared<Squirrel>(x, y);
            break;
        case NpcType::BearType:
            result = std::make_shared<Bear>(x, y);
            break;
        case NpcType::OrcType:
            result = std::make_shared<Orc>(x, y);
            break;
        default:
            break;
    }

    if (result) {
        result->subscribe(text_observer);
        result->subscribe(file_observer);
    }

    return result;
}

// Сохранение в файл
void save_npcs(const npc_set_t &npcs, const std::string &filename) {
    std::ofstream fs(filename);
    fs << npcs.size() << "\n";
    for (const auto &npc : npcs) {
        npc->save(fs);
    }
    fs.flush();
    fs.close();
}

// Загрузка из файла
npc_set_t load_npcs(const std::string &filename) {
    npc_set_t result;
    std::ifstream is(filename);
    if (is.good() && is.is_open()) {
        int count;
        is >> count;

        for (int i = 0; i < count; ++i) {
            result.insert(create_npc(is));
        }

        is.close();
    }

    return result;
}

std::ostream &operator<<(std::ostream &os, const npc_set_t &npcs) {
    for (const auto &npc : npcs) {
        npc->print();
    }
    return os;
}

using namespace std::chrono_literals;
std::atomic<bool> should_stop = false;

void npc_movement_processor(set_t &all_npcs)
{
    while (!should_stop) {
        {
            std::shared_lock<std::shared_mutex> npc_lock(npc_mutex);

            for (const auto &current_npc : all_npcs) {
                if (current_npc->is_alive()) {
                    // Передвижение
                    current_npc->move(all_npcs);

                    // Поиск ближайших для атаки
                    for (const auto &other_npc : all_npcs) {
                        if (current_npc.get() != other_npc.get() && 
                            other_npc->is_alive() && 
                            current_npc->is_close(other_npc, current_npc->get_range())) {
                            std::lock_guard<std::mutex> task_lock(combat_queue_mutex);
                            combat_queue.push({current_npc, other_npc});
                        }
                    }
                }
            }
        }
        
        std::this_thread::sleep_for(1s);
    }
}

void combat_resolution_thread()
{
    while (!should_stop) {
        Battle active_combat;
        bool has_task = false;

        {
            std::lock_guard<std::mutex> task_lock(combat_queue_mutex);

            if (!combat_queue.empty()) {
                active_combat = combat_queue.front();
                combat_queue.pop();
                has_task = true;
            }
        }

        if (has_task) {
            battle(active_combat);
        } else {
            std::this_thread::sleep_for(10ms);
        }
    }
}

void field_display_thread(set_t &all_npcs)
{
    const int grid_size = 25;
    const int cell_width = MAP_SIZE / grid_size;
    const int cell_height = MAP_SIZE / grid_size;
    
    // 2 массива для символов и для счетчиков
    std::array<char, grid_size * grid_size> symbols;
    std::array<int, grid_size * grid_size> counts;

    while (!should_stop) {
        int alive_count = 0;
        
        {
            std::shared_lock<std::shared_mutex> npc_lock(npc_mutex);
            
            symbols.fill('.');
            counts.fill(0);
            
            for (const std::shared_ptr<NPC> &entity : all_npcs) {
                if (entity->is_alive()) {
                    alive_count++;
                    const auto [x_coord, y_coord] = entity->position();
                    int grid_x = x_coord / cell_width;
                    int grid_y = y_coord / cell_height;
                    
                    if (grid_x >= 0 && grid_x < grid_size && 
                        grid_y >= 0 && grid_y < grid_size) {
                        int idx = grid_x + grid_size * grid_y;
                        counts[idx]++;

                        if (counts[idx] == 1) {
                            switch (entity->get_type()) {
                                case NpcType::BearType: symbols[idx] = 'B'; break;
                                case NpcType::SquirrelType: symbols[idx] = 'S'; break;
                                case NpcType::OrcType: symbols[idx] = 'O'; break;
                                default: symbols[idx] = '?'; break;
                            }
                        }
                    }
                }
            }
        }

        {
            std::lock_guard<std::mutex> output_lock(cout_mutex);

            std::cout << "\n          ТЕКУЩЕЕ ПОЛЕ БОЯ          \n";
            std::cout << "Всего живых NPC: " << alive_count << "\n";
            std::cout << "--------------------------------------\n";
            
            for (int row = 0; row < grid_size; ++row) {
                for (int col = 0; col < grid_size; ++col) {
                    int idx = col + row * grid_size;
                    
                    if (counts[idx] == 0) {
                        std::cout << " . ";
                    } else if (counts[idx] == 1) {
                        std::cout << " " << symbols[idx] << " ";
                    } else {
                        if (counts[idx] > 9) {
                            std::cout << " * ";
                        } else {
                            std::cout << " " << counts[idx] << " ";
                        }
                    }
                }
                std::cout << '\n';
            }
            std::cout << "--------------------------------------\n";
        }
        
        std::this_thread::sleep_for(1000ms);
    }
}

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    set_t npcs;

    {
        // Блокировка мьютекса, lock_guard автоматически освобождает мьютекс при выходе из блока
        std::lock_guard<std::shared_mutex> lock(npc_mutex);
        std::cout << "Генерация NPC..." << std::endl;
        for (size_t idx = 0; idx < NPC_COUNT; ++idx) {
            int npc_type = std::rand() % 3 + 1;

            int spawn_x = std::rand() % (MAP_SIZE + 1);
            int spawn_y = std::rand() % (MAP_SIZE + 1);

            auto npc = create_npc(static_cast<NpcType>(npc_type), spawn_x, spawn_y);
            if (npc) {
                npcs.insert(npc);
            }
        }
    }

    std::cout << "\nЗапуск боевой симуляции...\n" << std::endl;
    std::thread movement_thread(npc_movement_processor, std::ref(npcs));
    std::thread combat_thread(combat_resolution_thread);
    std::thread display_thread(field_display_thread, std::ref(npcs));

    std::this_thread::sleep_for(std::chrono::seconds(GAME_TIME));

    should_stop = true;

    // join() блокирует главный поток до завершения рабочего потока
    if (movement_thread.joinable()) {
        movement_thread.join();
    }
    if (combat_thread.joinable()) {
        combat_thread.join();
    }
    if (display_thread.joinable()) {
        display_thread.join();
    }

    std::cout << "Оставшиеся на поле боя:\n" << std::endl;

    // shared_lock позволяет нескольким потокам читать данные одновременно
    std::shared_lock<std::shared_mutex> lock(npc_mutex);
    bool survivors_found = false;
    for (const auto &remaining_npc : npcs) {
        if (remaining_npc->is_alive()) {
            survivors_found = true;
            std::cout << "> ";
            remaining_npc->print();
        }
    }

    if (!survivors_found) {
        std::cout << "На поле боя не осталось никого...\n";
    }

    std::cout << std::endl;

    return 0;
}
