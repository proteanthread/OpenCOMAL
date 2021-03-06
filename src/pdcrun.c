/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/*
 * Main file of OpenComal Command loop -- runtime only version 
 */

#define _XOPEN_SOURCE 700

#include "pdcglob.h"
#include "pdcstr.h"
#include "pdcprog.h"
#include "pdcmisc.h"
#include "pdcrun.h"

PUBLIC int      yydebug = 0;    /* To replace YACC's yydebug */
PUBLIC bool     show_exec = false;      /* To replace PDCLIST.C's
                                         * show_exec */


PUBLIC const char *
sys_interpreter(void)
{
    return "OpenComalRun";
}


PUBLIC const char *
lex_sym(int sym __my_unused)
{
    return "<Undefined>";
}


PUBLIC void
line_list(char **buf, struct comal_line *line __my_unused)
{
    **buf = '\0';
}


PUBLIC void
comal_loop(int newstate __my_unused)
{
    my_printf(MSG_ERROR, true, "Aborting...");
    longjmp(RESTART, QUIT);
}

PRIVATE char   *
get_runfilename(void)
{
    char            buf[128];

    if (sys_get
        (MSG_DIALOG, buf, sizeof(buf),
         "Enter filename of program to execute: "))
        return NULL;

    return my_strdup(MISC_POOL, buf);
}

PUBLIC void
pdc_go(int argc, char *argv[])
{
    if (argc == 1)
        runfilename = get_runfilename();
    else
        runfilename = my_strdup(MISC_POOL, argv[1]);

    if (runfilename && !setjmp(ERRBUF) && !setjmp(RESTART))
        prog_run();

    if (curenv->error)
        give_run_err(curenv->errline);
}
