/*
 *  sixad driver (based on BlueZ)
 * ------------------------------
 *  Used several code from BlueZ, and some other portions from many projects around the web
 *
 *  Written by falkTX, 2009
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <malloc.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/hidp.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include <linux/input.h>
#include <linux/joystick.h>
#include <linux/uinput.h>

#include "textfile.h"
#include "sdp.h"

#define _GNU_SOURCE
#define L2CAP_PSM_HIDP_CTRL 0x11
#define L2CAP_PSM_HIDP_INTR 0x13

int n_six, enable_leds, led_js_n, led_n, enable_ledplus, enable_led_anim, use_legacy, debug;

static volatile sig_atomic_t __io_canceled = 0;

static void sig_hup(int sig)
{
}

static void sig_term(int sig)
{
    __io_canceled = 1;
}

static inline int ppoll(struct pollfd *fds, nfds_t nfds,
                        const struct timespec *timeout, const sigset_t *sigmask)
{
    if (timeout == NULL)
        return poll(fds, nfds, -1);
    else if (timeout->tv_sec == 0)
        return poll(fds, nfds, 500);
    else
        return poll(fds, nfds, timeout->tv_sec * 1000);
}


//Legacy Compatibility LEDs
static void enable_sixaxis(int csk, int led_n, int enable_led_anim)
{
    char buf[1024];
    unsigned char enable[] = {
        0x53, /* HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_FEATURE */
        0xf4, 0x42, 0x03, 0x00, 0x00
    };
    unsigned char setleds[] = {
        0x52, /* HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_OUTPUT */
        0x01,
        0x00, 0x00, 0x00, 0x00, 0x00,	// rumble values
        0x00, 0x00, 0x00, 0x00, 0x1E,	// 0x10=LED1 .. 0x02=LED4
        0xff, 0x27, 0x10, 0x00, 0x32,	// LED 4
        0xff, 0x27, 0x10, 0x00, 0x32,	// LED 3
        0xff, 0x27, 0x10, 0x00, 0x32,	// LED 2
        0xff, 0x27, 0x10, 0x00, 0x32,	// LED 1
        0x00, 0x00, 0x00, 0x00, 0x00
    };
    const unsigned char ledpattern[8] = {  // last one (0x20) is "all-off", none
        0x02, 0x04, 0x08, 0x10,
        0x12, 0x14, 0x18, 0x20
    };

    if (enable_leds) {
	if (led_n < 1) {
	    n_six = 0;
	} else if (n_six > 7) {
	    n_six = 6;
	} else {
	    n_six = led_n - 1;
	}
    } else
      n_six = 7;

    /* enable reporting */
    send(csk, enable, sizeof(enable), 0);
    recv(csk, buf, sizeof(buf), 0);

    if (enable_led_anim && enable_leds)
    {
        /* Sixaxis LED animation - Way Cool!! */
        int animation;
        animation = 0;
        while ( animation < 4 ) {  // repeat it 4 times
            setleds[11] = ledpattern[0];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(10000);
            setleds[11] = ledpattern[1];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(5000);
            setleds[11] = ledpattern[2];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(5000);
            setleds[11] = ledpattern[3];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(10000);
            setleds[11] = ledpattern[2];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(5000);
            setleds[11] = ledpattern[1];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(5000);
            animation = animation + 1;
        }
        /* 2nd part of animation (animate until LED reaches selected number) */
        if (n_six == 1)
        {
            setleds[11] = ledpattern[0];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
        }
        else if (n_six == 2)
        {
            setleds[11] = ledpattern[0];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(10000);
            setleds[11] = ledpattern[1];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
        }
        else if (n_six == 3)
        {
            setleds[11] = ledpattern[0];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(100000);
            setleds[11] = ledpattern[1];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(50000);
            setleds[11] = ledpattern[2];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
        }
        else if (n_six == 5)
        {
            setleds[11] = ledpattern[0];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
        }
        else if (n_six == 6)
        {
            setleds[11] = ledpattern[0];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
            usleep(100000);
            setleds[11] = ledpattern[1];
            send(csk, setleds, sizeof(setleds), 0);
            recv(csk, buf, sizeof(buf), 0);
        }
    }

    /* set LEDs */
    setleds[11] = ledpattern[n_six];
    send(csk, setleds, sizeof(setleds), 0);
    recv(csk, buf, sizeof(buf), 0);
}

static int get_type(int snsk)
{
    struct hidp_connadd_req req;
    struct sockaddr_l2 addr;
    socklen_t addrlen;
    bdaddr_t src, dst;

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if (getsockname(snsk, (struct sockaddr *) &addr, &addrlen) < 0)
        return -1;

    bacpy(&src, &addr.l2_bdaddr);

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if (getpeername(snsk, (struct sockaddr *) &addr, &addrlen) < 0)
        return -1;

    bacpy(&dst, &addr.l2_bdaddr);

    get_sdp_device_info(&src, &dst, &req);

    if (req.vendor == 0x054c && req.product == 0x0268) {
        return 1;
    }
    else {
        return 0;
    }
}

static int l2cap_listen(const bdaddr_t *bdaddr, unsigned short psm, int lm, int backlog)
{
    struct sockaddr_l2 addr;
    struct l2cap_options opts;
    int sk;

    if ((sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, bdaddr);
    addr.l2_psm = htobs(psm);

    if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(sk);
        return -1;
    }

    setsockopt(sk, SOL_L2CAP, L2CAP_LM, &lm, sizeof(lm));

    memset(&opts, 0, sizeof(opts));
    opts.imtu = 64;
    opts.omtu = HIDP_DEFAULT_MTU; //64?
    opts.flush_to = 0xffff;

    setsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts));

    if (listen(sk, backlog) < 0) { //backlog = 5 instead of 10?
        close(sk);
        return -1;
    }

    return sk;
}

static int create_device(int ctl, int csk, int isk)
{
    struct hidp_connadd_req req;
    struct sockaddr_l2 addr;
    socklen_t addrlen;
    bdaddr_t src, dst;
    char bda[18];
    int err;

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if (getsockname(csk, (struct sockaddr *) &addr, &addrlen) < 0)
        return -1;

    bacpy(&src, &addr.l2_bdaddr);

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if (getpeername(csk, (struct sockaddr *) &addr, &addrlen) < 0)
        return -1;

    bacpy(&dst, &addr.l2_bdaddr);

    memset(&req, 0, sizeof(req));
    req.ctrl_sock = csk;
    req.intr_sock = isk;
    req.flags     = 0;
    req.idle_to   = 1800;

    err = get_stored_device_info(&src, &dst, &req);
    if (!err)
        goto create;

    err = get_sdp_device_info(&src, &dst, &req);
    if (err < 0)
        goto error;

create:
    ba2str(&dst, bda);
    syslog(LOG_INFO, "Connected %s (%s)", req.name, bda);

    if (req.vendor == 0x054c && req.product == 0x0268 && use_legacy != 1)
    {
        syslog(LOG_ERR, "Cannot start Sixaxis now; It should had been initialized before");
        return -1;
    }
    else
    {
        if (req.vendor == 0x054c && req.product == 0x0268) {
            enable_sixaxis(csk, led_n, enable_led_anim);
        }
        err = ioctl(ctl, HIDPCONNADD, &req);
    }

error:
    if (req.rd_data)
        free(req.rd_data);

    return err;
}

void l2cap_accept(int ctl, int csk, int isk)
{
    bdaddr_t bdaddr;
    int ctrl_socket, intr_socket, err;
    struct sockaddr_l2 addr;
    socklen_t addrlen;

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if ((ctrl_socket =
                accept(csk, (struct sockaddr *)&addr, &addrlen)) < 0) {
        syslog(LOG_INFO, "unable to accept control stream");
        return;
    }
    bacpy(&bdaddr, &addr.l2_bdaddr);

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if ((intr_socket =
                accept(isk, (struct sockaddr *)&addr, &addrlen)) < 0) {
        syslog(LOG_INFO, "unable to accept info stream");
        close(ctrl_socket);
        return;
    }

    if (bacmp(&bdaddr, &addr.l2_bdaddr)) {
        syslog(LOG_INFO,
               "intr and ctrl streams from different hosts - rejecting both");
        close(ctrl_socket);
        close(intr_socket);
        return;
    }

    int type = get_type(ctrl_socket);

    if (type == 1 && use_legacy != 1) {
        if (debug) printf("Will initiate Sixaxis now\n");

        char bda[18];
        ba2str(&addr.l2_bdaddr, bda);
        char *uinput_sixaxis_cmd = "/usr/sbin/sixad-uinput-sixaxis";
	char *led_n_str;

        int pid = fork();
        if (pid == 0) {

            close(ctl);
            close(csk);
            close(isk);

            dup2(ctrl_socket, 0);
            close(ctrl_socket);
            dup2(intr_socket, 1);
            close(intr_socket);
	    
	    if (led_n <= 1) { led_n_str = "1";
	    } else if (led_n == 2) { led_n_str = "2";
	    } else if (led_n == 3) { led_n_str = "3";
	    } else if (led_n == 4) { led_n_str = "4";
	    } else if (led_n == 5) { led_n_str = "5";
	    } else if (led_n == 6) { led_n_str = "6";
	    } else if (led_n >= 7) { led_n_str = "7";
	    } else { led_n_str = "1"; }
	    
            char *argv[] = { uinput_sixaxis_cmd, led_n_str, bda, NULL };

            char *envp[] = { NULL };

            syslog(LOG_INFO, "Connected PLAYSTATION(R)3 Controller (%s)", bda);

            if (execve(argv[0], argv, envp) < 0) {
                syslog(LOG_INFO, "cannot exec %s", uinput_sixaxis_cmd);
                close(0);
                close(1);
            }
            else
                if (debug) syslog(LOG_INFO, "Awesome! The device %s won't be disconnected when killing sixad", bda);
        }
    }
    else {
        err = create_device(ctl, ctrl_socket, intr_socket);
        if (debug) printf("Connected new device using the default driver\n");
        if (err < 0)
            syslog(LOG_ERR, "HID create error %d (%s)", errno, strerror(errno));
        close(intr_socket);
        close(ctrl_socket);
    }
    return;
}

static void run_server(int ctl, int csk, int isk)
{
    struct pollfd p[2];
    sigset_t sigs;
    short events;
    struct timespec timeout;

    sigfillset(&sigs);
    sigdelset(&sigs, SIGCHLD);
    sigdelset(&sigs, SIGPIPE);
    sigdelset(&sigs, SIGTERM);
    sigdelset(&sigs, SIGINT);
    sigdelset(&sigs, SIGHUP);

    if (debug) printf("Server mode now active, will start search now\n");

    p[0].fd = csk; //ctrl_listen
    p[0].events = POLLIN | POLLERR | POLLHUP;

    p[1].fd = isk; //intr_listen
    p[1].events = POLLIN | POLLERR | POLLHUP;

    while (!__io_canceled) {
        int idx = 2;
        int i;

        for (i = 0; i < idx; i++)
            p[i].revents = 0;

        timeout.tv_sec = 1;
        timeout.tv_nsec = 0;

        if (ppoll(p, idx, &timeout, &sigs) < 1)
            continue;

        events = p[0].revents | p[1].revents;

        if (events & POLLIN) {

            if (debug) printf("One event received\n");
            l2cap_accept(ctl, csk, isk);
            if (debug) printf("One event proccessed\n");

            if (led_js_n) {
                if (debug) printf("Will auto-change LED # to js#\n");
	    } else if (enable_ledplus && enable_leds) {
                /* for the next connection, don't allow the sixaxis to have the same LED; if no other choice, set to 1 */
                if (led_n == 1) {
                    led_n = 2;
                }
                else if (led_n == 2)  {
                    led_n = 3;
                }
                else if (led_n == 3)  {
                    led_n = 4;
                }
                else if (led_n == 4)  {
                    led_n = 5;
                }
                else if (led_n == 5)  {
                    led_n = 6;
                }
                else if (led_n == 6)  {
                    led_n = 7;
                }
                else if (led_n == 7)  {
                    led_n = 1;    //should it stay on LED #7 or go back to LED #1?
                }
                else  {
                    led_n = 1;
                }
                if (debug) printf("Changing next LED # to %i\n", led_n);
            } else {
		if (debug) printf("No changing next LED #, keeping it on %i\n", led_n);
	    }

        }

        if (events & (POLLERR | POLLHUP)) {
	    if (debug) printf("Main loop was broken...\n");
            break;
        }

    }
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    bdaddr_t bdaddr;
    char addr[18];
    int log_option = LOG_NDELAY | LOG_PID;
    int ctl, csk, isk, lm = 0;

    // sixad: leds led_js_n led_n ledplus led_anim legacy
    if (argc < 6) {
        printf("Running %s requires 'sixad'. Please run sixad instead\n",  argv[0]);
        exit(-1);
    }

    enable_leds = atoi(argv[1]);
    led_js_n = atoi(argv[2]);
    led_n = atoi(argv[3]);
    if (led_n < 1) {
        led_n = 1;
    } else if (led_n > 7) {
        led_n = 7;
    }
    enable_ledplus = atoi(argv[4]);
    enable_led_anim = atoi(argv[5]);
    use_legacy = atoi(argv[6]);
    
    log_option |= LOG_PERROR;
    openlog("sixad", log_option, LOG_DAEMON);
    
    int sfile[15];
    FILE *s = popen(". /etc/default/sixad; echo $Enable_leds $LED_js_n $LED_n $LED_plus $LED_anim $Enable_buttons $Enable_sbuttons $Enable_axis $Enable_accel $Enable_accon $Enable_speed $Enable_pos $Enable_rumble $Legacy $Debug", "r");
    if ( !s || fscanf(s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", &sfile[0], &sfile[1], &sfile[2], &sfile[3], &sfile[4], &sfile[5], &sfile[6], &sfile[7], &sfile[8], &sfile[9], &sfile[10], &sfile[11], &sfile[12], &sfile[13], &sfile[14]) != 15 ) {
      syslog(LOG_ERR, "No valid sixad configuration file found, using default settings");
    } else {
	if (use_legacy) {
	    printf("Legacy driver workaround enabled (no neat stuff...)\n");
	}
	else {
	    printf("sixad settings:\nEnable LEDs:\t%i\njs# as LED #:\t%i\nStart LED #:\t%i\nLED # increase:\t%i\nLED animation:\t%i\nButtons:\t%i\nSens. buttons:\t%i\nAxis:\t\t%i\nAccelerometers:\t%i\nAcceleration:\t%i\nSpeed:\t\t%i\nPosition:\t%i\nRumble:\t\t%i\nDebug:\t\t%i\n", sfile[0], sfile[1], sfile[2], sfile[3], sfile[4], sfile[5], sfile[6], sfile[7], sfile[8], sfile[9], sfile[10], sfile[11], sfile[12], sfile[14]);
	}
    }
        
    //removed code after 0.8.0
    bacpy(&bdaddr, BDADDR_ANY);
    
    if (bacmp(&bdaddr, BDADDR_ANY))
        syslog(LOG_INFO, "sixad started (adress %s), press the PS button now", addr);
    else
        syslog(LOG_INFO, "sixad started, press the PS button now");
    //removed code after 0.8.0

    lm |= L2CAP_LM_MASTER;

    ba2str(&bdaddr, addr);

    ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HIDP);
    if (ctl < 0) {
        perror("Can't open HIDP control socket");
        exit(1);
    }

    csk = l2cap_listen(&bdaddr, L2CAP_PSM_HIDP_CTRL, lm, 10); //ctrl_listen
    if (csk < 0) {
        perror("Can't listen on HID control channel");
        close(ctl);
        exit(1);
    }

    isk = l2cap_listen(&bdaddr, L2CAP_PSM_HIDP_INTR, lm, 10); //intr_listen
    if (isk < 0) {
        perror("Can't listen on HID interrupt channel");
        close(ctl);
        close(csk);
        exit(1);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_NOCLDSTOP;

    sa.sa_handler = sig_term;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sa.sa_handler = sig_hup;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);

    run_server(ctl, csk, isk); //csk = ctrl_listen; isk = intr_listen

    syslog(LOG_INFO, "Exit");

    close(csk);
    close(isk);
    close(ctl);

    return 0;
}
