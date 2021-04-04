#include "test.h"

void test_add_mul_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 + 2 * 2");
  struct node *prog = parse(&tokenizer);
  ASSERT_EQ(6, eval_node(prog->body));
}

void test_add_cmp_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 < 1 + 2");
  struct node *prog = parse(&tokenizer);
  ASSERT_EQ(1, eval_node(prog->body));
}

void test_div_cmp_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "5 > 10 / 3");
  struct node *prog = parse(&tokenizer);
  ASSERT_EQ(1, eval_node(prog->body));
}

void test_paren_precedence() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 / (3 - 1)");
  struct node *prog = parse(&tokenizer);
  ASSERT_EQ(1, eval_node(prog->body));
}

void test_add_mul_recursive() {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer, "2 * (2 + 2) * 2");
  struct node *prog = parse(&tokenizer);
  ASSERT_EQ(16, eval_node(prog->body));
}

int main() {
  test_add_mul_precedence();
  test_add_cmp_precedence();
  test_div_cmp_precedence();
  test_paren_precedence();
  test_add_mul_recursive();
  return 0;
}
