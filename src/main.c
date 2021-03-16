#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "zapp.h"

static int arg_flags = 0x0;

_Noreturn static void usage() {
  printf("Usage: zapp [options] [-i cmd | filename]\n\n");
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
      fprintf(stderr, "Error: -i option requires an argument\n");
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
  } else if (!strncmp(*argv, "-", 1)) {
    fprintf(stderr, "Error: option `%s` is not recognized\n", *argv);
    usage();
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
    fprintf(stderr, "Error: no source input is provided\n");
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
