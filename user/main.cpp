#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <sstream>
#include <string>

#include "conf.hpp"
#include "misc.hpp"
#include "math.hpp"

#include "drive.hpp"
#include "laser.hpp"
#include "qep.hpp"

#include "parser.hpp"
#include "seg.hpp"


float x_curr = 0, y_curr = 0;
float V_feed = V_feed_max;

bool exec_path(parser& gen) {
    Seg *curr, *next;
    bool curr_solid, next_solid;
    float V_last = V_feed_start;
    float V_next = V_feed_start;

    if (!gen(curr)) return true;
    if (!curr) return false;
    curr_solid = curr->solid();
    bool rem;

    do {
        //look-ahead
        rem = gen(next);
        if (rem) {
            if (!next) return false;
            next_solid = next->solid();

            //near-tangent
            float x = wrap_angle(next->theta1 - curr->theta2) / feed_tan_cutoff;
            float v = V_feed_tan*(1 - x*x);
            if (v > V_next) {
                //DEBUG
                V_next = v;
            }
        }

        //execute path segment
        laser_set(curr_solid);
        curr->exec(V_last, V_feed, V_next);
        x_curr = curr->x2;
        y_curr = curr->y2;

        if (!rem || curr_solid != next_solid) {
            //wait for actual stop before switching laser
            while (qep_vel > qep_vel_zero_Npulse) {
                os_dly_wait(CONV(qep_vel_period_Ts, Ts, Tms));
            }
        }
        if (!rem) laser_off(); //always switch off after last segment

        //report position
        printf("    QEP: (%6u, %6u)\r\n", *qep_x, *qep_y);

        //iterate
        delete curr;
        curr = next;
        curr_solid = next_solid;
        V_last = V_next;
        V_next = V_feed_start;
    } while (rem) ;

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
            float t;
            if (cin.peek() != '\r') {
                cin >> t;
                if (cin.fail()) {
                    printf("\r\n"
                           "... usage: tone ### (range: [0, 1])\r\n");
                    cin.clear();
                    cin.ignore(100, '\n');
                    continue;
                }
                laser_set_tone(t);
            }
            printf("\r\n"
                   "### tone: %6.4f\r\n",
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
