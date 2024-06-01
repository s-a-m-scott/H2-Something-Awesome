#ifndef GUI_HPP
#define GUI_HPP 
#include "splashkit.h"
#include "constants_enums.hpp"
#include "towers.hpp"
#include "pathing.hpp"
using namespace std;

struct thumbnail_path {
    //this struct is used to draw a thumbnail of a path, for choosing a path for a game
    path _path;
    rectangle bounding_box;
    int thumbnail_x;

    void create_thumbnail_path(path path_to_thumbnail, int x) {
        _path = path_to_thumbnail;
        thumbnail_x = x;
        bounding_box = rectangle_from(thumbnail_x, PATH_THUMBNAIL_Y, PATH_THUMBNAIL_WIDTH, PATH_THUMBNAIL_HEIGHT);
        //need to transform path to be sized and translated correctly for thumbnail
        for (int i = 0; i < PATH_LENGTH; i++) {
            _path.nodes[i].x = (_path.nodes[i].x / PLAYABLE_VIEW_WIDTH) * PATH_THUMBNAIL_WIDTH;
            _path.nodes[i].x += x;
            _path.nodes[i].y = (_path.nodes[i].y / PLAYABLE_VIEW_HEIGHT) * PATH_THUMBNAIL_HEIGHT;
            _path.nodes[i].y += PATH_THUMBNAIL_Y;
        }
    }

    void draw() {
        fill_rectangle(COLOR_WHITE, bounding_box);
        draw_rectangle(COLOR_BLACK, bounding_box);
        for (int i = 1; i < PATH_LENGTH; i++) {
            if (i != PATH_LENGTH - 1) fill_circle(COLOR_BLACK, circle_at(_path.nodes[i].x, _path.nodes[i].y, 5));
            if (i > 0) draw_line(COLOR_BLACK, _path.nodes[i].x, _path.nodes[i].y, _path.nodes[i-1].x, _path.nodes[i-1].y);
            
        }
    }
};

struct gui_text {
    point_2d corner;
    string text;
    color _color;

    gui_text(int x, int y, string _text, color color = COLOR_BLACK) {
        corner = point_at(x, y);
        text = _text;
        _color = color;
    }

    void draw() {
        draw_text_aligned(text, _color, corner.x, corner.y);
    }
};

struct button {
    rectangle bounding_box;
    point_2d corner;
    button_state state;
    string display_text;
    color draw_color;
    bool can_click;

    button(int x, int y, int width, int height, string text) {
        corner = point_at(x, y);
        bounding_box = rectangle_from(corner, width, height);
        state = button_state::standard;
        display_text = text;
        can_click = true;
    }

    void check_state() {
        state = button_state::standard;
        if (point_in_rectangle(point_at(mouse_x(),mouse_y()),bounding_box)) {
            if (mouse_clicked(LEFT_BUTTON)) state = button_state::clicked;
            else state = button_state::hovered;
        }
        if (!can_click) state = button_state::disabled;
    }

    void draw() {
        switch (state) {
            case button_state::standard: draw_color = ICON_COLOR; break;
            case button_state::hovered: draw_color = ICON_HOVERED_COLOR; break;
            case button_state::clicked: draw_color = ICON_CLICKED_COLOR; break;
            case button_state::disabled: draw_color = ICON_DISABLED_COLOR; break;
        }
        fill_rectangle(draw_color, bounding_box);
        draw_rectangle(COLOR_BLACK, bounding_box);
        draw_text_aligned(display_text, COLOR_BLACK, corner.x + (bounding_box.width/2), corner.y + (bounding_box.height/2));
    }
};

struct menu {
    vector<button> buttons;
    vector<gui_text> texts;
    vector<string> button_names;

    void draw() {
        if (quit_requested()) return;
        draw_rectangle(COLOR_BLACK, rectangle_from(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)); //border
        for (int i = 0; i < buttons.size(); i++) {
            buttons[i].draw();
        }
        for (int i = 0; i < texts.size(); i++) {
            texts[i].draw();
        }
    }

    void check_all_buttons() {
        for (int i = 0; i < buttons.size(); i++) {
            buttons[i].check_state();
        }
    }
};

menu create_tower_selected_menu(tower* selected_tower) {
    menu output;
    
    for (int i = 0; i < TARGETING_OPTIONS; i++) {
        button new_button = button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, TARGETING_ICON_Y_START + (TARGETING_ICON_Y_OFFSET * i), ICON_WIDTH, TARGETING_BUTTON_HEIGHT, targeting_to_string(i));
        output.buttons.push_back(new_button);
        output.button_names.push_back(targeting_to_string(i));
    }
    selected_tower->calculate_value();
    output.buttons.push_back(button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, OPTION_BUTTON_Y, ICON_WIDTH, OPTION_BUTTON_HEIGHT, "SELL: $" + to_string((int)(selected_tower->value * SELL_MULTIPLIER))));
    output.button_names.push_back("SELL");
    output.button_names.push_back("UPGRADE");
    string upgrade_text = "UPGRADE: $" + to_string(selected_tower->cost[selected_tower->upgrade_level + 1]);
    if (selected_tower->upgrade_level == selected_tower->max_upgrades) upgrade_text = "MAX UPGRADE";
    output.buttons.push_back(button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, OPTION_BUTTON_Y + OPTION_BUTTON_Y_OFFSET, ICON_WIDTH, OPTION_BUTTON_HEIGHT, upgrade_text));
    //send wave button
    output.button_names.push_back("SEND WAVE");
    output.buttons.push_back(button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, PLAYABLE_VIEW_HEIGHT - (ICON_WIDTH_OFFSET + ICON_HEIGHT), ICON_WIDTH, ICON_HEIGHT, "SEND WAVE"));
    output.button_names.push_back("QUIT");
    output.buttons.push_back(button(0, SCREEN_HEIGHT - GAME_PLAY_QUIT_BUTTON_SIZE, GAME_PLAY_QUIT_BUTTON_SIZE, GAME_PLAY_QUIT_BUTTON_SIZE, "<"));
    return output;
}

menu create_free_menu() {
    menu output;
    //tower selection menu
    tower temp_tower;
    for (int i = 0; i < TOWER_TYPES; i++) {
        temp_tower.type_init((tower_type)i);
        string button_text = tower_type_to_string(i) + " $" + to_string(temp_tower.cost[0]);
        output.button_names.push_back(tower_type_to_string(i));
        output.buttons.push_back(button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, TOWER_ICON_START_Y + (TOWER_ICON_Y_OFFSET * i), ICON_WIDTH, ICON_HEIGHT, button_text));
    }
    //send wave button
    output.button_names.push_back("SEND WAVE");
    output.buttons.push_back(button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, PLAYABLE_VIEW_HEIGHT - (ICON_WIDTH_OFFSET + ICON_HEIGHT), ICON_WIDTH, ICON_HEIGHT, "SEND WAVE"));
    output.button_names.push_back("QUIT");
    output.buttons.push_back(button(0, SCREEN_HEIGHT - GAME_PLAY_QUIT_BUTTON_SIZE, GAME_PLAY_QUIT_BUTTON_SIZE, GAME_PLAY_QUIT_BUTTON_SIZE, "<"));
    return output;
}

menu create_placing_menu() {
    menu output;
    tower temp_tower;
    for (int i = 0; i < TOWER_TYPES; i++) {
        temp_tower.type_init((tower_type)i);
        string button_text = tower_type_to_string(i) + " $" + to_string(temp_tower.cost[0]);
        button new_button = button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, TOWER_ICON_START_Y + (TOWER_ICON_Y_OFFSET * i), ICON_WIDTH, ICON_HEIGHT, button_text);
        new_button.can_click = false;
        output.buttons.push_back(new_button);
    }
    return output;
    //send wave button
    output.button_names.push_back("SEND WAVE");
    output.buttons.push_back(button(PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, PLAYABLE_VIEW_HEIGHT - (ICON_WIDTH_OFFSET + ICON_HEIGHT), ICON_WIDTH, ICON_HEIGHT, "SEND WAVE"));
}

menu create_main_menu() {
    menu output;
    output.buttons.push_back(button(SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, (TITLE_Y) + BUTTON_Y_OFFSET, BUTTON_WIDTH, BUTTON_HEIGHT, "START"));
    output.button_names.push_back("START");
    output.buttons.push_back(button(SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, (TITLE_Y) + (BUTTON_Y_OFFSET*2), BUTTON_WIDTH, BUTTON_HEIGHT, "QUIT"));
    output.button_names.push_back("QUIT");
    output.texts.push_back(gui_text(SCREEN_WIDTH / 2, TITLE_Y, TITLE));
    return output;
}

menu create_map_select_menu() {
    menu output;
    int half_width = SCREEN_WIDTH / 2;
    output.texts.push_back(gui_text(half_width, TITLE_Y - (BUTTON_HEIGHT/2), "CHOOSE A MAP"));

    string button_text;
    int button_y = (SCREEN_HEIGHT / 2) + BUTTON_Y_OFFSET;
    for (int i = 0; i < 3; i++) {
        button_text = difficulty_to_string((difficulty)i) + ": " + to_string(TOTAL_WAVES[i]) + " WAVES";
        output.buttons.push_back(button(half_width + (BUTTON_WIDTH * ((1.5*i) - 2)), button_y, BUTTON_WIDTH, BUTTON_HEIGHT, button_text));

        output.button_names.push_back(difficulty_to_string((difficulty)i));
    }
    button_y += BUTTON_Y_OFFSET;
    //buttons for generating new maps, and quitting
    int smaller_button_width = BUTTON_WIDTH * 0.8;
    int smaller_button_height = BUTTON_HEIGHT * 0.5;
    output.buttons.push_back(button(half_width - (smaller_button_width/2), button_y, smaller_button_width, smaller_button_height, "GENERATE NEW MAP"));
    output.button_names.push_back("REGENERATE");
    output.buttons.push_back(button(half_width - (smaller_button_width/2), button_y + (smaller_button_height*1.2), smaller_button_width, smaller_button_height, "QUIT"));
    output.button_names.push_back("QUIT");

    return output;

}

menu create_game_over_menu() {
    menu output;
    output.buttons.push_back(button(SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, (TITLE_Y) + (BUTTON_Y_OFFSET * 2), BUTTON_WIDTH, BUTTON_HEIGHT, "RETURN TO MENU"));
    output.button_names.push_back("RETURN");

    output.texts.push_back(gui_text(SCREEN_WIDTH / 2, TITLE_Y + (BUTTON_Y_OFFSET), "GAME OVER", COLOR_RED));
    return output;
}

menu create_win_menu() {
    menu output;
    output.buttons.push_back(button(SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, (TITLE_Y) + BUTTON_Y_OFFSET, BUTTON_WIDTH, BUTTON_HEIGHT, "INFINITE MODE"));
    output.button_names.push_back("INFINITE MODE");
    output.buttons.push_back(button(SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, (TITLE_Y) + (BUTTON_Y_OFFSET*2), BUTTON_WIDTH, BUTTON_HEIGHT, "RETURN TO MENU"));
    output.button_names.push_back("RETURN");

    output.texts.push_back(gui_text(SCREEN_WIDTH / 2, TITLE_Y, "YOU WIN!"));
    return output;
}

void draw_basic_gui(int lives, int money, int current_wave, difficulty game_difficulty, game_state state) {
    //draw sidebar
    fill_rectangle(GUI_BG_COLOR, rectangle_from(PLAYABLE_VIEW_WIDTH, 0, SCREEN_WIDTH - PLAYABLE_VIEW_WIDTH, SCREEN_HEIGHT));
    draw_rectangle(COLOR_BLACK, rectangle_from(PLAYABLE_VIEW_WIDTH, 0, SCREEN_WIDTH - PLAYABLE_VIEW_WIDTH, SCREEN_HEIGHT));
    draw_text("LIVES: " + to_string(lives), COLOR_BLACK, PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, GUI_TEXT_OFFSET_TOP);
    draw_text("MONEY: $" + to_string(money), COLOR_BLACK, PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, GUI_TEXT_OFFSET_TOP*2);
    string wave_text = "WAVE: " + to_string(current_wave + 1) + "/";
    if (state == game_state::infinite) wave_text += "???";
    else wave_text += to_string(TOTAL_WAVES[(int)game_difficulty]);
    draw_text(wave_text, COLOR_BLACK, PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, GUI_TEXT_OFFSET_TOP*3);
    draw_text("DIFFICULTY: " + difficulty_to_string(game_difficulty), COLOR_BLACK, PLAYABLE_VIEW_WIDTH + ICON_WIDTH_OFFSET, GUI_TEXT_OFFSET_TOP*4);
}

int find_button_index(vector<string> button_names, string name) {
    for (int i = 0; i < button_names.size(); i++) {
        if (button_names[i] == name) return i;
    }
    return -1;
}

#endif