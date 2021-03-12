#include "test.h"

int main() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 + 2 * 2");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = expr(&tok, tok);
  ASSERT_EQ(6, eval_node(prog));
  return 0;
}
