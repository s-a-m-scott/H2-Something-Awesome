#ifndef ENEMIES_HPP
#define ENEMIES_HPP

#include "splashkit.h"
#include "constants_enums.hpp"
#include "utility.hpp"
#include "pathing.hpp"

struct enemy {
    //instance definitions
    enemy_type type;
    difficulty diff;
    int current_hit_points;
    float x;
    float y;
    float dist_from_end;
    int current_goal_index;
    point_2d goal_pos;
    bool at_current_goal;
    bool alive;
    int index;
    int hit_flash_time = 0;
    int instance_id;

    //status effects
    int poison_ticks = 0;
    int poison_damage = 0;
    int poison_tick_rate = 0;
    int poison_timer = 0;
    int freeze_time = 0;

    //type definitions
    int size;
    float speed;
    int max_hit_points;
    color _color;
    int spawn_weight; //chance to spawn when in wave
    int spawn_value; //amount removed from wave capacity when spawned
    int money_value;
    int earliest_wave; //earliest wave this enemy can spawn in

    enemy() {      
        //variable qualities
        instance_id = rand();
        alive = true;
        x = 0;
        y = 0;
        current_goal_index = 0;
        at_current_goal = false;
        goal_pos = point_at(0,0);
        poison_ticks = 0;
    }

    void draw() {
        switch (type) {
            default: {
                draw_body_standard();
                draw_hp_bar_standard();
            } break;
            case splitting: {
                draw_body_standard();
                draw_body_splitting();
                draw_hp_bar_standard();
            } break;
        }
    }

    void loop(path &level, int &lives, int &money, int current_wave, vector<enemy> &enemies) {
        switch (type) {
            default: {
                calc_dist_to_end(level);
                standard_move();
                take_poison_damage();
                standard_goal_reset(level, lives);
                standard_death(money, current_wave);
            } break;
            case splitting: {
                calc_dist_to_end(level);
                standard_move();
                take_poison_damage();
                standard_goal_reset(level, lives);
                splitting_death(money, current_wave, enemies);
            } break;
            case elite: {
                calc_dist_to_end(level);
                elite_move();
                take_poison_damage();
                standard_goal_reset(level, lives);
                standard_death(money, current_wave);
            } break;
        }
    }
    #pragma region standard loop functions

    void standard_move() {
        float dist_to_node = distance_between_points(x, y, goal_pos.x, goal_pos.y); //still should calculate distance when frozen, just in case
        if (freeze_time > timer_ticks(GAME_TIMER)) return;
        float move_dir = direction_between_points(x, y, goal_pos.x, goal_pos.y);
        if (dist_to_node < speed) {
            at_current_goal = true;
            x = goal_pos.x;
            y = goal_pos.y;
        }
        else {
            x += lengthdir_x(speed, move_dir);
            y += lengthdir_y(speed, move_dir);
        }
    }

    void take_poison_damage() {
        if (poison_ticks == 0) return;
        if (poison_timer > timer_ticks(GAME_TIMER)) return;
        current_hit_points -= poison_damage;
        poison_timer = timer_ticks(GAME_TIMER) + poison_tick_rate;
        poison_ticks--;
    }

    void calc_dist_to_end(path &level) {
        dist_from_end = distance_between_points(x, y, goal_pos.x, goal_pos.y);
        for (int i = current_goal_index + 1; i < PATH_LENGTH; i++) {
            dist_from_end += distance_between_points(level.nodes[i].x, level.nodes[i].y, level.nodes[i-1].x, level.nodes[i-1].y);
        }
    }

    void kill_enemy(vector<enemy> &enemies) {
        vector<enemy>::iterator enemy_to_kill = enemies.begin() + index;
        for (int i = index + 1; i < enemies.size(); i++) {
            enemies[i].index--;
        }
        enemies.erase(enemy_to_kill);
    }

    void standard_goal_reset(path &level, int &lives) {
        if (at_current_goal) {
            current_goal_index++;
            if (current_goal_index >= PATH_LENGTH) {
                alive = false;
                lives--;
                return;
            }
            goal_pos = point_at(level.nodes[current_goal_index].x, level.nodes[current_goal_index].y);
            at_current_goal = false;
        }
    }

    void standard_death(int &money, int current_wave) {
        if (current_hit_points <= 0) {
            money += random_range(money_value*0.9, money_value*1.1) * pow(MONEY_WAVE_INCREMENT,current_wave);
            alive = false;
        }
    }
   #pragma endregion

   #pragma region type-specific loop functions

    void splitting_death(int &money, int current_wave, vector<enemy> &enemies) {
        if (current_hit_points <= 0) {
            money += random_range(money_value*0.9, money_value*1.1) * pow(MONEY_WAVE_INCREMENT,current_wave);
            alive = false;
            int offset = size;
            double direction = direction_between_points(x, y, goal_pos.x, goal_pos.y);
            for (int i = 0; i < ENEMY_SPLITTING_CHILDREN; i++) {
                enemy child = enemy();
                enemy* child_ptr = child.spawn_enemy(split_child, enemies, current_wave, diff);
                child_ptr->x = x + lengthdir_x(offset * (i-1), direction);
                child_ptr->y = y + lengthdir_y(offset * (i-1), direction);
                child_ptr->current_goal_index = current_goal_index;
                child_ptr->at_current_goal = false;
                child_ptr->goal_pos = goal_pos;
            }
        }
    }

    void elite_move() {
        //move faster when hp is lower
        double adaptive_speed = speed;
        adaptive_speed += speed * ((double)current_hit_points / (double)max_hit_points) * 4;//triple speed at full hp
        float dist_to_node = distance_between_points(x, y, goal_pos.x, goal_pos.y); //still should calculate distance when frozen, just in case
        if (freeze_time > timer_ticks(GAME_TIMER)) return;
        float move_dir = direction_between_points(x, y, goal_pos.x, goal_pos.y);
        if (dist_to_node < adaptive_speed) {
            at_current_goal = true;
        }
        else {
            x += lengthdir_x(adaptive_speed, move_dir);
            y += lengthdir_y(adaptive_speed, move_dir);
        }
    }

   #pragma endregion

    #pragma region draw
    void draw_body_standard() {
        circle enemy_shape = circle_at(x,y,size);
        color _fill = _color;
        color _outline = COLOR_BLACK;
        if (freeze_time > timer_ticks(GAME_TIMER)) _outline = COLOR_LIGHT_BLUE;
        if (hit_flash_time > timer_ticks(GAME_TIMER)) {
            _fill = COLOR_WHITE;
            _outline = COLOR_GRAY;
        }
        fill_circle(_fill, enemy_shape);
        draw_circle(_outline, enemy_shape);
    }

    void draw_hp_bar_standard() {
        if (current_hit_points == max_hit_points) return; //draw nothing if full hp
        //background
        point_2d top_left_corner = point_at(x-ENEMY_HP_BAR_WIDTH, y - size - ENEMY_HP_BAR_HEIGHT);
        rectangle hp_bar_bg = rectangle_from(top_left_corner, ENEMY_HP_BAR_WIDTH * 2, ENEMY_HP_BAR_HEIGHT);
        fill_rectangle(ENEMY_HP_BG_COLOR, hp_bar_bg);
        //hp left
        float hp_bar_width = max(((float)current_hit_points / (float)max_hit_points), 0.0f) * (ENEMY_HP_BAR_WIDTH * 2);
        rectangle hp_bar = rectangle_from(top_left_corner, hp_bar_width,ENEMY_HP_BAR_HEIGHT);
        if (poison_ticks > 0) fill_rectangle(COLOR_GREEN, hp_bar);
        else fill_rectangle(ENEMY_HP_COLOR, hp_bar);
        //outline
        draw_rectangle(COLOR_BLACK, hp_bar_bg);
    }
    
    void draw_body_splitting() {
        //children
        double child_offset_angle = (2 * PI) / ENEMY_SPLITTING_CHILDREN;
        double angle = timer_ticks(GAME_TIMER) * 0.004;
        double child_dist = size * 0.7;
        double child_size = size * 0.25;
        for (int i = 0; i < ENEMY_SPLITTING_CHILDREN; i++) {
            circle child_shape = circle_at(x + lengthdir_x(child_dist, angle), y + lengthdir_y(child_dist, angle), child_size);
            fill_circle(_color, child_shape);
            draw_circle(COLOR_BLACK, child_shape);
            angle += child_offset_angle;
        }
    }
    #pragma endregion

    #pragma region enemy type definitions

    void type_init(enemy_type _type) {
        type = _type;
        switch (type) {
            case basic: basic_enemy(); break;
            case fast: fast_enemy(); break;
            case beefy: beefy_enemy(); break;
            case boss: boss_enemy(); break;
            case tiny: tiny_enemy(); break;
            case splitting: splitting_enemy(); break;
            case split_child: split_child_enemy(); break;
            case flying: flying_enemy(); break;
            case elite: elite_enemy(); break;
        }
    }

    void basic_enemy() {
        size = 18;
        speed = 3.5;
        max_hit_points = 60;
        _color = COLOR_CRIMSON;
        spawn_weight = 70;
        spawn_value = 3;
        money_value = 3;
        earliest_wave = 0;
    }

    void fast_enemy() {
        size = 15;
        speed = 8;
        max_hit_points = 30;
        _color = COLOR_HOT_PINK;

        spawn_weight = 50;
        spawn_value = 5;
        money_value = 5;
        earliest_wave = 0;
    }

    void beefy_enemy() {
        size = 25;
        speed = 1.8;
        max_hit_points = 180;
        _color = COLOR_ORANGE;
        spawn_weight = 40;
        spawn_value = 10;
        money_value = 10;
        earliest_wave = 0;
    }

    void tiny_enemy() {
        size = 10;
        speed = 5;
        max_hit_points = 10;
        _color = COLOR_YELLOW;
        spawn_weight = 80;
        spawn_value = 1;
        money_value = 3;
        earliest_wave = 0;
    }

    void splitting_enemy() {
        size = 24;
        speed = 2;
        max_hit_points = 120;
        _color = COLOR_GREEN;
        spawn_weight = 30;
        spawn_value = 8;
        money_value = 15;
        earliest_wave = 14;
    }

    void split_child_enemy() {
        size = 12;
        speed = 5;
        max_hit_points = 50;
        _color = COLOR_GREEN;
        money_value = 5;
    }

    void flying_enemy() {
        size = 15;
        speed = 5;
        max_hit_points = 40;
        _color = COLOR_SKY_BLUE;
        spawn_weight = 90;
        spawn_value = 4;
        money_value = 7;
        earliest_wave = 17;
        goal_pos = point_at(PLAYABLE_VIEW_WIDTH,PLAYABLE_VIEW_HEIGHT);
        current_goal_index = PATH_LENGTH;
    }

    void elite_enemy() {
        size = 25;
        speed = 2;
        max_hit_points = 320;
        _color = COLOR_PURPLE;
        spawn_weight = 40;
        spawn_value = 20;
        money_value = 30;
        earliest_wave = 22;
    }

    void boss_enemy() {
        size = 32;
        speed = 0.8;
        max_hit_points = 3500;
        _color = COLOR_DARK_RED;
        spawn_weight = 1;
        spawn_value = 100;
        money_value = 100;
    }

    #pragma endregion

    enemy* spawn_enemy(enemy_type type, vector<enemy> &enemies, int current_wave, difficulty diff) {
        type_init(type);
        this->diff = diff;
        switch (diff) {
            case easy: {
                max_hit_points *= 0.8; 
                speed *= 0.9;
            } break;
            case medium: {
                //do nothing
            } break;
            case hard: {
                max_hit_points *= 1.2; 
                speed *= 1.1;
            } break;
        }
        current_hit_points = max_hit_points;
        current_hit_points *= pow(ENEMY_HP_SCALE, current_wave); // enemies get more hitpoints each wave
        max_hit_points = current_hit_points;
        index = enemies.size();
        enemies.push_back(*this);
        return &enemies[enemies.size()-1];
    }
};



#endif
