#ifndef _ZAPP_H
#define _ZAPP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>

#undef DEBUG
#define ENABLE_DEBUG 0

#if(ENABLE_DEBUG == 1)
#define DEBUG
#endif // if(ENABLE_DEBUG == 1)

#define IS_CHAR(c) (((unsigned int)c | (1 << 5)) - 'a' <= 'z' - 'a')
#define IS_SPACE(c) (c == ' ' || c == '\t' || c == '\n')

#define ARG_TBUF_FILLED 0x1
#define ARG_COMPILE 0x2
#define ARG_PRINT_TREE 0x4

/*
 * tokenize
 */

typedef enum {
  TY_INT,
  TY_FLOAT
} type_kind;

struct type {
  type_kind kind;
};

typedef enum {
  TOKEN_NUM,
  TOKEN_FNUM,
  TOKEN_PUNCT,
  TOKEN_IDENT,
  TOKEN_EOF
} token_kind;

struct tokenizer {
  char *buf;
  char *cur;
  char *tok_start;
  char *tok_end;
  int nline;
  int nrow;
};

struct token {
  struct token *next;
  token_kind kind;
  struct type *type;
  char *start;
  int len;
  int nline;
  int nrow;
};

void tokenizer_init(struct tokenizer *tokenizer, char *buf);
struct token *tokenize(struct tokenizer *tokenizer);
int tok_equals(struct token *tok, const char *s);
void tok_skip(struct token **tok, const char *s);
struct token *tok_consume(struct token *tok, const char *s);

/*
 * parse
 */

typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_LT,     // <
  ND_LTE,    // <=
  ND_EQ,     // ==
  ND_NEQ,    // !=
  ND_NEG,    // unary "-"
  ND_ASSIGN, // =
  ND_FOR,    // for loop
  ND_IF,     // if condition
  // TODO: ND_PRINT is temporary solution, proper solution would be to
  // replace ND_PRINT with regular fucntion calls
  ND_PRINT,  // print following experssion to stdout
  ND_BLOCK,  // sequence of statements (for instance, in `if` clause)
  ND_NUM,    // integer value
  ND_VAR     // variable
} node_kind;

union actual_value {
  int num;        // integer value if `kind` is ND_NUM
  double fnum;    // integer value if `kind` is ND_NUM
  char *str;
};

struct zapp_value {
  struct type *type;
  union actual_value val;
};

struct var {
  char *name;
};

struct node {
  node_kind kind;
  struct node *next;
  struct node *lhs;
  struct node *rhs;
  struct token *tok;
  struct type *type;
  union actual_value val;
  struct var var; // used if `kind` is ND_VAR

  // These three are used for conditional jumps
  struct node *cond;
  struct node *then;
  struct node *els;

  // For loops
  struct node *init;
  struct node *inc;

  // Sequeunce of statements
  struct node *body;
};

struct node *expr(struct token **rest, struct token *tok);
struct node *parse(struct token *tokens);

/*
 * misc
 */

_Noreturn void panic(const char *fmt, ...);

_Noreturn void panic_tok(struct token *tok, const char *fmt, ...);

/*
 * ast
 */

double eval_node(struct node *node);
void execute_node(struct node *node);
void print_node_tree(struct node *node);

/*
 * c_codegen
 */

void c_codegen(struct node *prog, FILE *fp);

#endif // _ZAPP_H
