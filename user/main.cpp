#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <sstream>
#include <string>

#include "conf.hpp"
#include "misc.hpp"

#include "drive.hpp"
#include "laser.hpp"

#include "parser.hpp"
#include "seg.hpp"


float x_curr = 0, y_curr = 0;
float V_feed = V_feed_max;

bool exec_path(parser& p) {
    for (Seg* seg ; p(seg) ; ) {
        if (!seg) return false;
        seg->exec(V_feed_start, V_feed, V_feed_start);
        x_curr = seg->x2;
        y_curr = seg->y2;
        delete seg;
    }
    return true;
}

__task void main_task(){
    printf(
        "\r\n\r\n"
        "### proto-nc-main %s %s\r\n"
        "\r\n",
        __DATE__, __TIME__
    );
    drive_start();
    os_dly_wait(100);

    string cmd;
    while (1) {
        cin >> cmd;
        if (0) {
        } else if (cmd == "path") {
            string path;

            cin >> ws;
            if (cin.get() != '"') {
                printf("\r\n"
                       "... usage: `path \"data\"`\r\n");
                continue;
            }
            getline(cin, path, '"');
            printf("\r\n"
                   "### path: start (%8.3f, %8.3f)\r\n",
                   x_curr, y_curr);

            typedef stringstream SS;
            SS in(path, SS::in);
            parser p(in, x_curr, y_curr);
            if (exec_path(p)) {
                printf("\r\n"
                       "### path: done\r\n");
            } else {
                printf("\r\n"
                       "!!! path: parse error\r\n");
            }
        } else if (cmd == "feed") {
            float f;
            if (cin.peek() != '\r') {
                cin >> f;
                if (cin.fail()) {
                    printf("\r\n"
                           "... usage: feed ###.## (mm/s)\r\n");
                    cin.clear();
                    cin.ignore(100, '\n');
                    continue;
                }
                V_feed = min(f, V_feed_max);
            }
            printf("\r\n"
                   "### feed: %5.1f mm/s\r\n",
                   V_feed);
        } else if (cmd == "tone") {
            int t;
            if (cin.peek() != '\r') {
                cin >> t;
                if (cin.fail()) {
                    printf("\r\n"
                           "... usage: tone ### (range: [0, 100]%)\r\n");
                    cin.clear();
                    cin.ignore(100, '\n');
                    continue;
                }
                laser_set_tone(t);
            }
            printf("\r\n"
                   "### tone: %3d%% \r\n",
                   laser_tone);
        } else if (cmd == "origin") {
            x_curr = y_curr = 0;
            printf("\r\n"
                   "### origin: done\r\n");
        } else {
            printf("\r\n"
                   "???\r\n");
        }
    }
}

int main(){
    os_sys_init_prio(main_task, 0x80);
}
