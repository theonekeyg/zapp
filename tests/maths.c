#include "test.h"

void add_mul_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 + 2 * 2");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  ASSERT_EQ(6, eval_node(prog));
}

void add_cmp_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 < 1 + 2");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  ASSERT_EQ(1, eval_node(prog));
}

void div_cmp_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "5 > 10 / 3");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  ASSERT_EQ(1, eval_node(prog));
}

void paren_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 / (3 - 1)");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  ASSERT_EQ(1, eval_node(prog));
}

int main() {
  add_mul_precedence();
  add_cmp_precedence();
  div_cmp_precedence();
  paren_precedence();
  return 0;
}
