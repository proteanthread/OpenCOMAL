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
 * OpenComal Routines for external OpenComal modules 
 */

#define _XOPEN_SOURCE 700

#include "pdcnana.h"
#include "pdcglob.h"
#include "pdcmisc.h"
#include "pdclexs.h"
#include "pdcid.h"
#include "pdcseg.h"
#include "pdcexec.h"
#include "pdcmod.h"

#include "fmt.h"

/*
 * The root of the loaded module table
 */
PRIVATE struct mod_entry *mod_root;

/*
 * The root of the exported routine table
 */
PRIVATE struct mod_func_entry *mod_func_root;

/*
 * Find a PROC or FUNC in a module
 */
struct comal_line *
find_func(struct comal_line *module, struct id_rec *fun)
{
    struct comal_line *walk;

    for (walk = module->lc.pfrec.localproc; walk;
         walk = walk->lc.pfrec.proclink)
        if (id_eql(walk->lc.pfrec.id, fun))
            return walk;

    return NULL;
}

/*
 * Register an OpenComal module
 */
PRIVATE bool
mod_register(struct comal_line *line, char *errtext,
             struct comal_line **errline)
{
    struct comal_line *walk;
    struct mod_entry *work = GETCORE(RUN_POOL, struct mod_entry);
    struct id_list *idwalk;
    struct comal_line *fun;
    struct mod_func_entry *modfun;

    work->next = mod_root;
    mod_root = work;
    work->id = line->lc.pfrec.id;
    work->line = line;

    DBG_PRINTF(true, "Registering MODULE %s", line->lc.pfrec.id->name);

    /*
     * Find all the exports in this MODULE and register them
     */
    for (walk = line; walk->cmd != endmoduleSYM; walk = walk->ld->next)
        if (walk->cmd == exportSYM)
            for (idwalk = walk->lc.idroot; idwalk; idwalk = idwalk->next) {
                fun = find_func(line, idwalk->id);

                if (!fun) {
                    Fmt_sfmt(errtext, MAX_LINELEN,
                             "Can not find PROC/FUNC %s in MODULE %s",
                             idwalk->id->name, line->lc.pfrec.id->name);
                    *errline = walk;
                    return false;
                }

                DBG_PRINTF(true, "  Exporting %s", fun->lc.pfrec.id->name);

                modfun = GETCORE(RUN_POOL, struct mod_func_entry);
                modfun->next = mod_func_root;
                mod_func_root = modfun;
                modfun->id = idwalk->id;
                modfun->line = fun;
                modfun->module = work;
            }

    return true;
}

struct mod_entry *
mod_find(struct id_rec *id)
{
    struct mod_entry *walk = mod_root;

    while (walk) {
        if (id_eql(walk->id, id))
            return walk;

        walk = walk->next;
    }

    return NULL;
}

PUBLIC struct comal_line *
mod_find_def(struct seg_des *seg, struct id_rec *id)
{
    struct comal_line *walk;

    FOR_EACH_LINE(seg, walk)
        if (walk->cmd == moduleSYM && id_eql(walk->lc.pfrec.id, id))
        return walk;

    return NULL;
}

PUBLIC bool
mod_use(struct seg_des *seg, struct id_rec *id, char *errtxt,
        struct comal_line **errline)
{
    struct comal_line *defline;

    /*
     * If this module has already been 
     * defined, there is nothing more we need to do
     */
    if (mod_find(id))
        return true;

    /*
     * Try to find the module inside this segment
     */
    defline = mod_find_def(seg, id);

    if (!defline) {
        Fmt_sfmt(errtxt, MAX_LINELEN, "Module %s can not be found",
                 id->name);
        return false;
    }

    return mod_register(defline, errtxt, errline);
}

PUBLIC void
mod_freeall(void)
{
    free_list((struct my_list *) mod_func_root);
    free_list((struct my_list *) mod_root);
    mod_func_root = NULL;
    mod_root = NULL;
}

PUBLIC struct comal_line *
mod_search_routine(struct id_rec *id, int type)
{
    struct mod_func_entry *walk = mod_func_root;

    while (walk) {
        if (id_eql(walk->id, id) && walk->line->cmd == type)
            return walk->line;

        walk = walk->next;
    }

    return NULL;
}

PUBLIC void
mod_initall(void)
{
    struct mod_entry *walk = mod_root;

    while (walk) {
        exec_mod_init(walk->line);
        walk = walk->next;
    }
}
