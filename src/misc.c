#include "zapp.h"

_Noreturn void panic(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  exit(1);
}

_Noreturn void panic_tok(struct token *tok, const char *fmt, ...) {
  va_list ap;
  char err_msg[500];
  va_start(ap, fmt);
  vsnprintf(err_msg, 500, fmt, ap);
  va_end(ap);
  panic("Error: %s at [%d;%d]\n", err_msg, tok->nline, tok->nrow);
}
