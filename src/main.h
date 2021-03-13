#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define IS_CHAR(c) (((unsigned int)c | (1 << 5)) - 'a' <= 'z' - 'a')

/*
 * tokenize
 */

typedef enum {
  TOKEN_NUM,
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
  char *start;
  int len;
  int nline;
  int nrow;
};

void tokenizer_init(struct tokenizer *tokenizer, char *buf);
struct token *tokenize(struct tokenizer *tokenizer);
int tok_equals(struct token *tok, const char *s);
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
  ND_ASSIGN, // =
  ND_IF,     // if condition
  // TODO: ND_PRINT is temporary solution, proper solution would be to
  // replace ND_PRINT with regular fucntion calls
  ND_PRINT,  // print following experssion to stdout
  ND_NUM,    // integer value
  ND_VAR     // variable
} node_kind;

struct var {
  char *name;
};

struct node {
  node_kind kind;
  struct node *next;
  struct node *lhs;
  struct node *rhs;
  struct token *tok;
  int num;        // integer value if `kind` is ND_NUM
  struct var var; // used if `kind` is ND_VAR

  // These three are used for conditional jumps
  struct node *cond;
  struct node *then;
  struct node *els;
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


#endif // _MAIN_H
