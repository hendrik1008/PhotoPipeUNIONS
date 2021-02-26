#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fitscat_defs.h"
#include "fitscat.h"
#include "parse_globals.h"
#include "utils_globals.h"
#include "utils_macros.h"

/*
   22.11.00:
   added sin and cos to the functions ldaccalc knows

   15.07.01:
   increased the depth of conditions from 10 to 100
   (10 was too small in one of my cases)

   20.04.2003:
   included the 'int' function for ldaccalc

   04.01.05:
   removed compiler warnings (gcc with -O3 optimisation)

   26.08.2010:
   I enable the possibility to use vector quantitites in ldacfilter
   conditions (NOTE: The procedure to do so probably
   crashes for vectors having more than 127 (max. number of signed
   characters) elements.
   NOTE: ldaccalc does not yet treat vectors correctly. It always
   creates a vector of full length, also if the calculations
   are done with individual vector columns!

   02.12.2011:
   I implemented powers with the '^' operator: x^a in the condition
   string calculates 'x' to the power of 'a'.

   23.01.2012:
   - I changed the tree_struct structure to better handle vector quantities
     in ldacfilter conditions. I adapted concerned code lines in this file.
   - I deleted the completely superfluous 'skip' function

   24.01.2012:
   Bug fix in function digit. The csign array needs to be one element larger
   (sprintf writes a trailing '\0').

   25.01.2012:
   Memory leak fix in functions get_char and init_condition
*/

#define OOPS_LEN 512
#define INCHAR_LEN 256

static char *regel;
static char oops[OOPS_LEN];
static char inchar[INCHAR_LEN];
static int iiii, err_set;

static int	*fieldsize, *field_id, nfields;
static char	**fields;

/*
 * Local function definitions
 */
static void get_char();
static void cond_error();
static void init_condition(char *, char **, int);
static void finish_condition();
static int reset_ident(tree_struct *, int, int);
static int check_condition();
static tree_struct *get_tree_elem();
static void copy_tree_elem(tree_struct *, tree_struct *);
static void test(char *,int ,int);
static int digit(tree_struct *);
static int number(tree_struct *);
static int ident(tree_struct *);
static int funct(tree_struct *);
static void factor(tree_struct *);
static void term(tree_struct *);
static void expr(tree_struct *);
static void condition(int, tree_struct *, char *);
static void accept_tree(tree_struct *);
double eval(tree_struct *,desc_struct *,rec_struct *);


static void get_char()
{
  int i;

  while (regel[iiii] == ' ') {
    iiii++;
  }

  i = 0;
  while (((iiii + i) < strlen(regel)) && (i < INCHAR_LEN-1)) {
    inchar[i] = regel[iiii+i];
    i++;
  }
  inchar[i] = '\0';
  iiii++;
}

static void cond_error()
{
  oops[iiii-1] = '^';
  err_set = 1;
}

static void init_condition(char *line, char **keynames, int nf)
{
  int i;

  err_set = 0;
  iiii = 0;

  for (i=0;i<OOPS_LEN-1;i++) {
    oops[i] = ' ';
  }
  oops[OOPS_LEN-1] = '\0';
  ED_MALLOC(fields,    "init_condition", char *, nf);
  ED_MALLOC(fieldsize, "init_condition", int,    nf);
  ED_MALLOC(field_id,  "init_condition", int,    nf);
  for (i = 0; i < nf; i++) {
    fieldsize[i] = strlen(keynames[i]);
    ED_CALLOC(fields[i],   "init_condition", char, fieldsize[i] + 1);
    strcpy(fields[i], keynames[i]);
    field_id[i] = -1;
  }
  nfields = nf;
  regel = line;
  get_char();
}

static int check_condition()
{
  int retcode = TRUE;

  if (inchar[0] != ';') {
    cond_error();
    retcode = FALSE;
  }

  if (err_set == 1) {
    syserr(WARNING,"check_condition",
           "Condition syntax cond_error:\n%s\n%s\n",regel,oops);
    retcode = FALSE;
  }

  return retcode;
}

static struct tree_elem *get_tree_elem()
{
  struct tree_elem *tr;

  ED_MALLOC(tr, "get_tree_elem", struct tree_elem, 1);
  tr->oper[0] = '\0';
  tr->type = UNKNOWN;
  tr->component = 0;
  tr->field[0] = '\0';
  tr->l = NULL;
  tr->r = NULL;

  return tr;
}

static void copy_tree_elem(tree_struct *dest, tree_struct *src)
{
  dest->l = src->l;
  dest->r = src->r;
  strcpy(dest->oper,src->oper);
  dest->type = src->type;

  switch (src->type) {
  case FIELD:   dest->par.fieldnum = src->par.fieldnum;
    break;
  case L_INT:   dest->par.i = src->par.i;
    break;
  case L_FLOAT: dest->par.f = src->par.f;
    break;
  }
  strcpy(dest->field,src->field);
}

static void test(char *c,int i,int k)
{
  int j;

  if (i == 1) {
    if (inchar[0] != c[0]) {
      cond_error();
    } else {
      get_char();
    }
  } else {
    if (strncmp(inchar, c, i) != 0) {
      cond_error();
    } else {
      for (j = 0; j < k; j++) {
        get_char();
      }
    }
  }
}

static int digit(tr)
struct tree_elem *tr;
{
  char str[2], cdig[11];
  int retcode;
  int i;

  retcode = -1;

  for (i = 0; i <= 9; i++) {
    sprintf(&cdig[i], "%1i", i);
    if (inchar[0] == cdig[i]) {
      str[0] = cdig[i];
      str[1] = '\0';
      test(str,1,1);
      strcat(tr->field,str);
      retcode = 0;
    }
  }
  return retcode;
}

static int number(tr)
struct tree_elem *tr;
{
  int retcode;

  retcode = 0;
  tr->type = L_INT;
  if (inchar[0] == '+') {
    test("+",1,1);
    strcat(tr->field,"+");
    if (digit(tr) != 0) {
      cond_error();
      retcode = -1;
    }
    while (digit(tr) == 0) ;
  } else {
    if (inchar[0] == '-') {
      test("-",1,1);
      strcat(tr->field,"-");
      if (digit(tr) != 0) {
        cond_error();
        retcode = -1;
      }
      while (digit(tr) == 0) ;
    } else {
      if (digit(tr) != 0) {
        cond_error();
        retcode = -1;
      }
      while (digit(tr) == 0) ;
    }
  }

  if (inchar[0] == '.') {
    test(".",1,1);
    tr->type = L_FLOAT;
    strcat(tr->field,".");
    while (digit(tr) == 0) ;
  }
  return retcode;
}

static int ident(tr)
struct tree_elem *tr;
{
  int retcode, i, j, length, *found, longest, num;
  char temp_field[MAXCHAR], temp_num[MAXCHAR], *t;

  ED_MALLOC(found, "ident", int, nfields);
  retcode = -1;
  length = 0;

  for (i = 0; i < nfields; i++) {
    found[i] = 0;

    for (j = 0; j < fieldsize[i]; j++) {
      temp_field[j] = fields[i][j];
    }
    temp_field[fieldsize[i]] = '\0';

    if (strncmp(inchar, temp_field,fieldsize[i]) == 0) {
      found[i] = 1;

      if (length < (int)strlen(fields[i])) {
        length = strlen(fields[i]);
      }
    }
  }
  longest = -1;
  j = -1;

  for (i = 0; i < nfields; i++) {
    if (found[i] == 1) {
      if (longest < fieldsize[i]) {
        longest = fieldsize[i];
        j = i;
      }
    }
  }

  for (i = 0; i < nfields; i++) {
    if (found[i] == 1 && j == i) {
      if (length == (int)strlen(fields[i])) {
        for (j = 0; j < fieldsize[i]; j++) {
          temp_field[j] = fields[i][j];
        }
        temp_field[fieldsize[i]] = '\0';
        test(temp_field,fieldsize[i],fieldsize[i]);
        tr->type = FIELD;
        strcpy(tr->field, fields[i]);
        field_id[i] = i;
        tr->par.fieldnum = i;
/*
 *      Allow for the naming of keyword column numbers in vector
 *      situations
 */
        if (inchar[0] == '(') {
          test("(",1,1);
          strcpy(temp_num, inchar);

          if ((t=strtok(temp_num,")"))==NULL) {
            cond_error();
          }
          num = atoi(t) - 1;
          tr->component = num;

          while (inchar[0] != ')') {
            get_char();
          }
          get_char();
        }
        retcode = 0;
      }
    }
  }
  ED_FREE(found);

  return retcode;
}

static int funct(tr)
struct tree_elem *tr;
{
  struct tree_elem *tn1;
  int retcode;

  retcode = 0;
  if (strncmp(inchar,"exp",3) == 0) {
    test("exp",3,3);
    strcpy(tr->oper,"exp");
    tn1 = get_tree_elem();
    tr->l = tn1;
  } else {
    if (strncmp(inchar,"log",3) == 0) {
      test("log",3,3);
      strcpy(tr->oper,"log");
      tn1 = get_tree_elem();
      tr->l = tn1;
    } else {
      if (strncmp(inchar,"abs",3) == 0) {
        test("abs",3,3);
        strcpy(tr->oper,"abs");
        tn1 = get_tree_elem();
        tr->l = tn1;
      } else {
        if (strncmp(inchar,"sqrt",3) == 0) {
          test("sqrt",4,4);
          strcpy(tr->oper,"sqrt");
          tn1 = get_tree_elem();
          tr->l = tn1;
        } else {
          if (strncmp(inchar,"sin",3) == 0) {
            test("sin",3,3);
            strcpy(tr->oper,"sin");
            tn1 = get_tree_elem();
            tr->l = tn1;
          } else {
            if (strncmp(inchar,"cos",3) == 0) {
              test("cos",3,3);
              strcpy(tr->oper,"cos");
              tn1 = get_tree_elem();
              tr->l = tn1;
            } else {		
              if (strncmp(inchar,"int",3) == 0) {
                test("int",3,3);
                strcpy(tr->oper,"int");
                tn1 = get_tree_elem();
                tr->l = tn1;
              } else {
                retcode = -1;
              }
            }
          }
        }
      }
    }
  }
  if (strcmp(tr->oper,"") != 0) {
    test("(",1,1);
    expr(tr->l);
    test(")",1,1);
  }

  return retcode;
}

static void factor(tr)
struct tree_elem *tr;
{
  if (ident(tr) != 0) {
    if (inchar[0] == '(') {
      test("(",1,1);
      expr(tr);
      test(")",1,1);
    } else {
      if (funct(tr) == 0) {
      } else {
        if (number(tr) != 0) {
          cond_error();
        }
      }
    }
  }
}

static void term(tr)
struct tree_elem *tr;
{
  struct tree_elem *tn1,*tn2;
  factor(tr);
  if (inchar[0] == '*') {
    test("*",1,1);
    tn1 = get_tree_elem();
    tn2 = get_tree_elem();
    copy_tree_elem(tn1,tr);
    tr->l = tn1; tr->r = tn2;
    strcpy(tr->oper,"*");
    tr->type = UNKNOWN;
    strcpy(tr->field,"");
    factor(tn2);
  } else {
    if (inchar[0] == '/') {
      test("/",1,1);
      tn1 = get_tree_elem();
      tn2 = get_tree_elem();
      copy_tree_elem(tn1,tr);
      tr->l = tn1; tr->r = tn2;
      strcpy(tr->oper,"/");
      tr->type = UNKNOWN;
      strcpy(tr->field,"");
      factor(tn2);
    } else {
      if (inchar[0] == '^') {
        test("^",1,1);
        tn1 = get_tree_elem();
        tn2 = get_tree_elem();
        copy_tree_elem(tn1,tr);
        tr->l = tn1; tr->r = tn2;
        strcpy(tr->oper,"^");
        tr->type = UNKNOWN;
        strcpy(tr->field,"");
        factor(tn2);
      } else {
        if (inchar[0] != ')' && inchar[0] != '+' && inchar[0] != '-' &&
            inchar[0] != ';' && inchar[0] != 'A' && inchar[0] != 'O') {
          cond_error();
        }
      }
    }
  }
}

static void expr(tr)
struct tree_elem *tr;
{
  struct tree_elem *tn1,*tn2;

  term(tr);
  while (inchar[0] == '+' || inchar[0] == '-') {
    if (inchar[0] == '+') {
      test("+",1,1);
      tn1 = get_tree_elem();
      tn2 = get_tree_elem();
      copy_tree_elem(tn1,tr);
      tr->l = tn1; tr->r = tn2;
      strcpy(tr->oper,"+");
      tr->type = UNKNOWN;
      strcpy(tr->field,"");
      term(tn2);
    } else {
      if (inchar[0] == '-') {
        test("-",1,1);
        tn1 = get_tree_elem();
        tn2 = get_tree_elem();
        copy_tree_elem(tn1,tr);
        tr->l = tn1; tr->r = tn2;
        strcpy(tr->oper,"-");
        tr->type = UNKNOWN;
        strcpy(tr->field,"");
        term(tn2);
      } else {
        cond_error();
      }
    }
  }
}

static void condition(depth, tr, type)
int depth;
struct tree_elem *tr;
char *type;
{
  struct tree_elem *tn;

  depth++;
  if (depth > 100) {
    syserr(FATAL, "condition", "Too many levels in condition\n");
  } else {
    if (strcmp(type, "expression") == 0) {
      expr(tr);
    } else {
      tn = get_tree_elem();
      tr->l = tn;
      if (inchar[0] == '(' || (number(tn) != 0 && ident(tn) != 0)) {
        err_set = 0;
        if (inchar[0] == '(') {
          test("(", 1, 1);
          condition(depth, tn, type);
          test(")", 1, 1);
          if (strncmp(inchar, "AND", 3) == 0) {
            test("AND", 3, 3);
            test("(", 1, 1);
            tn = get_tree_elem();
            tr->r = tn;
            strcpy(tr->oper, "AND");
            tr->type = UNKNOWN;
            condition(depth, tn, type);
            test(")",1,1);
          } else {
            if (strncmp(inchar, "OR", 2) == 0) {
              test("OR", 2, 2);
              test("(", 1, 1);
              tn = get_tree_elem();
              tr->r = tn;
              strcpy(tr->oper, "OR");
              tr->type = UNKNOWN;
              condition(depth, tn, type);
              test(")", 1, 1);
            } else {
              if (inchar[0] != ';') {
                cond_error();
              }
            }
          }
        } else {
          if (funct(tr) != 0) {
            cond_error();
          }
        }
      } else {
        err_set = 0;
        if (inchar[0] == '=') {
          test("=", 1, 1);
          strcpy(tr->oper, "=");
          tr->type = UNKNOWN;
          tn = get_tree_elem();
          tr->r = tn;
          expr(tn);
        } else {
          if (strncmp(inchar, "<=", 2) == 0) {
            test("<=", 2, 2);
            strcpy(tr->oper, "<=");
            tr->type = UNKNOWN;
            tn = get_tree_elem();
            tr->r = tn;
            expr(tn);
          } else {
            if (strncmp(inchar, ">=", 2) == 0) {
              strcpy(tr->oper, ">=");
              tr->type = UNKNOWN;
              test(">=", 2, 2);
              tn = get_tree_elem();
              tr->r = tn;
              expr(tn);
            } else {
              if (strncmp(inchar, "!=", 2) == 0) {
                strcpy(tr->oper, "!=");
                tr->type = UNKNOWN;
                test("!=", 2, 2);
                tn = get_tree_elem();
                tr->r = tn;
                expr(tn);
              } else {
                if (inchar[0] == '>') {
                  test(">", 1, 1);
                  strcpy(tr->oper, ">");
                  tr->type = UNKNOWN;
                  tn = get_tree_elem();
                  tr->r = tn;
                  expr(tn);
                } else {
                  if (inchar[0] == '<') {
                    test("<", 1, 1);
                    tn = get_tree_elem();
                    tr->r = tn;
                    strcpy(tr->oper, "<");
                    tr->type = UNKNOWN;
                    expr(tn);
                  } else {
                    cond_error();
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}


static void accept_tree(tr)
struct tree_elem *tr;
{
  if (tr->l != NULL) {
    accept_tree(tr->l);
  }
  if (tr->r != NULL) {
    accept_tree(tr->r);
  }
  if (tr->type != FIELD) {
    if (tr->type == L_INT) {
      tr->par.i = atoi(tr->field);
    }
    if (tr->type == L_FLOAT) {
      tr->par.f = atof(tr->field);
    }
  }
}

tree_struct *tree;

tree_struct *parse_line(char *line, char **keynames, int nkeys, char *type)
{
  init_condition(line, keynames, nkeys);
  tree = get_tree_elem();
  condition(0,tree, type);
  if (check_condition()) {
    accept_tree(tree);
    return tree;
  } else {
    return NULL;
  }
}

void set_vec_col(tree_struct *tr, int n)
{
  if (tr->l != NULL) {
    set_vec_col(tr->l, n);
  }

  if (tr->r != NULL) {
    set_vec_col(tr->r, n);
  }

  if (tr->type == FIELD) {
    tr->component = n;
  }
}

double eval(tr,record,data)
tree_struct *tr;
desc_struct *record;
rec_struct *data;
{
  double A=0.0;
  char t[2];
  int  col;

  if (strcmp(tr->oper, "") == 0) {
    switch(tr->type) {
    case FIELD:
      col = tr->component;

      if (col < 0 || col >= record[tr->par.fieldnum].depth) {
        syserr(FATAL, "eval", "Vector column number out of bounds\n");
      }

      switch (record[tr->par.fieldnum].type) {
      case T_SHORT:
        A = (float) data[tr->par.fieldnum].i[col];
        break;
      case T_LONG:
        A = (float) data[tr->par.fieldnum].l[col];
        break;
      case T_FLOAT:
        A = (double) data[tr->par.fieldnum].f[col];
        break;
      case T_DOUBLE:
        A = (double) data[tr->par.fieldnum].d[col];
        break;
      case T_BYTE:
        t[0] = data[tr->par.fieldnum].b[col];
        t[1] = '\0';
        A = atof(t);
        break;
      case T_STRING:
        break;
      };
      break;
    case L_INT:
      A = (double) tr->par.i;
      break;
    case L_FLOAT:
      A = tr->par.f;
      break;
    }
  } else {
    if (strcmp(tr->oper, "+") == 0) {
      A = eval(tr->l, record, data) + eval(tr->r, record, data);
    } else {
      if (strcmp(tr->oper, "-") == 0) {
        A = eval(tr->l, record,data) - eval(tr->r, record, data);
      } else {
        if (strcmp(tr->oper, "/") == 0) {
          A = eval(tr->l, record, data) / eval(tr->r, record, data);
        } else {
          if (strcmp(tr->oper, "*") == 0) {
            A = eval(tr->l, record, data) * eval(tr->r, record, data);
          } else {
            if (strcmp(tr->oper, "^") == 0) {
              A = pow(eval(tr->l, record, data),
                      eval(tr->r, record, data));
            } else {
              if (strcmp(tr->oper, "log") == 0) {
                A = log(eval(tr->l, record, data));
              } else {
                if (strcmp(tr->oper, "abs") == 0) {
                  A = fabs(eval(tr->l, record, data));
                } else {
                  if (strcmp(tr->oper, "exp") == 0) {
                    A = exp(eval(tr->l, record, data));
                  } else {
                    if (strcmp(tr->oper, "sqrt") == 0) {
                      A = sqrt(eval(tr->l, record, data));
                    } else {
                      if (strcmp(tr->oper, "sin") == 0) {
                        A = sin(eval(tr->l, record, data));
                      } else {
                        if (strcmp(tr->oper, "cos") == 0) {
                          A = cos(eval(tr->l, record, data));
                        } else {
                          if(strcmp(tr->oper, "int") == 0) {
                            A = (int)(eval(tr->l, record, data));
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return A;
}

/**/

int question(tr,record,data)
tree_struct *tr;
desc_struct *record;
rec_struct *data;
{
  int retcode;
  double A,B;

  retcode = 0;
  while (strcmp(tr->oper,"") == 0 && tr->l != NULL) {
    tr = tr->l;
  }

  if (strcmp(tr->oper,"") != 0) {
    if (strcmp(tr->oper,"AND") == 0) {
      if (question(tr->l, record,data) && question(tr->r, record,data)) {
        retcode = 1;
      }
    } else {
      if (strcmp(tr->oper,"OR") == 0) {
        if (question(tr->l, record,data) || question(tr->r, record,data)) {
          retcode = 1;
        }
      } else {
        A = eval(tr->l, record,data);
        B = eval(tr->r, record,data);

        if (strcmp(tr->oper, "=") == 0) {
          if (A == B) {
            retcode = 1;
          }
        } else {
          if (strcmp(tr->oper, "<=") == 0) {
            if (A <= B) {
              retcode = 1;
            }
          } else {
            if (strcmp(tr->oper, ">=") == 0) {
              if (A >= B) {
                retcode = 1;
              }
            } else {
              if (strcmp(tr->oper, "<") == 0) {
                if (A < B) {
                  retcode = 1;
                }
              } else {
                if (strcmp(tr->oper, ">") == 0) {
                  if (A > B) {
                    retcode = 1;
                  }
                } else {
                  if (strcmp(tr->oper, "!=") == 0) {
                    if (A != B) {
                      retcode = 1;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  } else {
/* if the tree is empty, the condition is always true */
    retcode = 1;
  }

  return retcode;
}

static void finish_condition()
{
  int i;

  for(i = 0; i < nfields; i++) {
    ED_FREE(fields[i]);
  }

  ED_FREE(field_id);
  ED_FREE(fieldsize);
  ED_FREE(fields);
}


static int reset_ident(tree_struct *tr, int i, int num)
{
  int retcode=0;

  if (tr->l != NULL) {
    retcode = reset_ident(tr->l, i, num);
  }

  if (tr->r != NULL) {
    retcode = reset_ident(tr->r, i, num);
  }

  if (tr->type == FIELD) {
    if (tr->par.i == i)  {
      tr->par.i = num;
      retcode = tr->component;
    }
  }

  return retcode;
}

char **find_idents(tree_struct *tr, int *n)
{
  int i, col, num = 0;
  char **s;

  ED_MALLOC(s, "find_idents", char *, 1);

  for (i = 0; i < nfields; i++) {
    if (field_id[i] >= 0) {
      if (num > 0) {
        ED_REALLOC(s,  "find_idents", char *, (num + 1));
      }
      ED_MALLOC(s[num], "find_idents", char,   MAXCHAR);
      strcpy(s[num], fields[i]);

      if ((col = reset_ident(tr, i, num)) != 0) {
        s[num][fieldsize[i] + 1] = col;
      }
      num++;
    }
  }

  *n = num;
  finish_condition();

  return s;
}
