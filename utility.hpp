#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "splashkit.h"
#include <cmath>
#include "constants_enums.hpp"


float distance_between_points(float x1, float y1, float x2, float y2) {
    if (x1 == x2 && y1 == y1) return 0;
    else return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}

float direction_between_points(float source_x, float source_y, float target_x, float target_y) {
    float output = atan((target_y-source_y)/(target_x-source_x)) + (PI * (target_x-source_x<0));
    if (isnan(output)) return 0;
    else return output;
}

float lengthdir_x(float _len, float _dir) {
    if (_len == 0) return 0;
    return (cos(_dir) * _len);
}

float lengthdir_y(float _len, float _dir) {
    if (_len == 0) return 0;
    return (sin(_dir) * _len);
    //angle in radians
}

bool circle_quad_intersect(circle _circle, quad _quad) {
    //this isn't a full implementation of this function, it basically only works for the way the path is built
    bool output = false;
    line line1 = line_from(_quad.points[0], _quad.points[2]);
    line line2 = line_from(_quad.points[1], _quad.points[3]);

    if (point_in_quad(_circle.center,_quad)) output = true;
    //line_intersects_circle gives an error if the line has length 0
    if (line_length(line1) > 0) {
        if (line_intersects_circle(line1, _circle)) output = true;
    }
    if (line_length(line2) > 0) {
        if (line_intersects_circle(line2, _circle)) output = true;
    }

    return output;
}

void draw_text_aligned(string text, color _color, int x, int y) {
    draw_text(text, _color, x - (text.length() * 4), y);
}

double random_range(double min, double max) {
    double rand_val = (double)rand() / (double)RAND_MAX;
    return min + rand_val * (max - min);
}

string targeting_to_string(int index) {
    switch (index) {
        case (int)targeting_type::first: return "FIRST";
        case (int)targeting_type::last: return "LAST";
        case (int)targeting_type::strong: return "STRONG";
        case (int)targeting_type::weak: return "WEAK";
    }
    return "";
}

string tower_type_to_string(int index) {
    switch (index) {
        case (int)tower_type::normal: return "NORMAL";
        case (int)tower_type::machine_gun: return "MACHINE GUN";
        case (int)tower_type::sniper: return "SNIPER";
        case (int)tower_type::super: return "SUPER";
        case (int)tower_type::triple_shot: return "TRIPLE SHOT";
        case (int)tower_type::poison: return "POISON";
        case (int)tower_type::freeze: return "FREEZE";
        case (int)tower_type::bomb: return "EXPLOSIVE";
        case (int)tower_type::laser: return "LASER";
        case (int)tower_type::gold_mine: return "GOLD MINE";
    }
    return "";
}

string difficulty_to_string(int index) {
    switch (index) {
        case (int)difficulty::easy: return "EASY";
        case (int)difficulty::medium: return "MEDIUM";
        case (int)difficulty::hard: return "HARD";
    }
    return "";
}

#endif