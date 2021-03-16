#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "zapp.h"

static int arg_flags = 0x0;

_Noreturn static void usage() {
  printf("Usage: zapp [options] [-i cmd | filename]\n\n");
  printf(" Very minimalistic language\n");
  exit(0);
}

static char *read_file(const char *fname) {
  int fd = open(fname, O_RDONLY);
  if (fd == -1) {
    return NULL;
  }

  struct stat st;
  if (fstat(fd, &st) != 0) {
    close(fd);
    return NULL;
  }

  if (!(st.st_mode & S_IFREG)) {
    errno = EBADF; // Error: not a regular file
    close(fd);
    return NULL;
  }

  char *buf = malloc(st.st_size * sizeof(char) + 1);
  if (!buf) {
    close(fd);
    return NULL;
  }

  if (read(fd, buf, st.st_size) == -1) {
    free(buf);
    close(fd);
    return NULL;
  }

  buf[st.st_size] = '\0';
  return buf;
}

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

static void shift_arg(int *argc, char **argv) {
  if (*argc < 1) {
    return;
  }
  memmove(argv, &argv[1], (*argc - 1) * sizeof(*argv));
  *argc -= 1;
}

static void parse_arg(int *argc, char **argv, struct tokenizer *tokenizer) {
  if (!strcmp(*argv, "--help")) {
    usage();
  } else if (!strcmp(*argv, "-i")) {
    if (*argc < 2) {
      panic("Error: -i option requires an argument\n");
      usage();
      exit(1);
    }
    // If buffer already filled, just skip -i argument
    if (tokenizer->buf) {
      shift_arg(argc, argv);
      shift_arg(argc, argv);
    } else {
      shift_arg(argc, argv);
      tokenizer_init(tokenizer, strdup(*argv));
      arg_flags |= ARG_TBUF_FILLED;
      shift_arg(argc, argv);
    }
  } else if (!strcmp(*argv, "-c")) {
    shift_arg(argc, argv);
    arg_flags |= ARG_COMPILE;
  } else if (!strcmp(*argv, "-t")) {
    shift_arg(argc, argv);
    arg_flags |= ARG_PRINT_TREE;
  } else {
    // If buffer already filled, do nothing
    if (tokenizer->buf) {
      shift_arg(argc, argv);
    } else {
      char *buf;
      if ((buf = read_file(*argv))) {
        tokenizer_init(tokenizer, read_file(*argv));
        arg_flags |= ARG_TBUF_FILLED;
        shift_arg(argc, argv);
      } else {
        panic("Error: %s\n", strerror(errno));
        exit(1);
      }
    }
  }
}

static void parse_args(int argc, char **argv, struct tokenizer *tokenizer) {
  shift_arg(&argc, argv);

  while (argc > 0) {
    parse_arg(&argc, argv, tokenizer);
  }
}

int main(int argc, char **argv) {
  struct tokenizer tokenizer = {};
  parse_args(argc, argv, &tokenizer);
  if (!(arg_flags & ARG_TBUF_FILLED)) {
    panic("Error: no source input is provided\n");
    usage();
  }
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

  if (arg_flags & ARG_PRINT_TREE) {
    print_node_tree(program);
  }

  if (arg_flags & ARG_COMPILE) {
    c_codegen(program, stdout);
  } else {
    execute_node(program);
  }

  return 0;
}
