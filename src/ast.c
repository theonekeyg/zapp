#include "main.h"
#include "hash/hashtable.h"

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
    case ND_VAR:
      if (htable_contains(&locals, node->var.name, strlen(node->var.name))) {
        void *value = htable_get(&locals, node->var.name, strlen(node->var.name));
        rv = *(double *)&value;
      } else {
        panic_tok(node->tok, "Undefined variable");
      }
      break;
    case ND_ASSIGN: {
      // TODO: if entry already exists in the htable, update it's value,
      // instead of pushing new one
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
        execute_node(node->then);
      } else {
        if (node->els) {
        execute_node(node->els);
        }
      }
      break;
    case ND_PRINT:
      printf("%lf\n", eval_node(node->rhs));
      break;
    default:
      eval_node(node);
      break;
  }
}
