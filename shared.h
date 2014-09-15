/*
 * shared.h
 *
 * This file is part of the QtSixA, the Sixaxis Joystick Manager
 * Copyright 2008-2011 Filipe Coelho <falktx@gmail.com>
 *
 * QtSixA can be redistributed and/or modified under the terms of the GNU General
 * Public License (Version 2), as published by the Free Software Foundation.
 * A copy of the license is included in the QtSixA source code, or can be found
 * online at www.gnu.org/licenses.
 *
 * QtSixA is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 */

#ifndef SHARED_H
#define SHARED_H

#define bool int
#define true 1
#define false 0

struct dev_led {
    bool enabled;
    bool anim;
    bool auto_sel;
    int number;
};

/** Represents a dualshock device settings */
struct dev_joystick {
    /** Whether the device is enabled */
    bool enabled;
    /** Whether buttons are enabled */
    bool buttons;
    /** Whether the axis position detection is enabled */
    bool axis;
    /** Whether the S buttons (?) are enables */
    bool sbuttons;
    /** Whether acceleration detection is enabled */
    bool accel;
    /** Whether accon (?) detection is enabled */
    bool accon;
    /** Whether speed detection is enabled */
    bool speed;
    /** Whether position detection is enabled */
    bool pos;
};

/** Represents settings for a PS3 remote */
struct dev_remote {
    /** Whether the remote is enabled */
    bool enabled;
    bool numeric;
    bool dvd;
    bool directional;
    bool multimedia;
};

/** Represents input device settings */
struct dev_input {
    bool enabled;
    /** The current state of the select button */
    int key_select, 
    /** The current state of the L3 button (left stick press) */
        key_l3, 
    /** The current state of the R3 button (right stick press) */
        key_r3, 
    /** The current state of the start button */
        key_start, 
    /** The current state of the up d-pad button */
        key_up, 
    /** The current state of the right d-pad button */
        key_right, 
    /** The current state of the down d-pad button */
        key_down, 
    /** The current state of the left d-pad button */
        key_left,
    /** The current state of the L2 button (left trigger) */
        key_l2, 
    /** The current state of the R2 button (right trigger) */
        key_r2, 
    /** The current state of the L1 button (left shoulder) */
        key_l1, 
    /** The current state of the R1 button (right shoulder) */
        key_r1, 
    /** The current state of the triangle button */
        key_tri, 
    /** The current state of the circle button */
        key_cir, 
    /** The current state of the square button */
        key_squ, 
    /** The current state of the cross button (X) */
        key_cro, 
    /** The current state of the PS button (HOME) */
        key_ps;
    int axis_l_type, 
        axis_r_type, 
        axis_speed,
    /** The upward position of the left stick */
        axis_l_up,
    /** The rightward position of the left stick */
        axis_l_right, 
    /** The downward position of the left stick */
        axis_l_down,
    /** The leftward position of the left stick */
        axis_l_left,
    /** The upward position of the left stick */
        axis_r_up, 
    /** The rightward position of the right stick */
        axis_r_right, 
    /** The downward position of the right stick */
        axis_r_down, 
    /** The leftward position of the right stick */
        axis_r_left;
    /** Whether or not to use the L3 and R3 controls */
    bool use_lr3;
};

/** Represents rumble settings for dualshock */
struct dev_rumble {
    bool enabled;
    bool old_mode;
};

/** Represents device timeout settings */
struct dev_timeout {
    bool enabled;
    int timeout;
};

/** Represents settings for any connected device */
struct device_settings {
    bool auto_disconnect;
    struct dev_led led;
    struct dev_joystick joystick;
    struct dev_remote remote;
    struct dev_input input;
    struct dev_rumble rumble;
    struct dev_timeout timeout;
};

bool was_active();
void set_active(int active);

bool io_canceled();
void sig_term(int sig);
void open_log(const char *app_name);

struct device_settings init_values(const char *mac);

int get_joystick_number();
void enable_sixaxis(int csk);

#endif // SHARED_H
