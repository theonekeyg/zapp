#include "test.h"

void test_basic_assign() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 123\n");
  struct node *prog = parse(&tokenizer);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  prog = parse(&tokenizer);
  ASSERT_EQ(123, eval_node(prog->body));
}

void test_assign_sum_of_values() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 5\nb = 4\nc = a + b\n");
  struct node *prog = parse(&tokenizer);
  execute_node(prog);

  tokenizer_init(&tokenizer, "c");
  prog = parse(&tokenizer);
  ASSERT_EQ(9, eval_node(prog->body));
}

void test_assign_expression() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 2 + 2 * 2");
  struct node *prog = parse(&tokenizer);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  prog = parse(&tokenizer);
  ASSERT_EQ(6, eval_node(prog->body));
}

void test_braces_assignment() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "a = 2 * (2 + 2) / 2");
  struct node *prog = parse(&tokenizer);
  execute_node(prog);

  tokenizer_init(&tokenizer, "a");
  prog = parse(&tokenizer);
  ASSERT_EQ(4, eval_node(prog->body));
}

int main() {
  test_basic_assign();
  test_assign_sum_of_values();
  test_assign_expression();
  test_braces_assignment();
  return 0;
}
