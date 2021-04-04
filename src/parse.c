#include "zapp.h"
#include "hash/hashtable.h"

struct node *stmt(struct tokenizer *tokenizer);

static struct hashtable vars = {};

static struct type type_int = { .kind = TY_INT };
static struct type type_float = { .kind = TY_FLOAT };

static double custom_atof(char *a) {
  double rv = 0;
  while (*a >= '0' && *a <= '9') {
    rv = rv * 10 + (*a++ - '0');
  }
  if (*a == '.') {
    ++a;
    float weight = 1.f;
    while (*a >= '0' && *a <= '9') {
      weight /= 10;
      rv += weight * (*a++ - '0');
    }
  }
  return rv;
}

static struct type *pick_type(struct type *ty1, struct type *ty2) {
  if (ty1->kind == TY_INT && ty2->kind == TY_INT) {
    return ty1;
  }

  if (ty1->kind == TY_FLOAT) {
    return ty1;
  }

  if (ty2->kind == TY_FLOAT) {
    return ty2;
  }

  return ty1;
}

static struct node *new_node(node_kind kind) {
  struct node *node = calloc(1, sizeof(*node));
  node->kind = kind;
  return node;
}

static struct node *new_binary(node_kind kind, struct node *lhs, struct node *rhs) {
  struct node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  node->type = pick_type(lhs->type, rhs->type);
  return node;
}

static struct node *new_num_literal(double num) {
  struct node *node = new_node(ND_NUM);
  node->val.num = num;
  if ((int)num != num) {
    node->type = &type_float;
  } else {
    node->type = &type_int;
  }
  return node;
}

// ident = [a-zA-Z_][a-zA-Z0-9_]*
struct node *ident(struct tokenizer *tokenizer) {
  struct token *tok;
  if ((tok = tok_peek(tokenizer))->kind == TOKEN_IDENT) {
    struct node *node = new_node(ND_VAR);
    node->var.name = strndup(tok->start, tok->len);
    node->var.len = tok->len;
    if (htable_contains(&vars, tok->start, tok->len)) {
      struct node *var_value = htable_get(&vars, tok->start, tok->len);
      node->type = var_value->type;
    }
    tok_consume_lookahead(tokenizer);
    return node;
  }
  panic_tok(tokenizer, "Expected an identifier, but received something else");
}

// num = [1-9][0-9]*
//     | '(' expr ')'
//     | ident
struct node *num(struct tokenizer *tokenizer) {
  if (tok_consume(tokenizer, "(")) {
    struct node *node = expr(tokenizer);
    if (!tok_consume(tokenizer, ")")) {
      panic_tok(tokenizer, "Not closed parentheses");
    }
    return node;
  }

  struct token *tok;
  if ((tok = tok_peek(tokenizer))->kind == TOKEN_NUM) {
    struct node *node = new_node(ND_NUM);
    if (tok->type->kind == TY_FLOAT) {
      node->val.fnum = custom_atof(tok->start);
    } else {
      node->val.num = custom_atof(tok->start);
    }
    node->type = tok->type;
    tok_consume_lookahead(tokenizer);
    return node;
  }

  if (tok_peek(tokenizer)->kind == TOKEN_IDENT) {
    struct node *node = ident(tokenizer);
    return node;
  }
  panic_tok(tokenizer, "Expected a number, but received something else");
}

// unary = ("-" | "+") unary
//       | num
struct node *unary(struct tokenizer *tokenizer) {
  if (tok_consume(tokenizer, "-")) {
    struct node *node = new_node(ND_NEG);
    node->rhs = unary(tokenizer);
    node->type = node->rhs->type;
    return node;
  }

  if (tok_consume(tokenizer, "+")) {
    return unary(tokenizer);
  }

  return num(tokenizer);
}

// mul = unary ('*' unary | '/' unary)*
struct node *mul(struct tokenizer *tokenizer) {
  struct node *node = unary(tokenizer);
  for (;;) {
    if (tok_consume(tokenizer, "*")) {
      node = new_binary(ND_MUL, node, unary(tokenizer));
      continue;
    }
    if (tok_consume(tokenizer, "/")) {
      node = new_binary(ND_DIV, node, unary(tokenizer));
      continue;
    }

    return node;
  }
}

// add = mul ('+' mul | '-' mul)*
struct node *add(struct tokenizer *tokenizer) {
  struct node *node = mul(tokenizer);
  for (;;) {
    if (tok_consume(tokenizer, "+")) {
      node = new_binary(ND_ADD, node, mul(tokenizer));
      continue;
    }
    if (tok_consume(tokenizer, "-")) {
      node = new_binary(ND_SUB, node, mul(tokenizer));
      continue;
    }

    return node;
  }
}

// logical = add ("<=" add | ">=" add | ">" add | "<" add)*
struct node *logical(struct tokenizer *tokenizer) {
  struct node *node = add(tokenizer);
  for (;;) {
    if (tok_consume(tokenizer, "<=")) {
      node = new_binary(ND_LTE, node, add(tokenizer));
    }
    if (tok_consume(tokenizer, "<")) {
      node = new_binary(ND_LT, node, add(tokenizer));
    }
    if (tok_consume(tokenizer, ">")) {
      node = new_binary(ND_LT, add(tokenizer), node);
    }
    if (tok_consume(tokenizer, ">=")) {
      node = new_binary(ND_LT, add(tokenizer), node);
    }

    return node;
  }
}

// equation = logical ("==" logical | "!=" logical)*
struct node *equation(struct tokenizer *tokenizer) {
  struct node *node = logical(tokenizer);

  for (;;) {
    if (tok_consume(tokenizer, "==")) {
      node = new_binary(ND_EQ, node, logical(tokenizer));
    }
    if (tok_consume(tokenizer, "!=")) {
      node = new_binary(ND_NEQ, node, logical(tokenizer));
    }

    return node;
  }
}

// expr = equation
//      | ident "=" expr
struct node *expr(struct tokenizer *tokenizer) {
  struct node *node;
  if (tok_peek(tokenizer)->kind == TOKEN_IDENT && tok_equals(tok_npeek(tokenizer, 2), "=")) {
    struct token *tok = tok_peek(tokenizer);
    struct node *var = ident(tokenizer);
    tok_skip(tokenizer, "=");
    struct node *rhs = expr(tokenizer);
    var->type = rhs->type;
    htable_push(&vars, var->var.name, var->var.len, var);
    node = new_binary(ND_ASSIGN, var, rhs);
  } else {
    node = equation(tokenizer);
  }
  return node;
}

// braces_body = "{" stmt* "}"
struct node *braces_body(struct tokenizer *tokenizer) {
  struct node *node = new_node(ND_BLOCK);
  tok_skip(tokenizer, "{");

  struct node **cur_node = &node->body;
  while (!(tok_equals(tok_peek(tokenizer), "}") || tok_peek(tokenizer)->kind == TOKEN_EOF)) {
    *cur_node = stmt(tokenizer);
    cur_node = &(*cur_node)->next;
  }

  tok_skip(tokenizer, "}");

  return node;
}

// stmt = "if" expr braces_body ("else" braces_body)?
//      | "print" expr
//      | "for" ident "in" num ".." num braces_body
//      | expr
struct node *stmt(struct tokenizer *tokenizer) {
  if (tok_consume(tokenizer, "if")) {
    struct node *node = new_node(ND_IF);
    node->cond = expr(tokenizer);
    node->then = braces_body(tokenizer);
    if (tok_consume(tokenizer, "else")) {
      node->els = braces_body(tokenizer);
    }
    return node;
  }

  if (tok_consume(tokenizer, "print")) {
    struct node *node = new_node(ND_PRINT);
    node->rhs = expr(tokenizer);
    return node;
  }

  if (tok_consume(tokenizer, "for")) {
    struct node *node = new_node(ND_FOR);

    struct node *var = ident(tokenizer);
    tok_skip(tokenizer, "in");
    struct node *start = expr(tokenizer);
    tok_skip(tokenizer, "..");
    struct node *end = expr(tokenizer);

    var->type = start->type;
    htable_push(&vars, var->var.name, var->var.len, var);

    node->init = new_binary(ND_ASSIGN, var, start);
    node->cond = new_binary(ND_LT, var, end);
    node->inc = new_binary(ND_ASSIGN, var,
        new_binary(ND_ADD, var, new_num_literal(1)));
    node->body = braces_body(tokenizer);
    return node;
  }

  return expr(tokenizer);
}

struct node *parse(struct tokenizer *tokenizer) {
  struct node *head = new_node(ND_BLOCK);
  struct node **cur_node = &head->body;
  if (!vars.buckets) {
    htable_init(&vars, NULL);
  }
  while (tok_peek(tokenizer)->kind != TOKEN_EOF) {
    struct node *node = stmt(tokenizer);
    (*cur_node) = node;
    cur_node = &(*cur_node)->next;
  }
  return head;
}
