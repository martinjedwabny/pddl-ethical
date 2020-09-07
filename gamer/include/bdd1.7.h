// ***********************************************************
// 
//  Book:       Heuristic Search
// 
//  Authors:    S.Edelkamp, S.Schroedl
// 
//  See file README for information on using and copying 
//  this software.
// 
//  Project:    Mips - model checking integrated planning
//              system
// 
//  Module:     mips\include\bdd.h
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

/*========================================================================
                            ** BuDDy *************************
                      BINARY DECISION DIAGRAMS
                               with
                  FINITE DOMAIN VARIABLE INTERFACE
                      Library Package ver. 1.5
                        By Jorn Lind-Nielsen

            Copyright (C) 1996-1998 by Jorn Lind-Nielsen

    Permission is hereby granted to reproduce and distribute this
    package by any means and for any fee, whether alone or as part
    of a larger distribution, in source or in binary form, provided
    this notice is included with any such distribution and is visible 
    for the end user, and is not removed from any of its header files. 

      *** I AM NOT RESPONSIBLE FOR ANY KIND OF DAMAGE TO YOUR 
      *** FILES, DATA, HARDWARE, LOSS OF MONEY, SYSTEM CRASHES *** 
      *** OR ANY OTHER THING YOU MIGHT COME UP WITH.          
      *** - USE THIS PROGRAM OF YOUR OWN FREE WILL !!!        

      Happy Hacking
                   Jorn Lind-Nielsen

========================================================================*/

/*************************************************************
  $Header: /home/jl/phd/bdd/src/RCS/bdd.h,v 1.11 1998/07/16 07:06:33 jl Exp jl $
  FILE:  bdd.h
  DESCR: C,C++ User interface for the BDD package
  AUTH:  Jorn Lind
  DATE:  (C) feb 1997
*************************************************************************/

#ifndef _BDD_H
#define _BDD_H

   /* Allow this headerfile to define C++ constructs if requested */
#ifdef __cplusplus
#define CPLUSPLUS
#endif

#include <stdio.h>

/*=== Defined operators for apply calls ================================*/

#define bddop_and       0
#define bddop_xor       1
#define bddop_or        2
#define bddop_nand      3
#define bddop_nor       4
#define bddop_imp       5
#define bddop_biimp     6
#define bddop_diff      7
#define bddop_less      8
#define bddop_invimp    9

   /* Should *not* be used in bdd_apply calls !!! */
#define bddop_not      10
#define bddop_simplify 11


/*=== User BDD types ===================================================*/

typedef int BDD;

#ifndef CPLUSPLUS
typedef BDD bdd;
#endif /* CPLUSPLUS */


typedef struct s_bddPair
{
   BDD *result;
   int last;
   int id;
   struct s_bddPair *next;
} bddPair;


/*=== Status information ===============================================*/

/*
NAME    {* bddStat *}
SECTION {* kernel *}
SHORT   {* Status information about the bdd package *}
PROTO   {* typedef struct s_bddStat
{
   long int produced;
   int nodenum;
   int maxnodenum;
   int freenodes;
   int minfreenodes;
   int varnum;
   int cachesize;
   int gbcnum;
} bddStat;  *}
DESCR   {* The fields are \\[\baselineskip] \begin{tabular}{ll}
  {\tt produced}     & total number of new nodes ever produced \\
  {\tt nodenum}      & currently allocated number of bdd nodes \\
  {\tt maxnodenum}   & user defined maximum number of bdd nodes \\
  {\tt freenodes}    & number of currently free nodes \\
  {\tt minfreenodes} & min. number of nodes that should be left after a
                       garbage collection. \\
  {\tt varnum}       & number of defined bdd variables \\
  {\tt cachesize}    & number of entries in the internal caches \\
  {\tt gbcnum}       & number of garbage collections done until now
  \end{tabular} *}
ALSO    {* bdd\_stats *}
*/
typedef struct s_bddStat
{
   long int produced;
   int nodenum;
   int maxnodenum;
   int freenodes;
   int minfreenodes;
   int varnum;
   int cachesize;
   int gbcnum;
} bddStat;


/*
NAME    {* bddGbcStat *}
SECTION {* kernel *}
SHORT   {* Status information about garbage collections *}
PROTO   {* typedef struct s_bddGbcStat
{
   int nodes;
   int freenodes;
   long time;
   long sumtime;
   int num;
} bddGbcStat;  *}
DESCR   {* The fields are \\[\baselineskip] \begin{tabular}{ll}
  {\tt nodes}     & Total number of allocated nodes in the nodetable \\
  {\tt freenodes} & Number of free nodes in the nodetable \\
  {\tt time}      & Time used for garbage collection this time \\
  {\tt sumtime}   & Total time used for garbage collection \\
  {\tt num}       & number of garbage collections done until now
  \end{tabular} *}
ALSO    {* bdd\_gbc\_hook *}
*/
typedef struct s_bddGbcStat
{
   int nodes;
   int freenodes;
   long time;
   long sumtime;
   int num;
} bddGbcStat;


/*=== BDD interface prototypes =========================================*/

/*
NAME    {* bdd\_relprod *}
SECTION {* operator *}
SHORT   {* relational product *}
PROTO   {* #define bdd_relprod(a,b,var) bdd_appex(a,b,bddop_and,var) *}
DESCR   {* Calculates the relational product of {\tt a} and {\tt b} as
           {\tt a AND b} with the variables in {\tt var} quantified out
       afterwards. *}
RETURN  {* The relational product or {\tt bddfalse} on errors. *}
ALSO    {* bdd\_appex *}
*/
#define bdd_relprod(a,b,var) bdd_appex((a),(b),bddop_and,(var))


  /* In file "kernel.c" */

#ifdef CPLUSPLUS
extern "C" {
#endif

typedef void (*bddinthandler)(int);
typedef void (*bddgbchandler)(int,bddGbcStat*);
typedef void (*bdd2inthandler)(int,int);
   
extern bddinthandler  bdd_error_hook(bddinthandler);
extern bddgbchandler  bdd_gbc_hook(bddgbchandler);
extern bdd2inthandler bdd_resize_hook(bdd2inthandler);
extern bdd2inthandler bdd_reorder_hook(bdd2inthandler);

extern int      bdd_init(int, int);
extern void     bdd_done(void);
extern int      bdd_setvarnum(int);
extern int      bdd_extvarnum(int);
extern int      bdd_isrunning(void);
extern int      bdd_setmaxnodenum(int);
extern int      bdd_setmaxincrease(int);
extern int      bdd_setminfreenodes(int);
extern int      bdd_getnodenum(void);
extern char*    bdd_versionstr(void);
extern int      bdd_versionnum(void);
extern void     bdd_stats(bddStat *);
extern void     bdd_default_gbchandler(int, bddGbcStat *);
extern void     bdd_default_errhandler(int);
extern const char *bdd_errstring(int);
#ifndef CPLUSPLUS
extern BDD      bdd_true(void);
extern BDD      bdd_false(void);
#endif
extern int      bdd_varnum(void);
extern BDD      bdd_ithvar(int);
extern BDD      bdd_nithvar(int);
extern int      bdd_var(BDD);
extern BDD      bdd_low(BDD);
extern BDD      bdd_high(BDD);
extern int      bdd_varlevel(int);
extern BDD      bdd_addref(BDD);
extern BDD      bdd_delref(BDD);
extern void     bdd_gbc(void);
extern int      bdd_scanset(BDD, int**, int*);
extern BDD      bdd_makeset(int *, int);
extern bddPair* bdd_newpair(void);
extern int      bdd_setpair(bddPair*, int, int);
extern int      bdd_setpairs(bddPair*, int*, int*, int);
extern int      bdd_setbddpair(bddPair*, int, BDD);
extern int      bdd_setbddpairs(bddPair*, int*, BDD*, int);
extern void     bdd_resetpair(bddPair *);
extern void     bdd_freepair(bddPair*);

  /* In bddop.c */

extern BDD      bdd_not(BDD);
extern BDD      bdd_apply(BDD, BDD, int);
extern BDD      bdd_ite(BDD, BDD, BDD);
extern BDD      bdd_restrict(BDD, BDD);
extern BDD      bdd_constraint(BDD, BDD);
extern BDD      bdd_replace(BDD, bddPair*);
extern BDD      bdd_compose(BDD, BDD, BDD);
extern BDD      bdd_veccompose(BDD, bddPair*);
extern BDD      bdd_simplify(BDD, BDD);
extern BDD      bdd_simplex(BDD, BDD, BDD);
extern BDD      bdd_exist(BDD, BDD);
extern BDD      bdd_forall(BDD, BDD);
extern BDD      bdd_appex(BDD, BDD, int, BDD);
extern BDD      bdd_appall(BDD, BDD, int, BDD);
extern BDD      bdd_support(BDD);
extern BDD      bdd_satone(BDD);
extern BDD      bdd_fullsatone(BDD);
extern double   bdd_satcount(BDD);
extern double   bdd_satcountset(BDD, BDD);
extern double   bdd_satcountln(BDD);
extern double   bdd_satcountlnset(BDD, BDD);
extern int      bdd_nodecount(BDD);
extern int*     bdd_varprofile(BDD);

/* In file "bddio.c" */

extern void     bdd_printall(void);
extern void     bdd_fprintall(FILE *);
extern void     bdd_fprinttable(FILE *, BDD);
extern void     bdd_printtable(BDD);
extern void     bdd_fprintset(FILE *, BDD);
extern void     bdd_printset(BDD);
extern int      bdd_fnprintdot(char *, BDD);
extern void     bdd_fprintdot(FILE *, BDD);
extern void     bdd_printdot(BDD);
extern int      bdd_fnsave(char *, BDD);
extern int      bdd_save(FILE *, BDD);
extern int      bdd_fnload(char *, BDD *);
extern int      bdd_load(FILE *ifile, BDD *);

/* In file reorder.c */

extern void     bdd_default_reohandler(int,int);
extern void     bdd_reorder(int);
extern void     bdd_clrvarblocks(void);
extern int      bdd_addvarblock(BDD, int);
extern int      bdd_intaddvarblock(int, int, int);
extern int      bdd_autoreorder(int);
extern int      bdd_reorder_verbose(int);
extern void     bdd_printorder(void);
extern void     bdd_fprintorder(FILE *);

/* In file fdd.c */

extern int      fdd_extdomain(int*, int);
extern int      fdd_overlapdomain(int, int);
extern void     fdd_clearall();
extern int      fdd_domainnum(void);
extern int      fdd_domainsize(int);
extern int      fdd_varnum(int);
extern int*     fdd_vars(int);
extern BDD      fdd_ithvar(int, int);
extern BDD      fdd_ithset(int);
extern BDD      fdd_domain(int);
extern BDD      fdd_equals(int, int);
extern void     fdd_printset(BDD);
extern void     fdd_fprintset(FILE*, BDD);
extern int      fdd_scanset(BDD, int**, int*);
extern BDD      fdd_makeset(int*, int);
extern int      fdd_intaddvarblock(int, int, int);
extern int      fdd_setpair(bddPair*, int, int);
extern int      fdd_setpairs(bddPair*, int*, int*, int);

#ifdef CPLUSPLUS
}
#endif


/*=== BDD constants ====================================================*/

#ifndef CPLUSPLUS

extern const BDD bddfalse;
extern const BDD bddtrue;

#endif /* CPLUSPLUS */


/*=== Reordering algorithms ============================================*/

#define BDD_REORDER_NONE     0
#define BDD_REORDER_WIN2     1
#define BDD_REORDER_WIN2ITE  2
#define BDD_REORDER_SIFT     3
#define BDD_REORDER_SIFTITE  4
#define BDD_REORDER_RANDOM   5

#define BDD_REORDER_FREE     0
#define BDD_REORDER_FIXED    1


/*=== Error codes ======================================================*/

#define BDD_MEMORY (-1)   /* Out of memory */
#define BDD_VAR (-2)      /* Unknown variable */
#define BDD_RANGE (-3)    /* Variable value out of range (not in domain) */
#define BDD_DEREF (-4)    /* Removing external reference to unknown node */
#define BDD_RUNNING (-5)  /* Called bdd_init() twice whithout bdd_done() */
#define BDD_FILE (-6)     /* Some file operation failed */
#define BDD_FORMAT (-7)   /* Incorrect file format */
#define BDD_ORDER (-8)    /* Vars. not in order for vector based functions */
#define BDD_BREAK (-9)    /* User called break */
#define BDD_VARNUM (-10)  /* Different number of vars. for vector pair */
#define BDD_NODES (-11)   /* Tried to set max. number of nodes to be fewer */
                          /* than there already has been allocated */
#define BDD_OP (-12)      /* Unknown operator */
#define BDD_VARSET (-13)  /* Illegal variable set */
#define BDD_VARBLK (-14)  /* Bad variable block */
#define BDD_DECVNUM (-15) /* Trying to decrease the number of variables */
#define BDD_REPLACE (-16) /* Replacing to already existing variables */
#define BDD_NODENUM (-17) /* Number of nodes reached user defined maximum */
#define BDD_ILLBDD (-18)  /* Illegal bdd argument */
#define BDD_SIZE (-19)    /* Illegal size argument */

#define BDD_ERRNUM 19

/*************************************************************
   If this file is included from a C++ compiler then the following
   classes, wrappers and hacks are supplied.
*************************************************************************/
#ifdef CPLUSPLUS
#include <iostream>

/*=== User BDD class ===================================================*/


class bdd
{
 public:

   bdd(void)           { root=0; }
   bdd(const bdd &r)   { bdd_addref(root=r.root); }
   ~bdd(void)          { bdd_delref(root); }

   int id(void) const;
   
   bdd operator=(const bdd &r);
   
   bdd operator&(const bdd &r) const;
   bdd operator&=(const bdd &r);
   bdd operator^(const bdd &r) const;
   bdd operator^=(const bdd &r);
   bdd operator|(const bdd &r) const;
   bdd operator|=(const bdd &r);
   bdd operator!(void) const;
   bdd operator>>(const bdd &r) const;
   bdd operator>>=(const bdd &r);
   bdd operator-(const bdd &r) const;
   bdd operator-=(const bdd &r);
   bdd operator>(const bdd &r) const;
   bdd operator<(const bdd &r) const;
   bdd operator<<(const bdd &r) const;
   bdd operator<<=(const bdd &r);
   int operator==(const bdd &r) const;
   int operator!=(const bdd &r) const;
   
private:
   int root;

   bdd(int r) { bdd_addref(root=r); }
   bdd operator=(int r);

   friend int      bdd_init(int, int);
   friend int      bdd_setvarnum(int);
   friend bdd      bdd_true(void);
   friend bdd      bdd_false(void);
   friend bdd      bdd_ithvarpp(int);
   friend bdd      bdd_nithvarpp(int);
   friend int      bdd_var(const bdd &);
   friend bdd      bdd_low(const bdd &);
   friend bdd      bdd_high(const bdd &);
   friend int      bdd_scanset(const bdd &, int *&, int &);
   friend bdd      bdd_makesetpp(int *, int);
   friend int      bdd_setbddpair(bddPair*, int, const bdd &);
   friend int      bdd_setbddpairs(bddPair*, int*, const bdd *, int);
   friend bdd      bdd_not(const bdd &);
   friend bdd      bdd_simplify(const bdd &, const bdd &);
   friend bdd      bdd_simplex(const bdd &, const bdd &, const bdd &);
   friend bdd      bdd_apply(const bdd &, const bdd &, int);
   friend bdd      bdd_ite(const bdd &, const bdd &, const bdd &);
   friend bdd      bdd_restrict(const bdd &, const bdd &);
   friend bdd      bdd_constraint(const bdd &, const bdd &);
   friend bdd      bdd_exist(const bdd &, const bdd &);
   friend bdd      bdd_forall(const bdd &, const bdd &);
   friend bdd      bdd_appex(const bdd &, const bdd &, int, const bdd &);
   friend bdd      bdd_appall(const bdd &, const bdd &, int, const bdd &);
   friend bdd      bdd_replace(const bdd &, bddPair*);
   friend bdd      bdd_compose(const bdd &, const bdd &, int);
   friend bdd      bdd_veccompose(const bdd &, bddPair*);
   friend bdd      bdd_support(const bdd &);
   friend bdd      bdd_satone(const bdd &);
   friend bdd      bdd_fullsatone(const bdd &);
   friend double   bdd_satcount(const bdd &);
   friend double   bdd_satcountset(const bdd &, const bdd &);
   friend double   bdd_satcountln(const bdd &);
   friend double   bdd_satcountlnset(const bdd &, const bdd &);
   friend int      bdd_nodecount(const bdd &);
   friend int*     bdd_varprofile(const bdd &);
   
   friend void   bdd_fprinttable(FILE *, const bdd &);
   friend void   bdd_printtable(const bdd &);
   friend void   bdd_fprintset(FILE *, const bdd &);
   friend void   bdd_printset(const bdd &);
   friend void   bdd_printdot(const bdd &);
   friend int    bdd_fnprintdot(char*, const bdd &);
   friend void   bdd_fprintdot(FILE*, const bdd &);
   friend ostream &operator<<(ostream &, const bdd &);
   friend int    bdd_fnsave(char*, const bdd &);
   friend int    bdd_save(FILE*, const bdd &);
   friend int    bdd_fnload(char*, bdd &);
   friend int    bdd_load(FILE*, bdd &);
   
   friend bdd    fdd_ithvarpp(int, int);
   friend bdd    fdd_ithsetpp(int);
   friend bdd    fdd_domainpp(int);
   friend bdd    fdd_equalspp(int, int);
   friend void   fdd_printset(const bdd &);
   friend void   fdd_fprintset(FILE*, const bdd &);
   friend bdd    fdd_makesetpp(int*, int);
   friend int    fdd_scanset(const bdd &, int *&, int &);

   friend int    bdd_addvarblock(const bdd &, int);
};


/*=== BDD constants ====================================================*/

extern const bdd bddfalsepp;
extern const bdd bddtruepp;

#define bddtrue bddtruepp
#define bddfalse bddfalsepp

/*=== Inline C++ interface =============================================*/

inline void bdd_stats(bddStat& s)
{ bdd_stats(&s); }

inline bdd bdd_ithvarpp(int v)
{ return bdd_ithvar(v); }

inline bdd bdd_nithvarpp(int v)
{ return bdd_nithvar(v); }

inline int bdd_var(const bdd &r)
{ return bdd_var(r.root); }

inline bdd bdd_low(const bdd &r)
{ return bdd_low(r.root); }

inline bdd bdd_high(const bdd &r)
{ return bdd_high(r.root); }

inline int bdd_scanset(const bdd &r, int *&v, int &n)
{ return bdd_scanset(r.root, &v, &n); }

inline bdd bdd_makesetpp(int *v, int n)
{ return bdd(bdd_makeset(v,n)); }

inline int bdd_setbddpair(bddPair *p, int ov, const bdd &nv)
{ return bdd_setbddpair(p,ov,nv.root); }

   /* In bddop.c */

inline bdd bdd_replace(const bdd &r, bddPair *p)
{ return bdd_replace(r.root, p); }

inline bdd bdd_compose(const bdd &f, const bdd &g, int v)
{ return bdd_compose(f.root, g.root, v); }

inline bdd bdd_veccompose(const bdd &f, bddPair *p)
{ return bdd_veccompose(f.root, p); }

inline bdd bdd_restrict(const bdd &r, const bdd &var)
{ return bdd_restrict(r.root, var.root); }

inline bdd bdd_constraint(const bdd &f, const bdd &c)
{ return bdd_constraint(f.root, c.root); }

inline bdd bdd_simplify(const bdd &d, const bdd &b)
{ return bdd_simplify(d.root, b.root); }

inline bdd bdd_simplex(const bdd &d, const bdd &b, const bdd &v)
{ return bdd_simplex(d.root, b.root, v.root); }

inline bdd bdd_not(const bdd &r)
{ return bdd_not(r.root); }

inline bdd bdd_apply(const bdd &l, const bdd &r, int op)
{ return bdd_apply(l.root, r.root, op); }

inline bdd bdd_ite(const bdd &f, const bdd &g, const bdd &h)
{ return bdd_ite(f.root, g.root, h.root); }

inline bdd bdd_exist(const bdd &r, const bdd &var)
{ return bdd_exist(r.root, var.root); }

inline bdd bdd_forall(const bdd &r, const bdd &var)
{ return bdd_forall(r.root, var.root); }

inline bdd bdd_appex(const bdd &l, const bdd &r, int op, const bdd &var)
{ return bdd_appex(l.root, r.root, op, var.root); }

inline bdd bdd_appall(const bdd &l, const bdd &r, int op, const bdd &var)
{ return bdd_appall(l.root, r.root, op, var.root); }

inline bdd bdd_support(const bdd &r)
{ return bdd_support(r.root); }

inline bdd bdd_satone(const bdd &r)
{ return bdd_satone(r.root); }

inline bdd bdd_fullsatone(const bdd &r)
{ return bdd_fullsatone(r.root); }

inline double bdd_satcount(const bdd &r)
{ return bdd_satcount(r.root); }

inline double bdd_satcountset(const bdd &r, const bdd &varset)
{ return bdd_satcountset(r.root, varset.root); }

inline double bdd_satcountln(const bdd &r)
{ return bdd_satcountln(r.root); }

inline double bdd_satcountlnset(const bdd &r, const bdd &varset)
{ return bdd_satcountlnset(r.root, varset.root); }

inline int bdd_nodecount(const bdd &r)
{ return bdd_nodecount(r.root); }

inline int* bdd_varprofile(const bdd &r)
{ return bdd_varprofile(r.root); }

   /* I/O extensions */

inline void bdd_fprinttable(FILE *file, const bdd &r)
{ bdd_fprinttable(file, r.root); }

inline void bdd_printtable(const bdd &r)
{ bdd_printtable(r.root); }

inline void bdd_fprintset(FILE *file, const bdd &r)
{ bdd_fprintset(file, r.root); }

inline void bdd_printset(const bdd &r)
{ bdd_printset(r.root); }

inline void bdd_printdot(const bdd &r)
{ bdd_printdot(r.root); }

inline void bdd_fprintdot(FILE* ofile, const bdd &r)
{ bdd_fprintdot(ofile, r.root); }

inline int bdd_fnprintdot(char* fname, const bdd &r)
{ return bdd_fnprintdot(fname, r.root); }

inline int bdd_fnsave(char *fname, const bdd &r)
{ return bdd_fnsave(fname, r.root); }

inline int bdd_save(FILE *ofile, const bdd &r)
{ return bdd_save(ofile, r.root); }

inline int bdd_fnload(char *fname, bdd &r)
{ int lr,e; e=bdd_fnload(fname, &lr); r=bdd(lr); return e; }

inline int bdd_load(FILE *ifile, bdd &r)
{ int lr,e; e=bdd_load(ifile, &lr); r=bdd(lr); return e; }

   /* FDD extensions */

inline bdd fdd_ithvarpp(int var, int val)
{ return fdd_ithvar(var, val); }

inline bdd fdd_ithsetpp(int var)
{ return fdd_ithset(var); }

inline bdd fdd_domainpp(int var)
{ return fdd_domain(var); }

inline bdd fdd_equalspp(int left, int right)
{ return fdd_equals(left, right); }

inline void fdd_printset(const bdd &r)
{ fdd_printset(r.root); }

inline void fdd_fprintset(FILE* ofile, const bdd &r)
{ fdd_fprintset(ofile, r.root); }

inline int fdd_scanset(const bdd &r, int *&v, int &n)
{ return fdd_scanset(r.root, &v, &n); }

inline bdd fdd_makesetpp(int *v, int n)
{ return fdd_makeset(v,n); }

inline int bdd_addvarblock(const bdd &v, int f)
{ return bdd_addvarblock(v.root, f); }


   /* Hack to allow for overloading */
#define bdd_ithvar bdd_ithvarpp
#define bdd_nithvar bdd_nithvarpp
#define bdd_makeset bdd_makesetpp
#define fdd_ithvar fdd_ithvarpp
#define fdd_ithset fdd_ithsetpp
#define fdd_domain fdd_domainpp
#define fdd_equals fdd_equalspp
#define fdd_makeset fdd_makesetpp

/*=== Inline C++ functions =============================================*/

inline int bdd::id(void) const
{ return root; }

inline bdd bdd::operator&(const bdd &r) const
{ return bdd_apply(*this,r,bddop_and); }

inline bdd bdd::operator&=(const bdd &r)
{ return (*this=bdd_apply(*this,r,bddop_and)); }

inline bdd bdd::operator^(const bdd &r) const
{ return bdd_apply(*this,r,bddop_xor); }

inline bdd bdd::operator^=(const bdd &r)
{ return (*this=bdd_apply(*this,r,bddop_xor)); }

inline bdd bdd::operator|(const bdd &r) const
{ return bdd_apply(*this,r,bddop_or); }

inline bdd bdd::operator|=(const bdd &r)
{ return (*this=bdd_apply(*this,r,bddop_or)); }

inline bdd bdd::operator!(void) const
{ return bdd_not(*this);}

inline bdd bdd::operator>>(const bdd &r) const
{ return bdd_apply(*this,r,bddop_imp); }

inline bdd bdd::operator>>=(const bdd &r)
{ return (*this=bdd_apply(*this,r,bddop_imp)); }

inline bdd bdd::operator-(const bdd &r) const
{ return bdd_apply(*this,r,bddop_diff); }

inline bdd bdd::operator-=(const bdd &r)
{ return (*this=bdd_apply(*this,r,bddop_diff)); }

inline bdd bdd::operator>(const bdd &r) const
{ return bdd_apply(*this,r,bddop_diff); }

inline bdd bdd::operator<(const bdd &r) const
{ return bdd_apply(*this,r,bddop_less); }

inline bdd bdd::operator<<(const bdd &r) const
{ return bdd_apply(*this,r,bddop_invimp); }

inline bdd bdd::operator<<=(const bdd &r)
{ return (*this=bdd_apply(*this,r,bddop_invimp)); }

inline int bdd::operator==(const bdd &r) const
{ return r.root==root; }

inline int bdd::operator!=(const bdd &r) const
{ return r.root!=root; }

inline bdd bdd_true(void)
{ return 1; }

inline bdd bdd_false(void)
{ return 0; }


/*=== Iostream printing ================================================*/

class bdd_ioformat
{
 public:
   bdd_ioformat(int f) { format=f; }
 private:
   bdd_ioformat(void)  { }
   int format;
   static int curformat;

   friend ostream &operator<<(ostream &, const bdd_ioformat &);
   friend ostream &operator<<(ostream &, const bdd &);
};

ostream &operator<<(ostream &, const bdd &);
ostream &operator<<(ostream &, const bdd_ioformat &);

extern bdd_ioformat bddset;
extern bdd_ioformat bddtable;
extern bdd_ioformat bdddot;
extern bdd_ioformat bddall;

#endif /* CPLUSPLUS */

#endif /* _BDD_H */

/* EOF */
