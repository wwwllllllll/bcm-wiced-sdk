/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 * Linux port of wl command line utility
 */

#include "typedefs.h"
#include <stdio.h>
#include "bcmutils.h"
#include "wlu_cmd.h"
#include <string.h>
#include "wlu_remote.h"
#include "wlu.h"

#define INTERACTIVE_NUM_ARGS            15
#define INTERACTIVE_MAX_INPUT_LENGTH    256
#define RWL_WIFI_JOIN_DELAY                5

/* Function prototypes */
static cmd_t *wl_find_cmd(char* name);
static int do_interactive( void* ifr);
static int wl_do_cmd( void* ifr, char **argv);
int process_args( void* ifr, char **argv);

/* RemoteWL declarations */
extern char g_rem_ifname[IFNAMSIZ];
extern char *g_rwl_buf_mac;
extern char* g_rwl_device_name_serial;
unsigned short g_rwl_servport;
char *g_rwl_servIP = NULL;
int remote_type = NO_REMOTE;
unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;
static uint interactive_flag = 0;
/* Main client function */

int
main(int argc, char **argv)
{
    void*  ifr;
    char *ifname = NULL;
    int err = 0;
    int help = 0;
    int status = CMD_WL;
    void* serialHandle = NULL;
    struct ipv4_addr temp;


    wlu_av0 = argv[0];

    wlu_init();

    argv++;

    if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
        if (ifname)
        {
            strncpy(g_rem_ifname, ifname, IFNAMSIZ);
        }
    }

    int found;
    do
    {
        found = 0;
        /* RWL socket transport Usage: --serial port_name */
        if ( *argv && strncmp ( *argv, "--serial", strlen( *argv )) == 0) {
            found = 1;
            argv++;
            remote_type = REMOTE_SERIAL;

            if (!(*argv)) {
                rwl_usage(remote_type);
                return err;
            }


            if ((serialHandle = rwl_open_transport(remote_type, *argv, 0, 0)) == NULL) {
                DPRINT_ERR(ERR, "serial device open error\n");
                return -1;
            }

            argv++;
            ifr = (void*) serialHandle;

        }

        if (( *argv ) && (strlen( *argv ) > 2) &&
            (strncmp( *argv, "--interactive", strlen( *argv )) == 0)) {
            interactive_flag = 1;
            found = 1;
            argv++;
        }
    } while ( found == 1 );

    if ( remote_type == NO_REMOTE )
    {
        printf( "Error: --serial <port> must be specified for Wiced serial WL app\n");
        return -1;
    }

    if (interactive_flag == 1) {
        err = do_interactive(ifr);
        return err;
    }

    if ((*argv) && (interactive_flag == 0)) {
        err = process_args(ifr, argv);
        return err;
    }
    rwl_usage(remote_type);

    if (remote_type != NO_REMOTE )
        rwl_close_transport(remote_type, ifr);

    return err;
}

/*
 * Function called for  'local' execution and for 'remote' non-interactive session
 * (shell cmd, wl cmd)
 */
int
process_args( void* ifr, char **argv)
{
    char *ifname = NULL;
    int help = 0;
    int status = 0, retry;
    int err = 0;
    cmd_t *cmd = NULL;

    while (*argv) {
        if ((strcmp (*argv, "sh") == 0) && (remote_type != NO_REMOTE)) {
            argv++; /* Get the shell command */
            if (*argv) {
                err = rwl_shell_cmd_proc( ifr, argv, SHELL_CMD);
            } else {
                DPRINT_ERR(ERR, "Enter the shell \
                    command(e.g ls(Linux) or dir(Win CE) \n");
                err = -1;
            }
            return err;
        }

#ifdef RWLASD
        if ((strcmp (*argv, "asd") == 0) && (remote_type != NO_REMOTE)) {
            argv++; /* Get the asd command */
            if (*argv) {
                err = rwl_shell_cmd_proc((void*)ifr, argv, ASD_CMD);
            } else {
                DPRINT_ERR(ERR, "Enter the asd command (e.g ca_get_version \n");
                err = -1;
            }
            return err;
        }
#endif

        if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
            if (help)
                break;
            if (ifname)
            {
                strncpy(g_rem_ifname, ifname, IFNAMSIZ);
            }
            continue;
        }
        /* parse error */
        else if (status == CMD_ERR)
            break;

        /* search for command */
        cmd = wl_find_cmd(*argv);

        /* do command */
        if ( cmd )
        {
            err = (*cmd->func)((void *) ifr, cmd, argv);
        }
        else
        {
            /* if not found, use default set_var and get_var commands */
            err = wl_varcmd.func((void *) ifr, cmd, argv);
            if ( err == 0 )
            {
                /* get/set var succeeded - pretend we used the command all along */
                cmd = &wl_varcmd;
            }
        }
        break;
    } /* while loop end */

/* provide for help on a particular command */
    if (help && *argv) {
        cmd = wl_find_cmd(*argv);
        if (cmd) {
            wl_cmd_usage(stdout, cmd);
        } else {
            DPRINT_ERR(ERR, "%s: Unrecognized command \"%s\", type -h for help\n",
                                                                      wlu_av0, *argv);
        }
    } else if (!cmd)
        wl_usage(stdout, NULL);
    else if (err == USAGE_ERROR)
        wl_cmd_usage(stderr, cmd);
    else if (err != 0)
        wl_printlasterror((void *) ifr);

    return err;
}

/* Function called for 'local' interactive session and for 'remote' interactive session */
static int
do_interactive( void* ifr )
{
    int err = 0, retry;

    while (1) {
        char *fgsret;
        char line[INTERACTIVE_MAX_INPUT_LENGTH];
        fprintf(stdout, "> ");
        fflush( stdout );
        fflush( stderr );
        fgsret = fgets(line, sizeof(line), stdin);

        /* end of file */
        if (fgsret == NULL)
            break;
        if (line[0] == '\n')
            continue;

        if (strlen (line) > 0) {
            /* skip past first arg if it's "wl" and parse up arguments */
            char *argv[INTERACTIVE_NUM_ARGS];
            int argc;
            char *token;
            argc = 0;

            while ((argc < (INTERACTIVE_NUM_ARGS - 1)) &&
                   ((token = strtok(argc ? NULL : line, " \t\n")) != NULL)) {
                argv[argc++] = token;
            }
            argv[argc] = NULL;

            if (strcmp(argv[0], "q") == 0 || strcmp(argv[0], "exit") == 0) {
                break;
            }

            if ((strcmp(argv[0], "sh") == 0) && (remote_type != NO_REMOTE))  {
                if (argv[1]) {
                    process_args(ifr, argv);
                } else {
                    DPRINT_ERR(ERR, "Give shell command");
                    continue;
                }
            } else { /* end shell */
                err = wl_do_cmd(ifr, argv);
            } /* end of wl */
            fflush( stdout );
            fflush( stderr );
        } /* end of strlen (line) > 0 */
    } /* while (1) */

    return err;
}

/*
 * find command in argv and execute it
 * Won't handle changing ifname yet, expects that to happen with the --interactive
 * Return an error if unable to find/execute command
 */
static int
wl_do_cmd( void* ifr, char **argv)
{
    cmd_t *cmd = NULL;
    int err = 0;
    int help = 0;
    char *ifname = NULL;
    int status = CMD_WL;

    /* skip over 'wl' if it's there */
    if (*argv && strcmp (*argv, "wl") == 0) {
        argv++;
    }

    /* handle help or interface name changes */
    if (*argv && (status = wl_option (&argv, &ifname, &help)) == CMD_OPT) {
        if (ifname) {
            strncpy(g_rem_ifname, ifname, IFNAMSIZ);
        }
    }

    /* in case wl_option eats all the args */
    if (!*argv) {
        return err;
    }

    if (status != CMD_ERR) {
        /* search for command */
        cmd = wl_find_cmd(*argv);

        /* defaults to using the set_var and get_var commands */
        if (!cmd) {
            cmd = &wl_varcmd;
        }
        /* do command */
        err = (*cmd->func)((void *)ifr, cmd, argv);
    }
    /* provide for help on a particular command */
    if (help && *argv) {
      cmd = wl_find_cmd(*argv);
     if (cmd) {
        wl_cmd_usage(stdout, cmd);
    } else {
            DPRINT_ERR(ERR, "%s: Unrecognized command \"%s\", type -h for help\n",
                   wlu_av0, *argv);
           }
    } else if (!cmd)
        wl_usage(stdout, NULL);
    else if (err == USAGE_ERROR)
        wl_cmd_usage(stderr, cmd);
    else if (err == IOCTL_ERROR)
        wl_printlasterror((void *)ifr);

    return err;
}

/* Search the wl_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t *
wl_find_cmd(char* name)
{
    cmd_t *cmd = NULL;

    /* search the wl_cmds for a matching name */
    for (cmd = wl_cmds; cmd->name && strcmp(cmd->name, name); cmd++);

    if (cmd->name == NULL)
        cmd = NULL;

    return cmd;
}

int
wl_get(void *wl, int cmd, void *buf, int len)
{
    unsigned long longlen = len;
    return rwl_queryinformation_fe( wl, cmd, buf, &longlen, 0, REMOTE_GET_IOCTL );
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
    unsigned long longlen = len;
    if (len >1514)
    {
        printf("long packet");
    }
    return rwl_setinformation_fe( wl, cmd, buf, &longlen, 0, REMOTE_SET_IOCTL );
}

