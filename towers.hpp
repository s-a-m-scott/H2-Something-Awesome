#ifndef TOWERS_HPP
#define TOWERS_HPP

#include "splashkit.h"
#include "enemies.hpp"
#include "constants_enums.hpp"
#include "utility.hpp"
#include "pathing.hpp"

#include <random>

struct explosion {
    point_2d pos;
    int radius;
    int death_time;
};

struct tower_projectile {
    point_2d pos;
    float direction;
    int instance_id = rand();
    int damage;
    float speed;
    color _color;
    int index;
    int pierce;
    int size;
    int death_time;
    vector<int> enemies_hit;

    projectile_type type;
    int radius;
    int poison_ticks;
    int poison_tick_rate;
    int freeze_time;

    void control_loop(vector<enemy> &enemies) {
        if (timer_ticks(GAME_TIMER) > death_time) {
            pierce = 0;
            return;
        }
        //move
        pos.x += lengthdir_x(speed, direction);
        pos.y += lengthdir_y(speed, direction);
        //collide
        for (int i = 0; i < enemies.size(); i++) {
            if (pierce <= 0) return; // no more pierce, don't bother checking
            circle enemy_circle = circle_at(enemies[i].x, enemies[i].y, enemies[i].size);
            circle my_circle = circle_at(pos.x, pos.y, size);

            if (circles_intersect(enemy_circle, my_circle) && !have_hit_enemy(enemies[i])) {
                //hit enemy
                switch (type) {
                    default: {
                        enemies[i].current_hit_points -= damage;
                    } break;
                    case p_explosive: {
                        //explode
                        enemies_hit.push_back(enemies[i].instance_id);
                        for (int j = 0; j < enemies.size(); j++) {
                            if (distance_between_points(enemies[i].x, enemies[i].y, enemies[j].x, enemies[j].y) < radius) {
                                enemies[j].current_hit_points -= damage;
                                enemies[j].hit_flash_time = timer_ticks(GAME_TIMER) + ENEMY_HIT_FLASH_TIME;
                                //enemies_hit.push_back(enemies[j].instance_id);
                            }
                        }
                    } break;
                    case p_poison: {
                        enemies[i].poison_tick_rate = poison_tick_rate;
                        enemies[i].poison_damage = damage;
                        enemies[i].poison_ticks = poison_ticks;
                    } break;
                    case p_freeze: {
                        enemies[i].current_hit_points -= damage;
                        enemies[i].freeze_time = timer_ticks(GAME_TIMER) + freeze_time;
                    } break;
                }
                enemies[i].hit_flash_time = timer_ticks(GAME_TIMER) + ENEMY_HIT_FLASH_TIME;
                enemies_hit.push_back(enemies[i].instance_id);
                pierce--;
            }
        }
        
    }

    bool have_hit_enemy(enemy &target) {
        bool output = false;
        for (int i = 0; i < enemies_hit.size(); i++) {
            if (enemies_hit[i] == target.instance_id) output = true;
        }
        return output;
    }
};

struct tower {
    //instance variables
    int x;
    int y;
    int instance_id = rand();
    double direction;
    int last_fire_time;
    int upgrade_level;
    int value;
    bool hovered;
    int index;
    enemy* target;
    targeting_type target_type;
    vector<tower_projectile> projectiles;
    tower_type type;
    int wave;

    //type defined variables
    vector<int> damage;
    vector<int> fire_rate;
    vector<int> range;
    vector<int> cost;
    vector<int> pierce;
    color barrel_color;
    color projectile_color;
    int projectile_size;
    int projectile_speed;
    int max_upgrades;
    int base_size;

    //type specific variables
    vector<int> poison_ticks;
    vector<int> poison_tick_rate;
    vector<int> freeze_time;
    vector<int> explosion_radius;
    vector<explosion> explosions;
    vector<double> accuracy;
    point_2d laser_end;
    vector<int> last_hit_pierce; //checking whether an explosion should happen

    #pragma region tower instance functions

    #pragma region tower loop functions
    tower() {
        x = 0;
        y = 0;
        direction = 0;
        last_fire_time = 0;
        target = nullptr;
        hovered = false;
        upgrade_level = 0;
    }

    void calculate_value() {
        value = 0;
        for (int i = 0; i <= upgrade_level; i++) {
            value += cost[i];
        }
    }

    bool tower_hovered() {
        return point_in_circle(point_at(mouse_x(),mouse_y()),circle_at(x,y,base_size));
    }

    void tower_find_target(vector<enemy> &enemies, int range) {
        //create a vector of all enemies in towers range
        enemy* new_target = nullptr;
        circle enemy_circle;
        for (int i = 0; i < enemies.size(); i++) {
            enemy_circle = circle_at(enemies[i].x, enemies[i].y, enemies[i].size);
            if (distance_between_points(x, y, enemies[i].x, enemies[i].y) < range || circles_intersect(enemy_circle, circle_at(x,y,range))) {
                if (new_target == nullptr) new_target = &enemies[i];
                switch (target_type) {
                    case first: {
                        if (enemies[i].dist_from_end < new_target->dist_from_end) new_target = &enemies[i];
                    } break;
                    case last: {
                        if (enemies[i].dist_from_end > new_target->dist_from_end) new_target = &enemies[i];
                    } break;
                    case strong: {
                        if (enemies[i].current_hit_points > new_target->current_hit_points) new_target = &enemies[i];
                    } break;
                    case weak: {
                        if (enemies[i].current_hit_points < new_target->current_hit_points) new_target = &enemies[i];
                    } break;
                }
            }
        }
        target = new_target;
    }

    void type_init(tower_type type) {
        switch (type) {
            case normal: simple_tower(); break;
            case machine_gun: machine_gun_tower(); break;
            case sniper: sniper_tower(); break;
            case super: super_tower(); break;
            case triple_shot: triple_shot_tower(); break;
            case tower_type::poison: poison_tower(); break;
            case tower_type::freeze: freeze_tower(); break;
            case bomb: bomb_tower(); break;
            case laser: laser_tower(); break;
            case gold_mine: gold_mine_tower(); break;
        }
    }

    void loop(vector<enemy> &enemies, int &money) {//need to have money for gold mine
        switch (type) {
            default: {
                tower_projectile_logic(enemies);
                standard_attack_loop(enemies);
            } break;
            case machine_gun: {
                tower_projectile_logic(enemies);
                machine_gun_loop(enemies);
            } break;
            case triple_shot: {
                tower_projectile_logic(enemies);
                triple_shot_loop(enemies);
            } break;
            case tower_type::poison: {
                tower_projectile_logic(enemies);
                poison_loop(enemies);
            } break;
            case tower_type::freeze: {
                tower_projectile_logic(enemies);
                freeze_loop(enemies);
            } break;
            case bomb: {
                explosive_projectile_logic(enemies);
                bomb_loop(enemies);
            } break;
            case laser: {
                laser_loop(enemies);
            } break;
            case gold_mine: {
                gold_mine_loop(money);
            } break;
        }
    }
    
    void tower_projectile_logic(vector<enemy> &enemies) {
        //do projectiles logic
        for (int i = 0; i < projectiles.size(); i++) {
            projectiles[i].control_loop(enemies);

            //if projectile is outta pierce, erase it
            if (projectiles[i].pierce > 0) continue;
            vector<tower_projectile>::iterator projectile_to_kill = projectiles.begin() + index;
            for (int j = index + 1; j < projectiles.size(); j++) {
                projectiles[j].index--;
            }
            projectiles.erase(projectile_to_kill);
            i--;
        }
    }

    void standard_attack_loop(vector<enemy> &enemies) {
        //check to fire
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;
        target = nullptr;
        tower_find_target(enemies, range[upgrade_level]);
        if (target == nullptr) return;
        // we shootin
        direction = direction_between_points(x,y, target->x, target->y);
        last_fire_time = timer_ticks(GAME_TIMER);
        tower_projectile new_projectile;
        new_projectile = create_projectile();
        projectiles.push_back(new_projectile);
    }

    void machine_gun_loop(vector<enemy> &enemies) {
        //check to fire
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;
        target = nullptr;
        tower_find_target(enemies, range[upgrade_level]);
        if (target == nullptr) return;
        // we shootin
        direction = direction_between_points(x,y, target->x, target->y);
        direction += random_range(-accuracy[upgrade_level], accuracy[upgrade_level]);
        last_fire_time = timer_ticks(GAME_TIMER);
        tower_projectile new_projectile;
        new_projectile = create_projectile();
        projectiles.push_back(new_projectile);
    }

    void triple_shot_loop(vector<enemy> &enemies) {
        //same as regular tower, just shoot 3 bullets
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;
        target = nullptr;
        tower_find_target(enemies, range[upgrade_level]);
        if (target == nullptr) return;
        // we shootin
        direction = direction_between_points(x,y, target->x, target->y);
        
        last_fire_time = timer_ticks(GAME_TIMER);
        double offset = PI/6;
        for (int i = 0; i < 3 ; i++) {
            tower_projectile new_projectile;
            new_projectile = create_projectile();
            new_projectile.direction += (i-1) * offset;
            projectiles.push_back(new_projectile);
        }
    }

    void poison_loop(vector<enemy> &enemies) {
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;
        tower_find_target(enemies, range[upgrade_level]);
        
        if (target == nullptr) return;
        // we shootin
        direction = direction_between_points(x,y, target->x, target->y);
        last_fire_time = timer_ticks(GAME_TIMER);

        tower_projectile new_projectile;
        new_projectile = create_projectile();
        new_projectile.type = p_poison;
        new_projectile.poison_ticks = poison_ticks[upgrade_level];
        new_projectile.poison_tick_rate = poison_tick_rate[upgrade_level];
        projectiles.push_back(new_projectile);
    }

    void explosive_projectile_logic(vector<enemy> &enemies) {
        for (int i = 0; i < projectiles.size(); i++) {
            //do projectiles logic
            projectiles[i].control_loop(enemies);
            if (projectiles[i].pierce < last_hit_pierce[i] && projectiles[i].death_time > timer_ticks(GAME_TIMER)) { // explode if hit anything
                explosion new_explosion;
                new_explosion.pos = projectiles[i].pos;
                new_explosion.radius = projectiles[i].radius;
                new_explosion.death_time = timer_ticks(GAME_TIMER) + EXPLOSION_TIME;
                explosions.push_back(new_explosion);
            }
            last_hit_pierce[i] = projectiles[i].pierce;
            //if projectile is outta pierce, erase it
            if (projectiles[i].pierce > 0) continue;
            vector<tower_projectile>::iterator projectile_to_kill = projectiles.begin() + index;
            for (int j = index + 1; j < projectiles.size(); j++) {
                projectiles[j].index--;
            }
            projectiles.erase(projectile_to_kill);
            last_hit_pierce.erase(last_hit_pierce.begin() + i);
            i--;
        }
    }

    void bomb_loop(vector<enemy> &enemies) {
        //bomb tower will apply an explosive effect to hit enemies
        //explode enemies that aren't exploded yet
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;
        tower_find_target(enemies, range[upgrade_level]);
        if (target == nullptr) return;
        // we shootin
        direction = direction_between_points(x,y, target->x, target->y);
        last_fire_time = timer_ticks(GAME_TIMER);

        tower_projectile new_projectile;
        new_projectile = create_projectile();
        new_projectile.type = p_explosive;
        new_projectile.radius = explosion_radius[upgrade_level];
        projectiles.push_back(new_projectile);
        last_hit_pierce.push_back(new_projectile.pierce);
    }

    void freeze_loop(vector<enemy> &enemies) {
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;

        tower_find_target(enemies, range[upgrade_level]);
        if (target == nullptr) return;
        // we shootin
        direction = direction_between_points(x,y, target->x, target->y);
        last_fire_time = timer_ticks(GAME_TIMER);

        tower_projectile new_projectile;
        new_projectile = create_projectile();
        new_projectile.type = p_freeze;
        new_projectile.freeze_time = freeze_time[upgrade_level];
        projectiles.push_back(new_projectile);
    }

    void laser_loop(vector<enemy> &enemies) {
        //check to fire
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;
        target = nullptr;
        tower_find_target(enemies, range[upgrade_level]);
        if (target == nullptr) return;

        // we shootin
        direction = direction_between_points(x,y, target->x, target->y);
        last_fire_time = timer_ticks(GAME_TIMER);
        //deal 1% of enemies current health as damage
        int adjusted_damage = damage[upgrade_level] + (target->current_hit_points / 100);
        target->current_hit_points -= adjusted_damage;
        laser_end = point_at(target->x, target->y);
    }

    void gold_mine_loop(int &money) {
        if (timer_ticks(GAME_TIMER) < last_fire_time + fire_rate[upgrade_level]) return;
        last_fire_time = timer_ticks(GAME_TIMER);
        money += damage[upgrade_level] * pow(MONEY_WAVE_INCREMENT, wave);
    }

    tower_projectile create_projectile() {
        tower_projectile new_projectile;
        new_projectile.pos = point_at(x + lengthdir_x(base_size, direction),y + lengthdir_y(base_size, direction));
        new_projectile.speed = projectile_speed;
        new_projectile.damage = damage[upgrade_level];
        new_projectile.direction = direction;
        new_projectile._color = projectile_color;
        new_projectile.index = projectiles.size();
        new_projectile.pierce = pierce[upgrade_level];
        new_projectile.size = projectile_size;
        new_projectile.type = p_standard;
        //calculate death time based on speed and range
        int lifetime = ((range[upgrade_level] * 2) / projectile_speed) * 20;
        new_projectile.death_time = timer_ticks(GAME_TIMER) + lifetime;
        return new_projectile;
    }

    #pragma endregion

    #pragma region draw methods
    
    void draw() {
        switch (type) {
            default: {
                draw_base();
                draw_nodes_standard();
                draw_projectiles_standard();
                draw_barrel_standard();
            } break;
            case bomb: {
                draw_base();
                draw_nodes_standard();
                draw_projectiles_standard();
                draw_barrel_standard();
                draw_explosions();
            } break;
            case laser: {
                draw_base();
                draw_nodes_standard();
                draw_barrel_standard();
                draw_laser();
            } break;
            case gold_mine: {
                draw_base();
                draw_nodes_standard();
                draw_gold_mine();
            } break;
        }
    }

    void draw_base() {
        color draw_color = TOWER_BASE_COLOR;
        if (hovered) draw_color = TOWER_HOVERED_COLOR;
        circle tower_base = circle_at(x,y,base_size);
        fill_circle(draw_color,tower_base);
        draw_circle(COLOR_BLACK,tower_base);
    }

    void draw_barrel_standard() {
        double bounce = calculate_bounce();
        //draw a quad centred on the tower x/y facing towards it's aim direction
        float side_angle_offset = PI / 2;
        float draw_length = (float)base_size * 0.9 * bounce;
        point_2d front = point_at(x + lengthdir_x(draw_length, direction),y + lengthdir_y(draw_length, direction));
        point_2d back = point_at(x + lengthdir_x(draw_length/4, direction + PI), y + lengthdir_y(draw_length/4, direction + PI));
        point_2d side1 = point_at(x + lengthdir_x(draw_length/2, direction + side_angle_offset),y + lengthdir_y(draw_length/2, direction + side_angle_offset));
        point_2d side2 = point_at(x + lengthdir_x(draw_length/2, direction - side_angle_offset),y + lengthdir_y(draw_length/2, direction - side_angle_offset));
        quad tower_quad = quad_from (front,side1,side2,back);
        fill_quad(barrel_color, tower_quad);
        draw_quad(COLOR_BLACK, tower_quad);
    }

    void draw_nodes_standard() {
        float upgrade_node_angle_increment = (2 * PI) / max_upgrades;
        int node_dist = base_size * 0.7;
        int node_radius = base_size * 0.25;
        for (int i = 0; i < max_upgrades; i++) {
            float angle = upgrade_node_angle_increment * i;
            point_2d upgrade_node = point_at(x + lengthdir_x(node_dist, angle - (PI/2)), y + lengthdir_y(node_dist, angle - (PI/2)));
            if (i < upgrade_level) fill_circle(barrel_color, circle_at(upgrade_node.x,upgrade_node.y,node_radius));
            else fill_circle(COLOR_GRAY, circle_at(upgrade_node.x,upgrade_node.y,node_radius));
            draw_circle(COLOR_BLACK, circle_at(upgrade_node.x,upgrade_node.y,node_radius));
        }
    }

    void draw_projectiles_standard() {
        for (int i = 0; i < projectiles.size(); i++) {
            fill_circle(projectiles[i]._color, circle_at(projectiles[i].pos.x, projectiles[i].pos.y, projectile_size));
            draw_circle(COLOR_BLACK, circle_at(projectiles[i].pos.x, projectiles[i].pos.y, projectile_size));
        }

    }

    void draw_explosions() {
        for (int j = 0; j < explosions.size(); j++) {
            
            fill_circle(EXPLOSION_COLOR, circle_at(explosions[j].pos.x, explosions[j].pos.y, explosions[j].radius));
            if (timer_ticks(GAME_TIMER) > explosions[j].death_time) {
                vector<explosion>::iterator explosion_to_kill = explosions.begin() + j;
                explosions.erase(explosion_to_kill);
                j--;
                continue;
            }
        }
    }
    
    void draw_gold_mine() {
        double bounce = calculate_bounce();
        //draw a quad centred on the tower x/y facing towards it's aim direction
        float side_angle_offset = PI / 2;
        float draw_length = (float)base_size * 0.75 * bounce;
        point_2d front = point_at(x + lengthdir_x(draw_length, direction),y + lengthdir_y(draw_length, direction));
        point_2d back = point_at(x + lengthdir_x(draw_length, direction + PI), y + lengthdir_y(draw_length, direction + PI));
        point_2d side1 = point_at(x + lengthdir_x(draw_length, direction + side_angle_offset),y + lengthdir_y(draw_length, direction + side_angle_offset));
        point_2d side2 = point_at(x + lengthdir_x(draw_length, direction - side_angle_offset),y + lengthdir_y(draw_length, direction - side_angle_offset));
        quad tower_quad = quad_from (front,side1,side2,back);
        fill_quad(barrel_color, tower_quad);
        draw_quad(COLOR_BLACK, tower_quad);
    }

    double calculate_bounce() {
        //bounce tower after firing
        float bounce = 1;
        float bounce_time = fire_rate[upgrade_level] * TOWER_BOUNCE_FALLOFF;
        if (last_fire_time + bounce_time > timer_ticks(GAME_TIMER)) {
            float percent = (timer_ticks(GAME_TIMER) - last_fire_time) / bounce_time; // 0 to 1
            bounce = 1 + ((1-percent) * TOWER_BOUNCE_MAGNITUDE);
        }
        return bounce;
    }

    void draw_laser() {
        //draw the laser
        if (target == nullptr) return;
        if (last_fire_time + LASER_TIME < timer_ticks(GAME_TIMER)) return;
        //create a quad for laser
        point_2d tower_origin = point_at(x + lengthdir_x(base_size, direction),y + lengthdir_y(base_size, direction));
        point_2d enemy_corner1 = point_at(laser_end.x + lengthdir_x(LASER_WIDTH, direction + (PI/2)), laser_end.y + lengthdir_y(LASER_WIDTH, direction + (PI/2)));
        point_2d enemy_corner2 = point_at(laser_end.x + lengthdir_x(LASER_WIDTH, direction - (PI/2)), laser_end.y + lengthdir_y(LASER_WIDTH, direction - (PI/2)));
        point_2d tower_corner1 = point_at(tower_origin.x + lengthdir_x(LASER_WIDTH, direction + (PI/2)), tower_origin.y + lengthdir_y(LASER_WIDTH, direction + (PI/2)));
        point_2d tower_corner2 = point_at(tower_origin.x + lengthdir_x(LASER_WIDTH, direction - (PI/2)), tower_origin.y + lengthdir_y(LASER_WIDTH, direction - (PI/2)));
        quad laser_quad = quad_from(tower_corner1, enemy_corner1, tower_corner2, enemy_corner2);
        //draw quad
        fill_quad(projectile_color, laser_quad);
        draw_line(COLOR_WHITE, tower_corner1, enemy_corner1);
        draw_line(COLOR_WHITE, tower_corner2, enemy_corner2);
        //draw circle for laser origin
        fill_circle(projectile_color, circle_at(tower_origin.x, tower_origin.y, LASER_WIDTH * 1.5));
        draw_circle(COLOR_WHITE, circle_at(tower_origin.x, tower_origin.y, LASER_WIDTH * 1.5));
    }

    #pragma endregion

    #pragma endregion

    #pragma region tower value definitions
    
    void simple_tower() {
        type = normal;
        max_upgrades = 9;
        damage = {12, 20, 33, 50, 80, 110, 185, 290, 375, 500};
        fire_rate = {700, 650, 600, 550, 500, 450, 400, 350, 300, 250};
        range = {160, 170, 180, 190, 200, 210, 220, 230, 240, 250};
        cost = {150, 250, 400, 650, 1050, 1780, 2430, 3500, 5500, 8700};
        pierce = {2, 2, 2, 3, 3, 4, 4, 5, 5, 6};
        projectile_size = 9;
        projectile_speed = 18;
        barrel_color = COLOR_BLUE;
        projectile_color = COLOR_LIGHT_BLUE;
        base_size = 24;
    };
    void machine_gun_tower() {
        type = machine_gun;
        max_upgrades = 6;
        damage = {5, 8, 14, 25, 45, 75, 120};
        fire_rate = {100, 85, 70, 55, 40, 30, 20};
        range = {150, 160, 170, 180, 190, 200, 210};
        cost = {250, 450, 620, 950, 1300, 2400, 3800};
        pierce = {1, 2, 2, 2, 3, 3, 4};
        projectile_size = 8;
        projectile_speed = 30;
        accuracy = {0.3, 0.27, 0.24, 0.21, 0.18, 0.15, 0.1};
        barrel_color = COLOR_LIME;
        projectile_color = COLOR_LIGHT_GREEN;
        base_size = 22;
    };
    void sniper_tower() {
        type = sniper;
        max_upgrades = 6;
        damage = {70, 120, 220, 400, 700, 1350, 2200};
        fire_rate = {1500, 1400, 1300, 1200, 1100, 1000, 900};
        range = {250, 260, 270, 280, 290, 300, 310};
        cost = {1850, 2200, 3150, 4600, 6500, 9300, 13600};
        pierce = {7, 8, 9, 10, 11, 12, 13};
        projectile_size = 14;
        projectile_speed = 12;
        barrel_color = COLOR_BLUE_VIOLET;
        projectile_color = COLOR_VIOLET;
        base_size = 28;
    };
    void super_tower() {
        type = super;
        max_upgrades = 4;
        damage = {80, 140, 300, 600, 1000};
        fire_rate = {200, 190, 180, 170, 160};
        range = {100, 110, 120, 130, 140};
        cost = {5000, 8000, 12500, 17300, 21000};
        pierce = {3, 3, 4, 5, 6};
        projectile_size = 8;
        projectile_speed = 25;
        barrel_color = COLOR_MAGENTA;
        projectile_color = COLOR_YELLOW;
        base_size = 32;
    };
    void triple_shot_tower() {
        type = triple_shot;
        max_upgrades = 5;
        damage = {10, 25, 50, 85, 140, 200};
        fire_rate = {350, 335, 320, 305, 290, 275};
        range = {200, 210, 220, 230, 240, 250};
        cost = {600, 940, 1320, 2130, 2900, 4300};
        pierce = {2, 2, 3, 3, 3, 4};
        projectile_size = 10;
        projectile_speed = 25;
        barrel_color = COLOR_PURPLE;
        projectile_color = COLOR_MAGENTA;
        base_size = 26;
    };
    void poison_tower() {
        type = tower_type::poison;
        max_upgrades = 5;
        damage = {3, 8, 20, 50, 80, 150};
        fire_rate = {750, 675, 600, 525, 450, 375};
        range = {170, 180, 190, 200, 210, 220};
        cost = {370, 600, 1000, 1500, 2100, 3000};
        pierce = {2, 3, 4, 5, 6, 8};
        projectile_size = 8;
        projectile_speed = 18;
        barrel_color = COLOR_GREEN;
        projectile_color = COLOR_LIME;
        base_size = 23;

        poison_ticks = {9, 12, 15, 18, 21, 24};
        poison_tick_rate = {1000, 900, 800, 700, 600, 500};
    };
    void freeze_tower() {
        type = tower_type::freeze;
        max_upgrades = 5;
        damage = {2, 4, 8, 16, 32, 64};
        fire_rate = {3000, 2800, 2600, 2400, 2200, 2000};
        range = {150, 160, 170, 180, 190, 200};
        cost = {460, 670, 1060, 1900, 2400, 3600};
        pierce = {2, 3, 4, 5, 6, 7};
        projectile_size = 8;
        projectile_speed = 18;
        barrel_color = COLOR_CYAN;
        projectile_color = COLOR_LIGHT_BLUE;
        base_size = 22;

        freeze_time = {500, 700, 900, 1100, 1300, 1500};
    };
    void bomb_tower() {
        type = bomb;
        max_upgrades = 5;
        damage = {60, 70, 90, 120, 170, 250};
        fire_rate = {1000, 900, 880, 700, 600, 500};
        range = {220, 235, 250, 265, 280, 295};
        cost = {1250, 1850, 2640, 3800, 4300, 6500};
        pierce = {1, 2, 3, 4, 5, 6};
        projectile_size = 12;
        projectile_speed = 15;
        barrel_color = COLOR_RED;
        projectile_color = COLOR_DARK_RED;
        base_size = 30;
        explosion_radius = {50, 60, 70, 80, 90, 100};
    };
    void laser_tower() {
        type = laser;
        max_upgrades = 5;
        damage = {40, 70, 120, 210, 390, 720};
        fire_rate = {500, 440, 380, 300, 280, 200};
        range = {120, 130, 140, 150, 160, 170};
        cost = {1000, 1480, 2150, 3600, 5400, 8800};
        projectile_size = 12;
        projectile_speed = 15;
        barrel_color = COLOR_ORANGE;
        projectile_color = COLOR_RED;
        base_size = 29;
    }
    void gold_mine_tower() {
        type = gold_mine;
        max_upgrades = 7;
        damage = {7, 15, 25, 40, 60, 85, 120, 160};
        fire_rate = {5000, 4750, 4500, 4250, 4000, 3750, 3500, 3250};
        for (int i = 0; i < max_upgrades; i++) range.push_back(40);
        cost = {800, 1190, 1600, 2250, 3400, 4900, 6200, 10300};
        for (int i = 0; i < max_upgrades; i++) pierce.push_back(0);
        projectile_size = 0;
        projectile_speed = 0;
        barrel_color = COLOR_YELLOW;
        projectile_color = COLOR_GOLD;
        base_size = 30;
        direction = 0;
    };
    #pragma endregion
};

tower place_a_tower(int &money, tower_type type, int _index, int wave) {
    tower new_tower = tower();
    new_tower.index = _index;
    new_tower.x = mouse_x();
    new_tower.y = mouse_y();
    new_tower.type = type;
    new_tower.type_init(type);
    new_tower.target_type = first;
    new_tower.calculate_value();
    new_tower.wave = wave;
    money -= new_tower.value;
    return new_tower;
}

void hover_towers(vector<tower> &placed_towers, tower* &selected_tower) {
    selected_tower = nullptr;
    for (int i = 0; i < placed_towers.size(); i++) {
        placed_towers[i].hovered = placed_towers[i].tower_hovered();
        if (placed_towers[i].hovered && mouse_clicked(LEFT_BUTTON)) selected_tower = &placed_towers[i];
    }
}

bool check_valid_tower_pos(vector<tower> &placed_towers, tower_type _type, path &level) {
    //loop over all path segments to check if we're trying to place tower in collision with path
    tower temp_tower ;
    temp_tower.type_init(_type);
    circle circle_check = circle_at(mouse_x(), mouse_y(), temp_tower.base_size);
    for (int i = 0; i < PATH_LENGTH; i++) {
        //check circle
        if (circles_intersect(level.nodes[i].segment.start_node, circle_check)) return false;
        //check quad
        if (circle_quad_intersect(circle_check, level.nodes[i].segment.path_quad)) return false;
    }
    //check we aren't placing on top of another tower
    for (int i = 0; i < placed_towers.size(); i++) {
        circle other_tower_circle = circle_at(placed_towers[i].x,placed_towers[i].y,placed_towers[i].base_size);
        if (circles_intersect(circle_check, other_tower_circle)) return false;
    }
    if (mouse_x() > PLAYABLE_VIEW_WIDTH) return false;
    //check we aren't placing outside of the playable area
    if (line_intersects_circle(line_from(PLAYABLE_VIEW_WIDTH,0,PLAYABLE_VIEW_WIDTH,PLAYABLE_VIEW_HEIGHT),circle_check)) return false;
    if (line_intersects_circle(line_from(0,0,0,PLAYABLE_VIEW_HEIGHT),circle_check)) return false;
    if (line_intersects_circle(line_from(0,0,PLAYABLE_VIEW_WIDTH,0),circle_check)) return false;
    if (line_intersects_circle(line_from(0,PLAYABLE_VIEW_HEIGHT,PLAYABLE_VIEW_WIDTH,PLAYABLE_VIEW_HEIGHT),circle_check)) return false;

    return true;
}


#endif
