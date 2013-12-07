#include "stdafx.h"
#pragma hdrstop
using namespace std;

#include <iostream>
#include <cctype>

#include "parser.hpp"


$emit_impl(parser, Seg*) {

    //shorthand macros: all self-descriptive
#define BREAK_IF_EOF() if (in.eof()) break
#define SKIP_UNTIL(cond) while (!in.eof() && (cond)) in.get()
#define FF() \
    in >> ws; \
    SKIP_UNTIL(in.peek() == ','); \
    in >> ws; \
    BREAK_IF_EOF()
#define INPUT(param) do { \
    FF(); \
    in >> param; \
    if (!in.good()) $yield(NULL); \
} while (false)
#define CAN_CHAIN (!isalpha(in.peek()))
#define HANDLE_REL() do { \
    if (is_rel) { \
        x += x_curr; \
        y += y_curr; \
    } \
} while (false)
#define UPDATE_CURR() do { \
    x_curr = x; \
    y_curr = y; \
} while (false)
#define SET_START() do { \
    x_start = x; \
    y_start = y; \
} while (false)

    while (!in.eof()) {
        FF();
        if(!isalpha(in.peek())) $yield(NULL); //expected: command
        cmd = in.get();
        is_rel = islower(cmd);
        cmd = toupper(cmd);

        //NOTE: `switch` can't be used due to generator
        if (cmd == 'Z') {
            $yield(new LineSeg(x_curr, y_curr, x_start, y_start));

            SET_START();
        } else if (cmd == 'M') {
            INPUT(x);
            INPUT(y);
            HANDLE_REL();

            $yield(new MoveSeg(x_curr, y_curr, x, y));

            UPDATE_CURR();
            SET_START();
            FF();
            if (CAN_CHAIN) goto fallthrough; //special: chained 'M' become 'L'
        } else if (cmd == 'L') {
            ;fallthrough:;
            do {
                INPUT(x);
                INPUT(y);
                HANDLE_REL();

                $yield(new LineSeg(x_curr, y_curr, x, y));

                UPDATE_CURR();
                FF();
            } while (CAN_CHAIN);
        } else if (cmd == 'H') {
            do {
                INPUT(x);
                HANDLE_REL();
                y = y_curr;

                $yield(new LineSeg(x_curr, y_curr, x, y));

                UPDATE_CURR();
                FF();
            } while (CAN_CHAIN);
        } else if (cmd == 'V') { //parallel to 'H'
            do {
                INPUT(y);
                HANDLE_REL();
                x = x_curr;

                $yield(new LineSeg(x_curr, y_curr, x, y));

                UPDATE_CURR();
                FF();
            } while (CAN_CHAIN);
        } else if (cmd == 'A') {
            do {
                INPUT(rx);
                INPUT(ry);
                INPUT(phi);
                INPUT(flag1);
                INPUT(flag2);
                INPUT(x);
                INPUT(y);
                HANDLE_REL();

                printf("A(r: (%8.3f, %8.3f), phi: %8.3f, flag: (%d, %d), (%8.3f, %8.3f))\n",
                       rx, ry, phi, flag1, flag2, x, y);
                //TODO: yield

                UPDATE_CURR();
                FF();
            } while (CAN_CHAIN);
        } else {
            printf("UNRECOGNIZED: %c\n", cmd);
            $yield(NULL);
        }
    }

} $stop;
