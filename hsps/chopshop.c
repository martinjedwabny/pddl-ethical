#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* translation options */

#define NO_REPEAT
#define TIME_SCALE  1
/* #define CHECK_AT_START */

#define WRITE_DOT_FILE

/* define max project size */

#define MAX_N_JOBS  20
#define MAX_N_MODES 10
#define MAX_T_RES   10

/* project info (read from .mm file, used to write PDDL files). */

int no_of_jobs = 0;
int n_renewable = 0;
int n_consumable = 0;

int n_modes[MAX_N_JOBS];
int prec[MAX_N_JOBS][MAX_N_JOBS];
int dur[MAX_N_JOBS][MAX_N_MODES];
int res[MAX_N_JOBS][MAX_N_MODES][MAX_T_RES];
int avail[MAX_T_RES];


/* some util functions */

char* skip_n_words(char* p, int n) {
  while (n > 0) {
    while (*p == ' ') p++;
    if (*p == '\0') return 0;
    while ((*p != ' ') && (*p != '\0')) p++;
    n -= 1;
  }
  if (*p == '\0') return 0;
  else return p;
}

char* get_int(char* p, int* v) {
  while ((*p == ' ') || (*p == '\n')) p++;
  if (*p == '\0') return 0;
  *v = atoi(p);
  while ((*p != ' ') && (*p != '\0')) p++;
  return p;
}

int get_int_vector(char* p, int v[]) {
  int i = 0;
  p = get_int(p, &(v[i]));
  while (p) {
    i += 1;
    p = get_int(p, &(v[i]));
  }
  return i;
}

int sum(int v[], int n0, int n) {
  int k, t = 0;
  for (k = n0; k < n; k++) t += v[k];
  return t;
}

void clear_project_info() {
  int i, j, m, r;
  for (j = 0; j < MAX_N_JOBS; j++) {
    n_modes[j] = 0;
    for (i = 0; i < MAX_N_JOBS; i++) prec[i][j] = 0;
    for (m = 0; m < MAX_N_MODES; m++) {
      dur[j][m] = 0;
      for (r = 0; r < MAX_T_RES; r++) res[j][m][r] = 0;
    }
  }
  for (r = 0; r < MAX_T_RES; r++) avail[r] = 0;
  no_of_jobs = 0;
  n_renewable = 0;
  n_consumable = 0;
}

/* The chop function parses project files in .mm format. Input is
 * an open (readable) file. */

void chop(FILE* f) {
  char  l[250];
  int mode, k, i, j, v0, v1, v2;
  int iv[MAX_T_RES + 3];
  char* p;
  mode = 0;
  while (!feof(f)) {
    fgets(l, 249, f);
    switch (mode) {
    case 0:
      if (strncmp(l, "PROJECT INFORMATION", 19) == 0)
	mode = 11;
      else if (strncmp(l, "PRECEDENCE RELATIONS", 20) == 0)
	mode = 21;
      else if (strncmp(l, "REQUESTS/DURATIONS", 18) == 0)
	mode = 31;
      else if (strncmp(l, "RESOURCEAVAILABILITIES", 22) == 0)
	mode = 41;
      else if (strncmp(l, "  - renewable", 13) == 0) {
	p = strchr(l, ':');
	if (p == NULL) {
	  printf("error: can't find ':' in:\n%s", l);
	  exit(1);
	}
	if (sscanf(p + 1, "%d", &n_renewable) < 1) {
	  printf("error: can't find integer in:\n%s", l);
	  exit(1);
	}
	if (n_renewable + n_consumable > MAX_T_RES) {
	  printf("error: too many resources (%d R + %d N)",
		n_renewable, n_consumable);
	  exit(1);
	}
	printf("%d renewable resources\n", n_renewable);
      }
      else if (strncmp(l, "  - nonrenewable", 16) == 0) {
	p = strchr(l, ':');
	if (p == NULL) {
	  printf("error: can't find ':' in:\n%s", l);
	  exit(1);
	}
	if (sscanf(p + 1, "%d", &n_consumable) < 1) {
	  printf("error: can't find integer in:\n%s", l);
	  exit(1);
	}
	if (n_renewable + n_consumable > MAX_T_RES) {
	  printf("error: too many resources (%d R + %d N)",
		 n_renewable, n_consumable);
	  exit(1);
	}
	printf("%d consumable resources\n", n_consumable);
      }
      break;
    case 11: /* project info */
      mode = 12;
      break;
    case 12:
      if (sscanf(l, "%d%d", &v0, &no_of_jobs) < 2) {
	printf("error: can't find #jobs in line:\n%s", l);
	exit(1);
      }
      if (no_of_jobs > MAX_N_JOBS) {
	printf("error: project has too many jobs (%d)\n", no_of_jobs);
	exit(1);
      }
      printf("# of jobs: %d\n", no_of_jobs);
      mode = 0;
      break;
    case 21: /* precedence relation */
      mode = 22;
      break;
    case 22:
      if (l[0] == '*') mode = 0;
      else if (sscanf(l, "%d%d%d", &k, &v0, &v1) == 3) {
	if ((k > 1) && (k <= no_of_jobs + 1)) {
	  j = k - 1;
	  n_modes[j - 1] = v0;
	  printf("J%d has %d modes and %d successors\n", j, v0, v1);
	  p = skip_n_words(l, 3);
	  for (k = 0; k < v1; k++) {
	    if (p == NULL) {
	      printf("error: can't find %d successors in line:\n%s", v1, l);
	      exit(1);
	    }
	    p = get_int(p, &v2);
	    if ((v2 < 2) || (v2 > no_of_jobs + 1)) {
	      printf("(ignoring successor id %d)\n", v2);
	    }
	    else {
	      printf("prec(J%d): J%d\n", v2 - 1, j);
	      prec[v2 - 2][j - 1] = 1;
	    }
	  }
	}
      }
      else {
	printf("error: terminating line (***) not found:\n%s", l);
	exit(1);
      }
      break;
    case 31: /* resource cons. & duration */
      mode = 32;
      break;
    case 32:
      j = -1;
      mode = 33;
      break;
    case 33:
      if (l[0] == '*') mode = 0;
      else {
	i = get_int_vector(l, iv);
	if (i < 2 + n_renewable + n_consumable) {
	  printf("error: terminating line (***) not found:\n%s", l);
	  exit(1);
	}
	if (i == 3 + n_renewable + n_consumable) {
	  j = iv[0] - 1;
	  for (k = 0; k < i - 1; k++) iv[k] = iv[k+1];
	}
	if ((j >= 1) && (j <= no_of_jobs)) {
	  if ((iv[0] < 1) || (iv[0] > n_modes[j - 1])) {
	    printf("error: J%d has no mode %d\n", j, iv[0]);
	    exit(1);
	  }
	  dur[j-1][iv[0]-1] = iv[1];
	  printf("J%d/M%d: dur = %d", j, iv[0], iv[1]);
	  for (k = 0; k < n_renewable + n_consumable; k++) {
	    res[j-1][iv[0]-1][k] = iv[k+2];
	    if (k < n_renewable) printf(", R%d = %d", k + 1, iv[k+2]);
	    else printf(", F%d = %d", (k - n_renewable) + 1, iv[k+2]);
	  }
	  printf("\n");
	}
      }
      break;
    case 41: /* resources available */
      mode = 42;
      break;
    case 42:
      i = get_int_vector(l, iv);
      if (i < n_renewable + n_consumable) {
	printf("error: can't find %d integers in line:\n%s",
	       n_renewable + n_consumable, l);
	exit(1);
      }
      for (k = 0; k < n_renewable + n_consumable; k++) {
	avail[k] = iv[k];
	if (k < n_renewable) printf("avail R%d = %d\n", k + 1, iv[k]);
	else printf("avail F%d = %d\n", (k - n_renewable) + 1, iv[k]);
      }
      printf("\n");
      mode = 0;
      break;
    }
  }
}

/* mk_domain and mk_problem writes a PDDL domain/problem for the current
 * project info. Input to both are open (writable) files. */

void mk_domain(FILE* f, char* name) {
  int i, j, n_pc, n_rc, m, r;
  fprintf(f, "(define (domain MRCPS)\n");
  fprintf(f, "(:requirements :strips");
#ifdef NO_REPEAT
  fprintf(f, " :negative-preconditions");
#endif
  fprintf(f, " :fluents :durative-actions)\n");
  fprintf(f, "(:predicates");
  for (j = 0; j < no_of_jobs; j++) fprintf(f, " (J%d)", j + 1);
  fprintf(f, ")\n");
  if (n_renewable + n_consumable > 0) {
    fprintf(f, "(:functions");
    for (r = 0; r < n_renewable; r++) fprintf(f, " (R%d)", r + 1);
    for (r = 0; r < n_consumable; r++) fprintf(f, " (F%d)", r + 1);
    fprintf(f, ")\n");
  }
  fprintf(f, "\n");
  for (j = 0; j < no_of_jobs; j++)
    for (m = 0; m < n_modes[j]; m++) {
      fprintf(f, "(:durative-action J%d_M%d\n", j + 1, m + 1);
      fprintf(f, " :parameters ()\n");
      if (dur[j][m] <= 0) {
	printf("warning: J%d/M%d has duration %d <= 0\n", j, m, dur[j][m]);
      }
      fprintf(f, " :duration (= ?duration %d)\n", TIME_SCALE*dur[j][m]);
      n_pc = sum(prec[j], 0, MAX_N_JOBS);
#ifdef NO_REPEAT
      n_pc += 1;
#endif
      n_rc = (sum(res[j][m], 0, n_renewable) +
	      sum(res[j][m], n_renewable, n_renewable + n_consumable));
      if ((n_pc + n_rc) > 0) {
	fprintf(f, " :condition");
	if ((n_pc + n_rc) > 1) fprintf(f, "  (and");
#ifdef NO_REPEAT
	fprintf(f, " (at start (not (J%d)))", j + 1);
#endif
	for (i = 0; i < no_of_jobs; i++) if (prec[j][i])
	  fprintf(f, " (at start (J%d))", i + 1);
#ifdef CHECK_AT_START
	for (r = 0; r < n_renewable; r++) if (res[j][m][r])
	  fprintf(f, " (at start (>= (R%d) %d))", r + 1, res[j][m][r]);
	for (r = 0; r < n_consumable; r++) if (res[j][m][r+n_renewable])
	  fprintf(f, " (at start (>= (F%d) %d))", r + 1,
		  res[j][m][r+n_renewable]);
#else
	for (r = 0; r < n_renewable; r++) if (res[j][m][r])
	  fprintf(f, " (over all (>= (R%d) 0))", r + 1);
	for (r = 0; r < n_consumable; r++) if (res[j][m][r+n_renewable])
	  fprintf(f, " (over all (>= (F%d) 0))", r + 1);
#endif
	if ((n_pc + n_rc) > 1) fprintf(f, ")");
	fprintf(f, "\n");
      }
      fprintf(f, " :effect (and (at end (J%d))", j + 1);
      for (r = 0; r < n_renewable; r++) if (res[j][m][r]) {
	fprintf(f, " (at start (decrease (R%d) %d.0))", r + 1, res[j][m][r]);
	fprintf(f, " (at end (increase (R%d) %d.0))", r + 1, res[j][m][r]);
      }
      for (r = 0; r < n_consumable; r++) if (res[j][m][r+n_renewable])
	fprintf(f, " (at start (decrease (F%d) %d.0))",
		r + 1, res[j][m][r+n_renewable]);
      fprintf(f, "))\n");
    }
  fprintf(f, ")\n");
}

void mk_problem(FILE* f, char* name) {
  int j, r;

  fprintf(f, "(define (problem %s)\n", name);
  fprintf(f, "(:domain MRCPS)\n");
  if (n_renewable + n_consumable > 0) {
    fprintf(f, "(:init");
    for (r = 0; r < n_renewable; r++)
      fprintf(f, " (= (R%d) %d.0)", r + 1, avail[r]);
    for (r = 0; r < n_consumable; r++) 
      fprintf(f, " (= (F%d) %d.0)", r + 1, avail[r+n_renewable]);
    fprintf(f, ")\n");
  }
  fprintf(f, "(:goal (and");
  for (j = 0; j < no_of_jobs; j++) fprintf(f, " (J%d)", j + 1);
  fprintf(f, "))\n");
  fprintf(f, "(:metric minimize (total-time)))\n");
}

void mk_graph(FILE* f, char* name) {
  int i, j, m, r;

  fprintf(f, "digraph %s {\n", name);
  fprintf(f, "rankdir=LR;\n");
  for (j = 0; j < no_of_jobs; j++) {
    fprintf(f, "J%d [shape=record,width=0,height=0,label=\"{J%d|{", j + 1, j+1);
    for (m = 0; m < n_modes[j]; m++) {
      fprintf(f, "M%d (d=%d", m + 1, dur[j][m]);
      for (r = 0; r < n_renewable; r++)
	if (res[j][m][r] > 0) {
	  fprintf(f, ",R%d:%d", r + 1, res[j][m][r]);
	}
      for (r = 0; r < n_consumable; r++)
	if (res[j][m][r+n_renewable] > 0) {
	  fprintf(f, ",F%d:%d", r + 1, res[j][m][r+n_renewable]);
	}
      fprintf(f, ")");
      if (m < n_modes[j] - 1) fprintf(f, "|");
    }
    fprintf(f, "}}\"];\n");
  }

  for (j = 0; j < no_of_jobs; j++)
    for (i = 0; i < no_of_jobs; i++)
      if (prec[j][i]) fprintf(f, "J%d -> J%d;\n", i + 1, j + 1);

  fprintf(f, "}\n");
}

/* functions that open/close files around calls to chop, mk_domain
 * and mk_problem. */

void chop_file(char* fname) {
  FILE* f;
  f = fopen(fname, "r");
  if (f == NULL) {
    printf("error: failed to open %s\n", fname);
    exit(1);
  }
  chop(f);
  close(f);
}

void write_PDDL_files(char* pname) {
  char *dfname, *pfname;
  FILE* f;
  dfname = malloc(strlen(pname)+15);
  strcpy(dfname, pname);
  strcat(dfname, "_domain.pddl");
  f = fopen(dfname, "w");
  if (f == NULL) {
    printf("error: failed to open %s\n", dfname);
    exit(1);
  }
  printf("writing project %s to %s...\n", pname, dfname);
  mk_domain(f, pname);
  close(f);
  pfname = malloc(strlen(pname)+15);
  strcpy(pfname, pname);
  strcat(pfname, ".pddl");
  f = fopen(pfname, "w");
  if (f == NULL) {
    printf("error: failed to open %s\n", pfname);
    exit(1);
  }
  printf("writing project %s to %s...\n", pname, pfname);
  mk_problem(f, pname);
  close(f);
}

void write_DOT_file(char* pname) {
  char *fname;
  FILE* f;
  fname = malloc(strlen(pname)+15);
  strcpy(fname, pname);
  strcat(fname, ".dot");
  f = fopen(fname, "w");
  if (f == NULL) {
    printf("error: failed to open %s\n", fname);
    exit(1);
  }
  printf("writing graph to %s...\n", fname);
  mk_graph(f, pname);
  close(f);
}

int main(int argc, char* argv[]) {
  char *pname = 0, *p;
  if (argc > 1) {
    clear_project_info();
    chop_file(argv[1]);
    if (argc > 2) {
      write_PDDL_files(argv[2]);
#ifdef WRITE_DOT_FILE
      write_DOT_file(argv[2]);
#endif
    }
    else {
      pname = strdup(argv[1]);
      p = strrchr(pname, '.');
      if (p) *p = '\0';
      p = strrchr(pname, '/');
      if (p) pname = p + 1;
      write_PDDL_files(pname);
#ifdef WRITE_DOT_FILE
      write_DOT_file(pname);
#endif
    }
  }
  else {
    printf("%s <MRCPS File> [project name]\n", argv[0]);
  }
  return 0;
}
