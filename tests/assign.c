#include "test.h"

void test_basic_assign() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 123\n");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  tok = tokenize(&tokenizer);
  prog = parse(tok);
  ASSERT_EQ(123, eval_node(prog->body));
}

void test_assign_sum_of_values() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 5\nb = 4\nc = a + b\n");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  execute_node(prog);

  tokenizer_init(&tokenizer, "c");
  tok = tokenize(&tokenizer);
  prog = parse(tok);
  ASSERT_EQ(9, eval_node(prog->body));
}

void test_assign_expression() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 2 + 2 * 2");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  tok = tokenize(&tokenizer);
  prog = parse(tok);
  ASSERT_EQ(6, eval_node(prog->body));
}

void test_braces_assignment() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 2 * (2 + 2) / 2");
  struct token *tok = tokenize(&tokenizer);
  struct node *prog = parse(tok);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  tok = tokenize(&tokenizer);
  prog = parse(tok);
  ASSERT_EQ(4, eval_node(prog->body));
}


int main() {
  test_basic_assign();
  test_assign_sum_of_values();
  test_assign_expression();
  test_braces_assignment();
  return 0;
}
