#include "main.h"

const char *nodekind_to_str[] = {
  "ND_ADD",
  "ND_SUB",
  "ND_MUL",
  "ND_DIV",
  "ND_NUM"
};

void _print_node_tree_recursive(struct node *node, int level) {
  if (node->kind == ND_NUM) {
    printf("%*c%s : %d\n", level * 2, ' ', nodekind_to_str[node->kind], node->num);
  } else {
    printf("%*c%s\n", level * 2, ' ', nodekind_to_str[node->kind]);
    _print_node_tree_recursive(node->lhs, level + 1);
    _print_node_tree_recursive(node->rhs, level + 1);
  }
}

void print_node_tree(struct node *node) {
  _print_node_tree_recursive(node, 0);
}

double eval_node(struct node *node) {
  double rv = 0;
  switch(node->kind) {
    case ND_ADD:
      rv = eval_node(node->lhs) + eval_node(node->rhs);
      break;
    case ND_SUB:
      rv = eval_node(node->lhs) - eval_node(node->rhs);
      break;
    case ND_MUL:
      rv = eval_node(node->lhs) * eval_node(node->rhs);
      break;
    case ND_DIV:
      rv = eval_node(node->lhs) / eval_node(node->rhs);
      break;
    case ND_NUM:
      rv = node->num;
      break;
  }
  return rv;
}

int main() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 / (2 + 2) * 2");
  struct token *tok_list = tokenize(&tokenizer);
  for (struct token *tok = tok_list; tok->kind != TOKEN_EOF; tok = tok->next) {
    for (int i = 0; i < tok->len; ++i) {
      putchar(tok->start[i]);
    }
    printf(" at [%d;%d]\n", tok->nline, tok->nrow);
  }
  struct node *program = bin_op(&tok_list, tok_list);
  print_node_tree(program);
  printf("%lf\n", eval_node(program));
  return 0;
}
