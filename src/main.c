#include "zapp.h"

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

#define INDENT_LEN 2

void _print_node_tree_recursive(struct node *node, int level) {
  switch (node->kind) {
    case ND_NUM:
      printf("%*c%s : %d\n", level * INDENT_LEN, ' ', nodekind_to_str[node->kind], node->num);
      break;
    case ND_VAR:
      printf("%*c%s : %s\n", level * INDENT_LEN, ' ', nodekind_to_str[node->kind], node->var.name);
      break;
    case ND_IF:
      printf("%*c%s\n", level * INDENT_LEN, ' ', nodekind_to_str[node->kind]);
      _print_node_tree_recursive(node->cond, level + 1);
      _print_node_tree_recursive(node->then, level + 1);
      if (node->els) {
        _print_node_tree_recursive(node->els, level + 1);
      }
      break;
    case ND_PRINT:
      printf("%*c%s\n", level * INDENT_LEN, ' ', nodekind_to_str[node->kind]);
      _print_node_tree_recursive(node->rhs, level + 1);
      break;
    case ND_FOR:
      printf("%*c%s\n", level * INDENT_LEN, ' ', nodekind_to_str[node->kind]);
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
      printf("%*c%s\n", level * INDENT_LEN, ' ', nodekind_to_str[node->kind]);
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

int main() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "for i in 0..5 { print i }");
  struct token *tok_list = tokenize(&tokenizer);
#ifdef DEBUG
  for (struct token *tok = tok_list; tok->kind != TOKEN_EOF; tok = tok->next) {
    for (int i = 0; i < tok->len; ++i) {
      putchar(tok->start[i]);
    }
    printf(" at [%d;%d]\n", tok->nline, tok->nrow);
  }
#endif // DEBUG
  struct node *program = parse(tok_list);
  print_node_tree(program);
  execute_node(program);
  return 0;
}
