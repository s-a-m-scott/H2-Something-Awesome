#ifndef WAVES_HPP
#define WAVES_HPP

#include "splashkit.h"
#include "constants_enums.hpp"
#include "enemies.hpp"
#include <algorithm> 


struct wave {
    int count; //number of enemies in this wave
    int wave_capacity; //total combined value of enemies in wave
    vector<enemy_type> spawn_blacklist = {boss, split_child};
    vector<enemy_type> enemy_types; // the enemies making up this wave
    vector<enemy_type> enemy_types_to_spawn; // the enemies that can spawn in this wave

    void generate_wave(int wave_number) {
        enemy_types.clear();
        enemy_types_to_spawn.clear();

        //special waves
        
        if (generate_first_wave(wave_number)) return;
        if (generate_boss_wave(wave_number)) return;


        //regular wave generation
        wave_capacity = WAVE_CAPACITY_BASE + (wave_number * WAVE_CAPACITY_INCREASE);
        int types_weight = calculate_types_of_wave(wave_number);
        generate_enemy_vector(types_weight);

    }

    int calculate_types_of_wave(int wave_number) {
        //determine which types of enemies will be spawned this wave
        enemy temp_enemy;
        count = 0;
        enemy_type type_spawn;
        int types_weight = 0;
        while (count < ENEMY_TYPES_WAVE) {
            type_spawn = (enemy_type)rnd(0,ENEMY_TYPES);
            if (find(spawn_blacklist.begin(), spawn_blacklist.end(), type_spawn) != spawn_blacklist.end()) continue;
            if (find(enemy_types_to_spawn.begin(), enemy_types_to_spawn.end(), type_spawn) != enemy_types_to_spawn.end()) continue;
            temp_enemy.type_init(type_spawn);
            if (temp_enemy.earliest_wave > wave_number) continue; //dont spawn this enemy yet)
            enemy_types_to_spawn.push_back(temp_enemy.type);
            types_weight += temp_enemy.spawn_weight;
            count++;
        }
        return types_weight;
    }

    void generate_enemy_vector(int types_weight) {
        //fill enemy type vector with these enemies, depending on spawn weights
        int weight_counter = 0;
        count = 0;
        int type_weight;
        enemy temp_enemy;
        while (wave_capacity > 0) {
            type_weight = rnd(0,types_weight);
            weight_counter = 0;
            for (int i = 0; i < enemy_types_to_spawn.size(); i++) {
                temp_enemy.type_init(enemy_types_to_spawn[i]);
                weight_counter += temp_enemy.spawn_weight;
                if (weight_counter < type_weight) continue; //continue because this is the wrong enemy
                enemy_types.push_back(temp_enemy.type);
                wave_capacity -= temp_enemy.spawn_value;
                count++;
                break; //break because we've found an enemy
            }
        }
    }

    //special waves
    bool generate_first_wave(int wave_number) {
        if (wave_number != 0) return false;
        count = 10;
        for (int i = 0; i < count; i++) enemy_types.push_back(basic);
        return true;
    }

    bool generate_boss_wave(int wave_number) {
        wave_number += 1; // starts at 0, so displays 1 higher. calculation must match with displayed value
        if (wave_number % INFINITE_WAVE_BOSS_INTERVAL == 0 && wave_number >= FIRST_BOSS_WAVE) {
            count = ((wave_number - FIRST_BOSS_WAVE) % INFINITE_WAVE_BOSS_INTERVAL) + 1;
            for (int i = 0; i < count; i++) enemy_types.push_back(boss);
            return true;
        } else return false;
    }
};


#endif