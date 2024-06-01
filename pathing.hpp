#ifndef PATHS_HPP
#define PATHS_HPP

#include "splashkit.h"
#include "constants_enums.hpp"
#include "utility.hpp"

#include <algorithm>
using namespace std;



struct path_segment {
    circle start_node;
    quad path_quad;
    int length;

    void draw_path_segment() {

        fill_quad(PATH_COLOR,path_quad);
        draw_line(COLOR_BLACK,path_quad.points[0], path_quad.points[2]);
        draw_line(COLOR_BLACK,path_quad.points[1], path_quad.points[3]);
        fill_circle(PATH_COLOR, start_node);
        draw_circle(COLOR_BLACK, start_node);
    }
};

struct path_node {
    float x;
    float y;
    path_segment segment;
    path_node() {
        int x_center = (PLAYABLE_VIEW_WIDTH / PATH_CELLS_HORIZONTAL)/2;
        int y_center = (PLAYABLE_VIEW_HEIGHT / PATH_CELLS_VERTICAL)/2;
        int x_rand_max = (x_center * PATH_NODE_SPREAD_FACTOR);
        int y_rand_max = (y_center * PATH_NODE_SPREAD_FACTOR);
        x = x_center + rnd(-x_rand_max,x_rand_max);
        y = y_center + rnd(-y_rand_max,y_rand_max);
    }
};


struct path {
    path_node nodes[PATH_LENGTH];
    int length;

    path() {
        int index = 1;
        for (int i = 0; i < PATH_CELLS_VERTICAL; i++) {
            for (int j = 0; j < PATH_CELLS_HORIZONTAL; j++) {
                nodes[index] = path_node();
                nodes[index].x += j * (PLAYABLE_VIEW_WIDTH / PATH_CELLS_HORIZONTAL);
                nodes[index].y += i * (PLAYABLE_VIEW_HEIGHT / PATH_CELLS_VERTICAL);
                index++;
            }
        }
        //shuffle array - not affecting first two and last two elements
        int offset = 2;
        for (int i = offset + 1; i < PATH_LENGTH-offset; i++) {
            swap(nodes[i], nodes[rnd(offset,i)]);
        }
        nodes[0].x = 0;
        nodes[0].y = 0;
        nodes[PATH_LENGTH-1].x = PLAYABLE_VIEW_WIDTH;
        nodes[PATH_LENGTH-1].y = PLAYABLE_VIEW_HEIGHT;
        length = 0;
        for (int i = 0; i < PATH_LENGTH-1; i++) {
            set_segment(nodes[i], nodes[i+1]);
            length += distance_between_points(nodes[i].x,nodes[i].y,nodes[i+1].x,nodes[i+1].y);
        }
    }

    void draw_path() {
        for (int i = 0; i < PATH_LENGTH-1; i++) nodes[i].segment.draw_path_segment();
    }

    void set_segment(path_node &node1, path_node node2) {
        path_segment new_segment;

        float angle = direction_between_points(node1.x,node1.y,node2.x,node2.y);
        //corners atclose node
        point_2d corner1 = point_at(node1.x + lengthdir_x(PATH_WIDTH/2, angle - (PI/2)), node1.y + lengthdir_y(PATH_WIDTH/2, angle - (PI/2)));
        point_2d corner2 = point_at(node1.x + lengthdir_x(PATH_WIDTH/2, angle + (PI/2)), node1.y + lengthdir_y(PATH_WIDTH/2, angle + (PI/2)));
        //corners at second node
        point_2d corner3 = point_at(node2.x + lengthdir_x(PATH_WIDTH/2, angle - (PI/2)), node2.y + lengthdir_y(PATH_WIDTH/2, angle - (PI/2)));
        point_2d corner4 = point_at(node2.x + lengthdir_x(PATH_WIDTH/2, angle + (PI/2)), node2.y + lengthdir_y(PATH_WIDTH/2, angle + (PI/2)));

        

        new_segment.path_quad = quad_from(corner1, corner2, corner3, corner4);
        new_segment.start_node = circle_at(node1.x,node1.y,PATH_WIDTH/2);
        node1.segment = new_segment;
}
};


vector<path> generate_paths() {
    vector<path> paths;
    for (int i = 0; i < PATHS_GENERATED; i++) {
        path new_path;
        paths.push_back(new_path);
    }
    //sort paths by length
    sort(paths.begin(), paths.end(), [](path a, path b) {return a.length < b.length;});
    vector<path> final_paths;
    final_paths.push_back(paths[PATHS_GENERATED-1]); //easiest path (longest)
    final_paths.push_back(paths[PATHS_GENERATED/2]); //medium path
    final_paths.push_back(paths[0]); //hardest path (shortest)
    return final_paths;
}

#endif