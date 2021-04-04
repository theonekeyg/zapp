#include "zapp.h"
#include "hash/hashtable.h"

#define INDENT_SIZE 2

static FILE *out; // global output file being compiled
struct hashtable vars;

__attribute__((format(printf, 1, 2)))
static void println(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vfprintf(out, fmt, va);
  va_end(va);
}

static void codegen_init(FILE *fp) {
  out = fp;
  htable_init(&vars, NULL);
  println("extern int printf(const char *__restrict __format, ...);\n\n");
  println("int main(int argc, char **argv) ");
}

static void c_generate_node(struct node *node) {
  static int level = 0;
  static bool with_newline = 1;

  switch (node->kind) {
    case ND_ADD:
      c_generate_node(node->lhs);
      println(" + ");
      c_generate_node(node->rhs);
      break;
    case ND_SUB:
      c_generate_node(node->lhs);
      println(" - ");
      c_generate_node(node->rhs);
      break;
    case ND_MUL:
      c_generate_node(node->lhs);
      println(" * ");
      c_generate_node(node->rhs);
      break;
    case ND_DIV:
      c_generate_node(node->lhs);
      println(" / ");
      c_generate_node(node->rhs);
      break;
    case ND_LT:
      c_generate_node(node->lhs);
      println(" < ");
      c_generate_node(node->rhs);
      break;
    case ND_LTE:
      c_generate_node(node->lhs);
      println(" <= ");
      c_generate_node(node->rhs);
      break;
    case ND_EQ:
      c_generate_node(node->lhs);
      println(" == ");
      c_generate_node(node->rhs);
      break;
    case ND_NEQ:
      c_generate_node(node->lhs);
      println(" != ");
      c_generate_node(node->rhs);
      break;
    case ND_NEG:
      println("-");
      c_generate_node(node->rhs);
      break;
    case ND_ASSIGN:
      if (with_newline) {
        println("\n%*c", level * INDENT_SIZE, ' ');
      }
      if (!htable_contains(&vars, node->lhs->var.name, node->lhs->var.len)) {
        htable_push(&vars, node->lhs->var.name, node->lhs->var.len, NULL);
        if (node->lhs->type->kind == TY_INT) {
          println("int ");
        } else if (node->lhs->type->kind == TY_FLOAT) {
          println("double ");
        }
      }

      c_generate_node(node->lhs);
      println(" = ");
      c_generate_node(node->rhs);

      if (with_newline) {
        println(";");
      }
      break;
    case ND_FOR:
      with_newline = 0;
      println("\n%*cfor (", level * INDENT_SIZE, ' ');
      if (node->init) {
        c_generate_node(node->init);
        println("; ");
      }

      if (node->cond) {
        c_generate_node(node->cond);
      }
      println("; ");
      if (node->inc) {
        c_generate_node(node->inc);
      }

      println(") ");
      with_newline = 1;
      c_generate_node(node->body);
      break;
    case ND_IF:
      println("\n%*cif (", level * INDENT_SIZE, ' ');
      c_generate_node(node->cond);
      println(") ");
      c_generate_node(node->then);
      if (node->els) {
        println(" else ");
        c_generate_node(node->els);
      }
      break;
    case ND_PRINT: {
        char *spec = "%d";
        if (node->rhs->type->kind == TY_FLOAT) {
          spec = "%lf";
        }
        println("\n%*cprintf(\"%s\\n\", ", level * INDENT_SIZE, ' ', spec);
        c_generate_node(node->rhs);
        println(");");
        break;
      }
    case ND_BLOCK:
      ++level;
      println("{");
      for (struct node *cur = node->body; cur; cur = cur->next) {
        c_generate_node(cur);
      }
      --level;
      if (level) {
        println("\n%*c}", level * INDENT_SIZE, ' ');
      } else {
        println("\n}");
      }
      break;
    case ND_NUM:
      if (node->type->kind == TY_INT) {
        println("%d", node->val.num);
      } else if (node->type->kind == TY_FLOAT) {
        println("%lf", node->val.fnum);
      }
      break;
    case ND_VAR:
      println("%.*s", node->var.len, node->var.name);
      break;
  }
}

void c_codegen(struct node *prog, FILE *fp) {
  codegen_init(fp);
  c_generate_node(prog);
  println("\n");
}
