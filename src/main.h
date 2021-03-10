#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
 * tokenize
 */

typedef enum {
  TOKEN_NUM,
  TOKEN_PUNCT,
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
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_LT,  // <
  ND_LTE, // <=
  ND_EQ,  // ==
  ND_NEQ, // !=
  ND_NUM  // [1-9][0-9]*
} node_kind;

struct node {
  node_kind kind;
  struct node *lhs;
  struct node *rhs;
  struct token *tok;
  int num; // integer value if `kind` is ND_NUM
};

struct node *bin_op(struct token **rest, struct token *tok);

/*
 * misc
 */

_Noreturn void panic(const char *fmt, ...);

_Noreturn void panic_tok(struct token *tok, const char *fmt, ...);

#endif // _MAIN_H
