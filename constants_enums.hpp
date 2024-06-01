#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "splashkit.h"

#pragma region general utility
const int TOWER_GUI_WIDTH = 180;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int PLAYABLE_VIEW_WIDTH = SCREEN_WIDTH - TOWER_GUI_WIDTH;
const int PLAYABLE_VIEW_HEIGHT = SCREEN_HEIGHT;

const float PI = 3.14159;
const float TEXT_OFFSET_MULTIPLIER = 2.25;

//constants
const string GAME_TIMER = "timer";
const string GAME_NAME = "Custom Project - Tower Defence Game";
const string TITLE = "TOWER DEFENCE GAME";

const int START_LIVES[3] = {100, 50, 10};
const int STARTING_MONEY[3] = {750, 500, 350};
const int TOTAL_WAVES[3] = {30, 50, 100};
const int END_WAVE_MONEY[3] = {100, 75, 50};
const int FIRST_BOSS_WAVE = 30;
const int GAME_PLAY_QUIT_BUTTON_SIZE = 25;

const float MONEY_WAVE_INCREMENT = 1.01;

const int ENEMY_SPAWN_RATE = 500; //milliseconds
const int ENEMY_SPAWN_RATE_MIN = 50;
const float SPAWN_RATE_REDUCTION = 0.97;
const color PLAY_BG_COLOR = COLOR_GREEN;


//enums
enum user_state {
    free_select, 
    tower_placing, 
    tower_selected,
};

enum game_state {
    main_menu,
    map_select,
    playing,
    won,
    infinite,
    lost,
};

enum difficulty {
    easy,
    medium,
    hard,
};
#pragma endregion

#pragma region tower
//constants

const color TOWER_BASE_COLOR = COLOR_GRAY;
const color TOWER_HOVERED_COLOR = COLOR_DARK_GRAY;
const color TOWER_SELECTED_COLOR = COLOR_LIGHT_GRAY;
const int TOWER_TYPES = 10;
const int TARGETING_OPTIONS = 4;
const int UPGRADE_TYPES = 4;
const double SELL_MULTIPLIER = 0.75;
const int EXPLOSION_TIME = 30;
const color EXPLOSION_COLOR = rgba_color(255,255,255,100);
const double TOWER_BOUNCE_FALLOFF = 0.25;
const double TOWER_BOUNCE_MAGNITUDE = 0.3;
const int TOWER_BOUNCE_TIME = 40;
const int LASER_TIME = 100;
const int LASER_WIDTH = 5;

//enums
enum tower_type {
    normal=0,
    machine_gun=1,
    poison = 2,
    freeze = 3,
    triple_shot = 4,
    gold_mine = 5,
    laser = 6,
    bomb = 7,
    sniper=8,
    super=9,
};

enum targeting_type {
    first = 0,
    last = 1,
    strong = 2,
    weak = 3,
};

enum projectile_type {
    p_standard,
    p_poison,
    p_freeze,
    p_explosive,
};

#pragma endregion

#pragma region enemy
enum enemy_type {
    basic=0,
    fast=1,
    beefy=2,
    tiny=3,
    splitting=4,
    split_child=5,
    flying=6,
    elite=7,
    boss=8,
};

const color ENEMY_HP_COLOR = COLOR_RED;
const color ENEMY_HP_BG_COLOR = COLOR_DARK_GRAY;

const int ENEMY_HP_BAR_WIDTH = 20;
const int ENEMY_HP_BAR_HEIGHT = 10;
const int ENEMY_TYPES = 9;
const int ENEMY_HIT_FLASH_TIME = 100;
const int ENEMY_SPLITTING_CHILDREN = 3;
const double ENEMY_HP_SCALE = 1.03;
#pragma endregion

#pragma region path
const color PATH_COLOR = COLOR_GRAY;

const int PATH_WIDTH = 20;
const int PATH_CELLS_HORIZONTAL = 3;
const int PATH_CELLS_VERTICAL = 3;
const int PATH_LENGTH = (PATH_CELLS_HORIZONTAL * PATH_CELLS_VERTICAL) + 2;
const float PATH_NODE_SPREAD_FACTOR = 0.6; // how strong the randomisation is on node position. 0-1
const int PATHS_GENERATED = 50;
#pragma endregion

#pragma region wave

const int INFINITE_WAVE_BOSS_INTERVAL = 10;

const int ENEMY_TYPES_WAVE = 3;
const int WAVE_CAPACITY_BASE = 50;
const int WAVE_CAPACITY_INCREASE = 10;
const int SPAWN_RATE_MULTIPLIER = 200;
//enums
enum wave_state {
    waiting,
    going,
};
#pragma endregion

#pragma region gui
//colors
const color ICON_DISABLED_COLOR = COLOR_GRAY;
const color ICON_COLOR = COLOR_DARK_GRAY; //dark gray is lighter than gray, for some reason
const color ICON_HOVERED_COLOR = COLOR_LIGHT_GRAY;
const color ICON_CLICKED_COLOR = COLOR_WHITE;
const color GUI_BG_COLOR = COLOR_SLATE_GRAY;
const color TARGETING_SELECTED_COLOR = COLOR_GRAY;

const color VALID_TOWER_POS = rgba_color(0,255,0,100);
const color INVALID_TOWER_POS = rgba_color(255,0,0,100);

//text alignments
const int GUI_TEXT_OFFSET_RIGHT = 3;
const int GUI_TEXT_OFFSET_TOP = 10;
const int ICON_WIDTH_OFFSET = 5;
const int ICON_WIDTH = TOWER_GUI_WIDTH - (ICON_WIDTH_OFFSET * 2);
const int ICON_HEIGHT = 40;
const int TOWER_ICON_START_Y = 80;
const int TOWER_ICON_Y_OFFSET = ICON_HEIGHT + 5;
const int TITLE_Y = 200;
const int BUTTON_WIDTH = 200;
const int BUTTON_HEIGHT = 70;
const int BUTTON_Y_OFFSET = 100;
const int TARGETING_ICON_Y_START = 150;
const int TARGETING_BUTTON_HEIGHT = 30;
const int TARGETING_ICON_Y_OFFSET = TARGETING_BUTTON_HEIGHT + 5;
const int TARGETING_TEXT_Y_OFFSET = 10;
const int OPTION_BUTTON_Y = 500;
const int OPTION_BUTTON_HEIGHT = 50;
const int OPTION_BUTTON_Y_OFFSET = OPTION_BUTTON_HEIGHT + 10;
const int OPTION_TEXT_Y_OFFSET = 20;
const int BUTTON_TEXT_X_OFFSET = 10;

//text strings
const string PLAY_BUTTON_TEXT = "START";
const string RESTART_BUTTON_TEXT = "RESTART";
const string INFINITE_MODE_BUTTON_TEXT = "INFINITE MODE";
const string QUIT_BUTTON_TEXT = "QUIT";

//path thumbnails
const int PATH_THUMBNAIL_WIDTH = 250;
const int PATH_THUMBNAIL_HEIGHT = 250;
const int PATH_THUMBNAIL_SEPARATION = 50;
const int PATH_THUMBNAIL_START_X = SCREEN_WIDTH/2 - ((PATH_THUMBNAIL_WIDTH * 1.5) + PATH_THUMBNAIL_SEPARATION);
const int PATH_THUMBNAIL_Y = SCREEN_HEIGHT/2 - (PATH_THUMBNAIL_HEIGHT * 0.65);
const int PATH_THUMBNAIL_POS[3] = {
    PATH_THUMBNAIL_START_X, 
    PATH_THUMBNAIL_START_X + (PATH_THUMBNAIL_WIDTH + PATH_THUMBNAIL_SEPARATION), 
    PATH_THUMBNAIL_START_X + ((PATH_THUMBNAIL_WIDTH + PATH_THUMBNAIL_SEPARATION) * 2), 
};



enum button_state {
    hovered,
    clicked,
    standard,
    disabled,
};
#pragma endregion

#endif