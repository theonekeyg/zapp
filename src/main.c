#include "main.h"

const char *nodekind_to_str[] = {
  "ND_ADD",
  "ND_SUB",
  "ND_MUL",
  "ND_DIV",
  "ND_LT",
  "ND_LTE",
  "ND_EQ",
  "ND_NEQ",
  "ND_ASSIGN",
  "ND_NUM",
  "ND_VAR"
};

void _print_node_tree_recursive(struct node *node, int level) {
  if (node->kind == ND_NUM) {
    printf("%*c%s : %d\n", level * 2, ' ', nodekind_to_str[node->kind], node->num);
  } else if (node->kind == ND_VAR) {
    printf("%*c%s : %s\n", level * 2, ' ', nodekind_to_str[node->kind], node->var.name);
  } else {
    printf("%*c%s\n", level * 2, ' ', nodekind_to_str[node->kind]);
    _print_node_tree_recursive(node->lhs, level + 1);
    _print_node_tree_recursive(node->rhs, level + 1);
  }
}

void print_bin_tree(struct node *node) {
  _print_node_tree_recursive(node, 0);
}

int main() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = (2 + 2) * 2\na * 2");
  struct token *tok_list = tokenize(&tokenizer);
  for (struct token *tok = tok_list; tok->kind != TOKEN_EOF; tok = tok->next) {
    for (int i = 0; i < tok->len; ++i) {
      putchar(tok->start[i]);
    }
    printf(" at [%d;%d]\n", tok->nline, tok->nrow);
  }
  struct node *program = parse(tok_list);
  for (; program; program = program->next) {
    print_bin_tree(program);
    printf("%lf\n", eval_node(program));
  }
  return 0;
}
