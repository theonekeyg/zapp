#include "zapp.h"
#include "hash/hashtable.h"

#define NODE_INDENT_LEN 2

struct hashtable locals = {};

double _eval_node(struct node *node) {
  double rv = 0;
  switch(node->kind) {
    case ND_ADD:
      rv = _eval_node(node->lhs) + _eval_node(node->rhs);
      break;
    case ND_SUB:
      rv = _eval_node(node->lhs) - _eval_node(node->rhs);
      break;
    case ND_MUL:
      rv = _eval_node(node->lhs) * _eval_node(node->rhs);
      break;
    case ND_DIV:
      rv = _eval_node(node->lhs) / _eval_node(node->rhs);
      break;
    case ND_LT:
      rv = _eval_node(node->lhs) < _eval_node(node->rhs);
      break;
    case ND_LTE:
      rv = _eval_node(node->lhs) <= _eval_node(node->rhs);
      break;
    case ND_EQ:
      rv = _eval_node(node->lhs) == _eval_node(node->rhs);
      break;
    case ND_NEQ:
      rv = _eval_node(node->lhs) != _eval_node(node->rhs);
      break;
    case ND_NUM:
      rv = node->num;
      break;
    case ND_NEG:
      rv = -_eval_node(node->rhs);
      break;
    case ND_VAR:
      if (htable_contains(&locals, node->var.name, strlen(node->var.name))) {
        void *value = htable_get(&locals, node->var.name, strlen(node->var.name));
        rv = *(double *)&value;
      } else {
        panic_tok(node->tok, "Undefined variable");
      }
      break;
    case ND_ASSIGN: {
      double tmp = _eval_node(node->rhs);
      htable_push(
          &locals, node->lhs->var.name,
          strlen(node->lhs->var.name),
          (void *)*(uint64_t *)&tmp
      );
      rv = tmp;
      break;
    }
  }
  return rv;
}

double eval_node(struct node *node) {
  if (!locals.buckets) {
    htable_init(&locals, NULL);
  }
  return _eval_node(node);
}

void execute_node(struct node *node) {
  switch (node->kind) {
    case ND_IF:
      if (eval_node(node->cond)) {
        for (struct node *body = node->then->body; body; body = body->next) {
          execute_node(body);
        }
      } else {
        if (node->els) {
          for (struct node *body = node->els->body; body; body = body->next) {
            execute_node(body);
          }
        }
      }
      break;
    case ND_PRINT:
      printf("%lf\n", eval_node(node->rhs));
      break;
    case ND_FOR:
      if (node->init) {
        execute_node(node->init);
      }
      while (eval_node(node->cond) != 0) {
        execute_node(node->body);
        if (node->inc) {
          execute_node(node->inc);
        }
      }
      break;
    case ND_BLOCK:
      for (struct node *tmp = node->body; tmp; tmp = tmp->next) {
        execute_node(tmp);
      }
      break;
    default:
      eval_node(node);
      break;
  }
}

static const char *nodekind_to_str[] = {
  "ND_ADD",
  "ND_SUB",
  "ND_MUL",
  "ND_DIV",
  "ND_LT",
  "ND_LTE",
  "ND_EQ",
  "ND_NEQ",
  "ND_NEG",
  "ND_ASSIGN",
  "ND_FOR",
  "ND_IF",
  "ND_PRINT",
  "ND_BLOCK",
  "ND_NUM",
  "ND_VAR"
};

static void _print_node_tree_recursive(struct node *node, int level) {
  switch (node->kind) {
    case ND_NUM:
      printf("%*c%s : %d\n", level * NODE_INDENT_LEN, ' ', nodekind_to_str[node->kind],
             node->num);
      break;
    case ND_VAR:
      printf("%*c%s : %s\n", level * NODE_INDENT_LEN, ' ', nodekind_to_str[node->kind],
             node->var.name);
      break;
    case ND_IF:
      printf("%*c%s\n", level * NODE_INDENT_LEN, ' ', nodekind_to_str[node->kind]);
      _print_node_tree_recursive(node->cond, level + 1);
      _print_node_tree_recursive(node->then, level + 1);
      if (node->els) {
        _print_node_tree_recursive(node->els, level + 1);
      }
      break;
    case ND_PRINT:
      printf("%*c%s\n", level * NODE_INDENT_LEN, ' ', nodekind_to_str[node->kind]);
      _print_node_tree_recursive(node->rhs, level + 1);
      break;
    case ND_FOR:
      printf("%*c%s\n", level * NODE_INDENT_LEN, ' ', nodekind_to_str[node->kind]);
      _print_node_tree_recursive(node->init, level + 1);
      _print_node_tree_recursive(node->cond, level + 1);
      _print_node_tree_recursive(node->inc, level + 1);
      _print_node_tree_recursive(node->body, level + 1);
      break;
    case ND_BLOCK:
      node = node->body;
      for (; node; node = node->next) {
        _print_node_tree_recursive(node, level);
      }
      break;
    default:
      printf("%*c%s\n", level * NODE_INDENT_LEN, ' ', nodekind_to_str[node->kind]);
      if (node->lhs) {
        _print_node_tree_recursive(node->lhs, level + 1);
      }
      if (node->rhs) {
        _print_node_tree_recursive(node->rhs, level + 1);
      }
  }
}

void print_node_tree(struct node *node) {
  _print_node_tree_recursive(node, 0);
}
