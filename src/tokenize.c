#include "zapp.h"
#include <ctype.h>

void tokenizer_init(struct tokenizer *tokenizer, char *buf) {
  tokenizer->buf = tokenizer->cur = buf;
  memset(tokenizer->lookahead, 0, sizeof(struct token) * MAX_LOOKAHEAD);
  tokenizer->avail_tokens = 0;
  tokenizer->nline = 0;
  tokenizer->ncol = 0;
}

static int punct_lookup(char *s) {
  static char *kw[] = {
    "<=", ">=", "!=", "==", "..", "+", "-", "/", "*", "<", ">", "(", ")", "{", "}", "=", "."
  };
  for (int i = 0; i < sizeof(kw)/sizeof(*kw); ++i) {
    if (!strncmp(kw[i], s, strlen(kw[i]))) {
      return strlen(kw[i]);
    }
  }
  return 0;
}

static void tok_init(struct token *tok, token_kind kind, char *start,
                     int len, int nline, int ncol) {
  tok->kind = kind;
  tok->start = start;
  tok->len = len;
  tok->nline = nline;
  tok->ncol = ncol;
}

static struct type *new_type(type_kind kind) {
  struct type *type = malloc(sizeof(*type));
  type->kind = kind;
  return type;
}

static void lex_one(struct tokenizer *tokenizer, struct token *tok) {
retry:
  if (*tokenizer->cur == ' ' || *tokenizer->cur == '\t') {
    INCR_COL(tokenizer, 1);
    ++tokenizer->cur;
    goto retry;
  }

  if (*tokenizer->cur == '\n') {
    INCR_LINE(tokenizer);
    ++tokenizer->cur;
    goto retry;
  }

  char *start = tokenizer->cur;

  // [1-9][0-9]*(.[0-9]*|.\s)?
  if (*tokenizer->cur >= '0' && *tokenizer->cur <= '9') {
    ++tokenizer->cur;

    type_kind kind = TY_INT;

    while (*tokenizer->cur >= '0' && *tokenizer->cur <= '9') {
      ++tokenizer->cur;
    }
    if (*tokenizer->cur == '.' && (IS_SPACE(*(tokenizer->cur+1)) ||
         (*(tokenizer->cur+1) >= '0' && *(tokenizer->cur+1) <= '9')))
    {
      ++tokenizer->cur;
      kind = TY_FLOAT;
      while (*tokenizer->cur >= '0' && *tokenizer->cur <= '9') {
        ++tokenizer->cur;
      }
    }

    int len = tokenizer->cur - start;
    tok_init(tok, TOKEN_NUM, start, len, tokenizer->nline, tokenizer->ncol);
    INCR_COL(tokenizer, len);
    tok->type = new_type(kind);
    return;
  }

  // [a-zA-Z_][a-zA-Z0-9_]*
  if (IS_CHAR(*tokenizer->cur) || *tokenizer->cur == '_') {
    ++tokenizer->cur;
    while (IS_CHAR(*tokenizer->cur) || *tokenizer->cur == '_' ||
           (*tokenizer->cur >= '0' && *tokenizer->cur <= '9')) {
      ++tokenizer->cur;
    }

    int len = tokenizer->cur - start;
    tok_init(tok, TOKEN_IDENT, start, len, tokenizer->nline, tokenizer->ncol);
    INCR_COL(tokenizer, len);
    return;
  }

  int punct_len = punct_lookup(tokenizer->cur);
  if (punct_len) {
    tok_init(tok, TOKEN_PUNCT, start, punct_len, tokenizer->nline, tokenizer->ncol);
    INCR_COL(tokenizer, punct_len);
    tokenizer->cur += punct_len;
    return;
  }

  if (*tokenizer->cur == '\0') {
    tok_init(tok, TOKEN_EOF, start, 0, tokenizer->nline, tokenizer->ncol);
    return;
  }

  panic_tok(tokenizer, "Received incorrect token");
}

static const char *token_types_str[] = {
  "TOKEN_NUM",
  "TOKEN_FNUM",
  "TOKEN_PUNCT",
  "TOKEN_IDENT",
  "TOKEN_EOF"
};

static void lex_one_token(struct tokenizer *tokenizer, struct token *tok) {
  lex_one(tokenizer, tok);
#ifdef DEBUG
  fprintf(stderr, "[%s] `%.*s` at [%d;%d]\n", token_types_str[tok->kind],
          tok->len, tok->start, tok->nline, tok->ncol);
#endif
}

void tok_consume_lookahead(struct tokenizer *tokenizer) {
  memmove(&tokenizer->lookahead[0], &tokenizer->lookahead[1],
          sizeof(struct token) * (MAX_LOOKAHEAD - tokenizer->avail_tokens));
  tokenizer->avail_tokens--;
}

int tok_equals(struct token *tok, const char *s) {
  return !strncmp(tok->start, s, tok->len) && s[tok->len] == '\0';
}

static void _tok_skip(struct tokenizer *tokenizer, const char *s) {
  struct token *tok = &tokenizer->lookahead[0];
  if (tok_equals(tok, s)) {
    tok_consume_lookahead(tokenizer);
    return;
  }
  panic_tok(tokenizer, "Expected `%s`, but received %.*s", s, tok->len, tok->start);
}

void tok_skip(struct tokenizer *tokenizer, const char *s) {
  if (tokenizer->avail_tokens == 0) {
    lex_one_token(tokenizer, &tokenizer->lookahead[0]);
    tokenizer->avail_tokens = 1;
  }
  _tok_skip(tokenizer, s);
}

// _tok_consume expects at least one lookahead to be already filled,
// otherwise calling this is UB
static int _tok_consume(struct tokenizer *tokenizer, const char *s) {
  struct token *tok = &tokenizer->lookahead[0];
  if (tok_equals(tok, s)) {
    tok_consume_lookahead(tokenizer);
    return 1;
  }
  return 0;
}

int tok_consume(struct tokenizer *tokenizer, const char *s) {
  if (tokenizer->avail_tokens == 0) {
    lex_one_token(tokenizer, &tokenizer->lookahead[0]);
    tokenizer->avail_tokens = 1;
  }
  return _tok_consume(tokenizer, s);
}

struct token *tok_peek(struct tokenizer *tokenizer) {
  if (tokenizer->avail_tokens == 0) {
    lex_one_token(tokenizer, &tokenizer->lookahead[0]);
    tokenizer->avail_tokens = 1;
  }
  return &tokenizer->lookahead[0];
}

struct token *tok_npeek(struct tokenizer *tokenizer, int n) {
  if (n > MAX_LOOKAHEAD) {
    // TODO: Make a better way of raising ICE
    fprintf(stderr, "ICE: desired lookahead is disabled by the compiler configuration\n");
    exit(1);
  }

  if (tokenizer->avail_tokens >= n) {
    return &tokenizer->lookahead[n-1];
  }

  int avail;
  for (avail = tokenizer->avail_tokens; avail < n; ++avail) {
    lex_one_token(tokenizer, &tokenizer->lookahead[avail]);
  }
  tokenizer->avail_tokens = avail;
  return &tokenizer->lookahead[n-1];
}
