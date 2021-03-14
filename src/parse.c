#include "zapp.h"

struct node *stmt(struct token **rest, struct token *tok);

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

struct node *new_num_literal(double num, struct token *tok) {
  struct node *node = new_node(ND_NUM, tok);
  node->num = num;
  return node;
}

// ident = [a-zA-Z_][a-zA-Z0-9_]*
struct node *ident(struct token **rest, struct token *tok) {
  if (tok->kind == TOKEN_IDENT) {
    struct node *node = new_node(ND_VAR, tok);
    node->var.name = strndup(tok->start, tok->len);
    *rest = tok->next;
    return node;
  }
  panic_tok(tok, "Expected an identifier, but received something else");
}

// num = [1-9][0-9]*
//     | '(' expr ')'
//     | ident
struct node *num(struct token **rest, struct token *tok) {
  if (tok_equals(tok, "(")) {
    tok = tok->next;
    struct node *node = expr(&tok, tok);
    if (tok_equals(tok, "(")) {
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

  if (tok->kind == TOKEN_IDENT) {
    struct node *node = ident(&tok, tok);
    *rest = tok;
    return node;
  }
  panic_tok(tok, "Expected a number, but received something else");
}

// mul = num ('*' num | '/' num)*
struct node *mul(struct token **rest, struct token *tok) {
  struct node *node = num(&tok, tok);
  for (;;) { struct token *start = tok;
    if (tok_equals(tok, "*")) {
      node = new_binary(ND_MUL, node, num(&tok, tok->next), start);
      continue;
    }
    if (tok_equals(tok, "/")) {
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
    if (tok_equals(tok, "+")) {
      node = new_binary(ND_ADD, node, mul(&tok, tok->next), start);
      continue;
    }
    if (tok_equals(tok, "-")) {
      node = new_binary(ND_SUB, node, mul(&tok, tok->next), start);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// logical = add ("<=" add | ">=" add | ">" add | "<" add)*
struct node *logical(struct token **rest, struct token *tok) {
  struct node *node = add(&tok, tok);
  for (;;) {
    struct token *start = tok;
    if (tok_equals(tok, "<=")) {
      node = new_binary(ND_LTE, node, add(&tok, tok->next), start);
    }
    if (tok_equals(tok, "<")) {
      node = new_binary(ND_LT, node, add(&tok, tok->next), start);
    }
    if (tok_equals(tok, ">")) {
      node = new_binary(ND_LT, add(&tok, tok->next), node, start);
    }
    if (tok_equals(tok, ">=")) {
      node = new_binary(ND_LT, add(&tok, tok->next), node, start);
    }

    *rest = tok;
    return node;
  }
}

// equation = logical ("==" logical | "!=" logical)*
struct node *equation(struct token **rest, struct token *tok) {
  struct node *node = logical(&tok, tok);

  for (;;) {
    struct token *start = tok;
    if (tok_equals(tok, "==")) {
      node = new_binary(ND_EQ, node, logical(&tok, tok->next), start);
    }
    if (tok_equals(tok, "!=")) {
      node = new_binary(ND_NEQ, node, logical(&tok, tok->next), start);
    }

    *rest = tok;
    return node;
  }
}

// expr = equation
//      | ident "=" expr
struct node *expr(struct token **rest, struct token *tok) {
  struct node *node;
  if (tok->kind == TOKEN_IDENT && tok_equals(tok->next, "=")) {
    struct token *start = tok;
    struct node *var = ident(&tok, tok);
    tok = tok->next;
    node = new_binary(ND_ASSIGN, var, expr(&tok, tok), start);
  } else {
    node = equation(&tok, tok);
  }
  *rest = tok;
  return node;
}

// braces_body = "{" stmt* "}"
struct node *braces_body(struct token **rest, struct token *tok) {
  struct node *node = new_node(ND_BLOCK, tok);
  struct token *start = tok;
  tok = tok_consume(tok, "{");

  struct node **cur_node = &node->body;
  while (!(tok_equals(tok, "}") || tok->kind == TOKEN_EOF)) {
    *cur_node = stmt(&tok, tok);
    cur_node = &(*cur_node)->next;
  }

  if (tok->kind == TOKEN_EOF) {
    panic_tok(start, "Unclosed braces body");
  }

  *rest = tok->next;
  return node;
}

// stmt = "if" expr braces_body ("else" braces_body)?
//      | "print" expr
//      | "for" ident "in" num ".." num braces_body
//      | expr
struct node *stmt(struct token **rest, struct token *tok) {
  if (tok_equals(tok, "if")) {
    struct node *node = new_node(ND_IF, tok);
    node->cond = expr(&tok, tok->next);
    node->then = braces_body(&tok, tok);
    if (tok_equals(tok, "else")) {
      node->els = braces_body(&tok, tok->next);
    }
    *rest = tok;
    return node;
  }

  if (tok_equals(tok, "print")) {
    struct node *node = new_node(ND_PRINT, tok);
    node->rhs = expr(&tok, tok->next);
    *rest = tok;
    return node;
  }

  if (tok_equals(tok, "for")) {
    struct node *node = new_node(ND_FOR, tok);

    struct node *var = ident(&tok, tok->next);
    tok = tok_consume(tok, "in");
    struct node *start = expr(&tok, tok);
    tok = tok_consume(tok, "..");
    struct node *end = expr(&tok, tok);

    node->init = new_binary(ND_ASSIGN, var, start, var->tok);
    node->cond = new_binary(ND_LT, var, end, tok);
    node->inc = new_binary(ND_ASSIGN, var,
        new_binary(ND_ADD, var, new_num_literal(1, NULL), tok),
        tok);
    node->body = braces_body(&tok, tok);
    *rest = tok;
    return node;
  }

  return expr(rest, tok);
}

struct node *parse(struct token *tokens) {
  struct node head = {};
  struct node *cur_node = &head;
  while (tokens->kind != TOKEN_EOF) {
    struct node *node = stmt(&tokens, tokens);
    cur_node->next = node;
    cur_node = cur_node->next;
  }
  return head.next;
}
