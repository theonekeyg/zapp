#include "test.h"

void test_basic_if() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 0\nif (1) { a = 5 }");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  tok = tokenize(&tokenizer);
  prog = parse(tok);
  ASSERT_EQ(5, eval_node(prog->body));
}

void test_basic_if_else() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 0\nif (0) { a = 1 } else { a = 2 }");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  tok = tokenize(&tokenizer);
  prog = parse(tok);
  ASSERT_EQ(2, eval_node(prog->body));
}

int main() {
  test_basic_if();
}
