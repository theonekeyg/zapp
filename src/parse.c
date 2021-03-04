#include "main.h"

static int custom_atoi(char *a) {
  int rv = 0;
  while (*a >= '0' && *a <= '9') {
    rv = rv * 10 + (*a++ - '0');
  }
  return rv;
}

struct node *new_node(node_kind kind, struct token *tok) {
  struct node *node = calloc(1, sizeof(*node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

struct node *new_binary(node_kind kind, struct node *lhs, struct node *rhs, struct token *tok) {
  struct node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// num = ([1-9][0-9]* | '(' [1-9][0-9]* ')')
struct node *num(struct token **rest, struct token *tok) {
  if (tok->kind == TOKEN_OPAREN) {
    tok = tok->next;
    struct node *node = bin_op(&tok, tok);
    if (tok->kind != TOKEN_CPAREN) {
      panic_tok(tok, "Not closed parentheses");
    }
    *rest = tok->next;
    return node;
  }
  if (tok->kind == TOKEN_NUM) {
    struct node *node = new_node(ND_NUM, tok);
    node->num = custom_atoi(tok->start);
    *rest = tok->next;
    return node;
  }
  panic_tok(tok, "Expected a number, but received something else");
}

// mul = num ('*' num | '/' num)*
struct node *mul(struct token **rest, struct token *tok) {
  struct node *node = num(&tok, tok);
  for (;;) {
    struct token *start = tok;
    if (tok->kind == TOKEN_MUL) {
      node = new_binary(ND_MUL, node, num(&tok, tok->next), start);
      continue;
    }
    if (tok->kind == TOKEN_DIV) {
      node = new_binary(ND_DIV, node, num(&tok, tok->next), start);
      continue;
    }
    *rest = tok;
    return node;
  }
}

// add = mul ('+' mul | '-' mul)*
struct node *add(struct token **rest, struct token *tok) {
  struct node *node = mul(&tok, tok);
  for (;;) {
    struct token *start = tok;
    if (tok->kind == TOKEN_ADD) {
      node = new_binary(ND_ADD, node, mul(&tok, tok->next), start);
      continue;
    }
    if (tok->kind == TOKEN_SUB) {
      node = new_binary(ND_SUB, node, mul(&tok, tok->next), start);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// bin_op = add
struct node *bin_op(struct token **rest, struct token *tok) {
  struct node *node = add(&tok, tok);
  *rest = tok;
  return node;
}
