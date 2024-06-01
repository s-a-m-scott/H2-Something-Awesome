#include "splashkit.h"
using namespace std;

#include "utility.hpp"
#include "constants_enums.hpp"
#include "towers.hpp"
#include "enemies.hpp"
#include "pathing.hpp"
#include "gui.hpp"
#include "waves.hpp"

struct game_data {
    
    int lives;
    int current_wave;
    int money;
    difficulty _difficulty;

    menu _menu;
    game_state state;
    user_state u_state;
    color bg_color;

    path level;
    vector<path> levels_available;
    vector<thumbnail_path> level_thumbnails;

    vector<enemy> enemies;
    int enemies_spawned;

    wave_state _wave_state;
    wave current_wave_data;
    int next_enemy_spawn;

    int tower_icon_selected;
    bool valid_pos;
    vector<tower> placed_towers;
    tower_type type_to_place;

    tower* selected_tower;
    int costs[TOWER_TYPES];

    #pragma region general game functions

    game_data() {
        //general
        reset_game();
        tower temp_tower;
        for (int i = 0; i < TOWER_TYPES; i++) {
            temp_tower.type_init((tower_type)i);
            costs[i] = temp_tower.cost[0];
        }
        
    }

    void reset_game() {
        _menu = create_main_menu();
        bg_color = PLAY_BG_COLOR;
        level = path();
        valid_pos = false;
        tower_icon_selected = -1;
        next_enemy_spawn = 0;
        current_wave = 0;
        u_state = free_select;
        _wave_state = waiting;
        placed_towers.resize(0);
        enemies.resize(0);
        state = main_menu;
    }
    
    void draw_gameplay() {
        if (quit_requested()) return;
        //draw standard stuff for gameplay
        level.draw_path();
        
        for (int i = 0; i < placed_towers.size(); i++) placed_towers[i].draw();

        for (int i = 0; i < enemies.size(); i++) enemies[i].draw();

        //draw tower bounding box if trying to place
        switch (u_state) {
            case free_select: break;
            case tower_placing: {
                color tower_hitbox_color;
                if (valid_pos) tower_hitbox_color = VALID_TOWER_POS;
                else tower_hitbox_color = INVALID_TOWER_POS;
                

                tower temp_tower;
                temp_tower.type_init(type_to_place);
                fill_circle(tower_hitbox_color, circle_at(mouse_x(), mouse_y(), temp_tower.base_size));
                draw_circle(COLOR_BLACK, circle_at(mouse_x(), mouse_y(), temp_tower.range[0]));
            } break;
            case tower_selected: {
                if (selected_tower == nullptr) return;
                draw_circle(COLOR_BLACK, circle_at(selected_tower->x, selected_tower->y, selected_tower->range[selected_tower->upgrade_level]));
            }
            break;
        }
        draw_basic_gui(lives, money, current_wave, _difficulty, state);
    }

    void game_playing() {
        //lose the game if run out of lives
        if (lives <= 0) {
            state = lost;
            _menu = create_game_over_menu();
            bg_color = COLOR_BLACK;
            return;
        }
        
        //manage user state
        switch (u_state) {
            case free_select: user_free_state();
            break;
            case tower_placing: user_placing_tower();
            break;
            case tower_selected: user_tower_selected();
            break;
        }

        //manage wave state
        switch (_wave_state) {
            case waiting: wave_waiting();
            break;
            case going: wave_going();
            break;
        }
    }

    void gen_paths_and_thumbnails() {
        levels_available.clear();
        level_thumbnails.clear();
        levels_available = generate_paths();
        for (int i = 0; i < 3; i++) {
            thumbnail_path temp = thumbnail_path();
            temp.create_thumbnail_path(levels_available[i], PATH_THUMBNAIL_POS[i]);
            level_thumbnails.push_back(temp);
        }
    }

    #pragma endregion

    #pragma region menu functions

    void draw_path_thumbnails() {
        for (int i = 0; i < 3; i++) {
            level_thumbnails[i].draw();
        }
    }

    void main_menu_logic() {
        int start = find_button_index(_menu.button_names, "START");
        int quit = find_button_index(_menu.button_names, "QUIT");
        if (_menu.buttons[start].state == clicked) {
            state = map_select;
            gen_paths_and_thumbnails();
            _menu = create_map_select_menu();
        }
        if (_menu.buttons[quit].state == clicked) close_window(GAME_NAME);
    }

    void map_select_logic() {
        int quit = find_button_index(_menu.button_names, "QUIT");
        if (_menu.buttons[quit].state == clicked) {
            reset_game();
            return;
        }
        int regen = find_button_index(_menu.button_names, "REGENERATE");
        if (_menu.buttons[regen].state == clicked) {
            gen_paths_and_thumbnails();
            return;
        }
        for (int i = 0; i < 3; i++) {
            if (_menu.buttons[i].state == clicked) {
                level = levels_available[i];
                lives = START_LIVES[i];
                money = STARTING_MONEY[i];
                _difficulty = (difficulty)i;
                state = playing;
                _menu = create_free_menu();
                return;
            }
        }
    }

    void win_menu_logic() {
        int _return = find_button_index(_menu.button_names, "RETURN");
        int infinite = find_button_index(_menu.button_names, "INFINITE MODE");
        if (_menu.buttons[_return].state == clicked) {
            reset_game();
            return;
        }
        if (_menu.buttons[infinite].state == clicked) {
            state = game_state::infinite;
            _menu = create_free_menu();
        }
        
    }

    void game_over_menu_logic() {
        int _return = find_button_index(_menu.button_names, "RETURN");
        if (_menu.buttons[_return].state == clicked) reset_game();
    }

    #pragma endregion

    #pragma region user state functions
    
    void user_free_state() {
        int quit = find_button_index(_menu.button_names, "QUIT");
        if (_menu.buttons[quit].state == clicked) {
            reset_game();
            return;
        }
        hover_towers(placed_towers, selected_tower);
    
        tower_icon_selected = -1;
        for (int i = 0; i < TOWER_TYPES; i++) {
            if (money >= costs[i]) _menu.buttons[i].can_click = true;
            else _menu.buttons[i].can_click = false;

            if (_menu.buttons[i].state == clicked && _menu.buttons[i].can_click == true) {
                tower_icon_selected = i;
                u_state = tower_placing;
                type_to_place = (tower_type)i;
                _menu = create_placing_menu();
                return;
            }
        }

        if (selected_tower != nullptr) {
            u_state = tower_selected;
            _menu = create_tower_selected_menu(selected_tower);
            return;
        }
    }

    void user_placing_tower() { 
        valid_pos = check_valid_tower_pos(placed_towers, type_to_place,level);
        if (mouse_clicked(LEFT_BUTTON)) {
            if (valid_pos) {
                placed_towers.push_back(place_a_tower(money, type_to_place, placed_towers.size(), current_wave));
                selected_tower = &placed_towers.back();
                _menu = create_tower_selected_menu(&placed_towers.back());
                u_state = tower_selected;
            }
        }
        if (!key_down(ESCAPE_KEY)) return;
        u_state = free_select;
        _menu = create_free_menu();
    }

    void user_tower_selected() {
        int quit = find_button_index(_menu.button_names, "QUIT");
        if (_menu.buttons[quit].state == clicked) {
            reset_game();
            return;
        }
        _menu.buttons[selected_tower->target_type].state = disabled;
        if (selected_tower->upgrade_level == selected_tower->max_upgrades) _menu.buttons[find_button_index(_menu.button_names, "UPGRADE")].state = disabled;
        if (selected_tower->cost[selected_tower->upgrade_level + 1] > money) _menu.buttons[find_button_index(_menu.button_names, "UPGRADE")].state = disabled;
        
        for (int i = 0; i < _menu.buttons.size(); i++) {
            if (_menu.buttons[i].state != clicked) continue;
            if (i == find_button_index(_menu.button_names, targeting_to_string((targeting_type)i))) {
                selected_tower->target_type = (targeting_type)i;
                return;
            }
            if (i == find_button_index(_menu.button_names, "SELL")) {
                money += selected_tower->value * SELL_MULTIPLIER;
                vector<tower>::iterator tower_to_sell = placed_towers.begin() + selected_tower->index;
                for (int i = selected_tower->index + 1; i < placed_towers.size(); i++) {
                    placed_towers[i].index--;
                }
                placed_towers.erase(tower_to_sell);
                selected_tower = nullptr;
                u_state = free_select;
                _menu = create_free_menu();
                return;
            }
            if (i == find_button_index(_menu.button_names, "UPGRADE")) {
                money -= selected_tower->cost[selected_tower->upgrade_level + 1];
                selected_tower->upgrade_level++;
                //recalculate menu for new upgrade path
                _menu = create_tower_selected_menu(selected_tower);
                return;
            }
        }
        
        if (mouse_x() > PLAYABLE_VIEW_WIDTH) return;
        //exit selected tower state
        tower* this_tower = selected_tower;
        selected_tower->hovered = true;
        if (!mouse_clicked(LEFT_BUTTON)) return;
        hover_towers(placed_towers, selected_tower);
        //clicked on this tower again
        if (selected_tower == this_tower) return;
        if (selected_tower != nullptr) {
            //need to rebuild menu with newly selected tower's info
            _menu = create_tower_selected_menu(selected_tower);
            return;
        }
        
        //no tower selected
        u_state = free_select;
        _menu = create_free_menu();
        
    }
    #pragma endregion

    #pragma region wave state functions
    void wave_waiting() {    
        int index = find_button_index(_menu.button_names, "SEND WAVE");
        if (_menu.buttons[index].state != clicked && !key_down(SPACE_KEY)) return;
        _menu.buttons[index].state = clicked;
        _wave_state = going;
        current_wave_data.generate_wave(current_wave);
        next_enemy_spawn = timer_ticks(GAME_TIMER);
        enemies.clear();
        enemies_spawned = 0;
    }

    void wave_going() {
        int index = find_button_index(_menu.button_names, "SEND WAVE");
        _menu.buttons[index].state = disabled; //have to do this here, due to possibility of changing menus during wave
        //spawn enemies
        if (next_enemy_spawn < timer_ticks(GAME_TIMER) && enemies_spawned < current_wave_data.count) {
            enemy new_enemy = enemy();
            new_enemy.spawn_enemy(current_wave_data.enemy_types[enemies_spawned], enemies, current_wave, _difficulty);
            int spawn_rate = (ENEMY_SPAWN_RATE * pow(SPAWN_RATE_REDUCTION,current_wave));
            next_enemy_spawn = timer_ticks(GAME_TIMER) + max(new_enemy.spawn_value * pow(SPAWN_RATE_REDUCTION,current_wave) * SPAWN_RATE_MULTIPLIER, (double)ENEMY_SPAWN_RATE_MIN);
            enemies_spawned++;        
        }

        //move to next wave
        if (current_wave_data.count - enemies_spawned <= 0 && enemies.size() == 0) {
            if (current_wave == TOTAL_WAVES[(int)_difficulty]-1 && lives > 0 && state == playing) {
                state = won;
                _menu = create_win_menu();
            }
            else {
                _wave_state = waiting;
                //clear projectiles
                for (int i = 0; i < placed_towers.size(); i++) placed_towers[i].projectiles.clear();
                current_wave++;
                money += END_WAVE_MONEY[(int)_difficulty]; //makes first waves less boring, because you cant afford anything otherwise
            }
        }

        //tower and enemy behaviours
        for (int i = 0; i < placed_towers.size(); i++) placed_towers[i].loop(enemies, money);
        
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].loop(level, lives, money, current_wave, enemies);
            if (!enemies[i].alive) {
                enemies[i].kill_enemy(enemies);

                i--;
            }
        }
    }

    #pragma endregion

};


int main() {
    game_data game = game_data();
    game._menu = create_main_menu();

    create_timer(GAME_TIMER);
    start_timer(GAME_TIMER);

    open_window(GAME_NAME, SCREEN_WIDTH, SCREEN_HEIGHT);

    while (!quit_requested())
    {
        clear_screen(game.bg_color);
        game._menu.check_all_buttons();

        switch (game.state) {
            case main_menu: {
                game.main_menu_logic();
            } break;
            case map_select: {
                game.map_select_logic();
                game.draw_path_thumbnails();
            } break;
            case playing: {
                game.game_playing();
                game.draw_gameplay();
            } break;
            case lost: {
                game.game_over_menu_logic();
            } break;
            case won: {
                game.win_menu_logic();
            } break;
            case infinite: {
                game.game_playing();
                game.draw_gameplay();
            } break;
        }
        game._menu.draw();
        process_events();
        refresh_screen(60);
        
    }
}