#include "zapp.h"
#include <ctype.h>

static struct token *new_token(token_kind kind, char *start, int len, int nline, int ncol) {
  struct token *tok = calloc(1, sizeof(*tok));
  tok->kind = kind;
  tok->start = start;
  tok->len = len;
  tok->nline = nline;
  tok->ncol = ncol;
  return tok;
}

void tokenizer_init(struct tokenizer *tokenizer, char *buf) {
  tokenizer->buf = tokenizer->cur = buf;
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

static struct type *new_type(type_kind kind) {
  struct type *type = malloc(sizeof(*type));
  type->kind = kind;
  return type;
}

struct token *tokenize(struct tokenizer *tokenizer) {
  struct token head = {};
  struct token *cur_token = &head;
  for (;;) {
    if (*tokenizer->cur == ' ' || *tokenizer->cur == '\t') {
      INCR_COL(tokenizer, 1);
      ++tokenizer->cur;
      continue;
    }

    if (*tokenizer->cur == '\n') {
      INCR_LINE(tokenizer);
      ++tokenizer->cur;
      continue;
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
           (*(tokenizer->cur+1) >= '0' && *(tokenizer->cur+1) <= '9'))
         ) {
        ++tokenizer->cur;
        kind = TY_FLOAT;
        while (*tokenizer->cur >= '0' && *tokenizer->cur <= '9') {
          ++tokenizer->cur;
        }
      }

      int len = tokenizer->cur - start;
      INCR_COL(tokenizer, len);
      struct token *tok = new_token(TOKEN_NUM, start, len,
                                    tokenizer->nline, tokenizer->ncol);
      tok->type = new_type(kind);
      cur_token->next = tok;
      cur_token = cur_token->next;
      continue;
    }

    // [a-zA-Z_][a-zA-Z0-9_]*
    if (IS_CHAR(*tokenizer->cur) || *tokenizer->cur == '_') {
      ++tokenizer->cur;
      while (IS_CHAR(*tokenizer->cur) || *tokenizer->cur == '_' ||
             (*tokenizer->cur >= '0' && *tokenizer->cur <= '9')) {
        ++tokenizer->cur;
      }

      int len = tokenizer->cur - start;
      INCR_COL(tokenizer, len);
      struct token *tok = new_token(TOKEN_IDENT, start, len,
                                    tokenizer->nline, tokenizer->ncol);
      cur_token->next = tok;
      cur_token = cur_token->next;
      continue;
    }

    int punct_len = punct_lookup(tokenizer->cur);
    if (punct_len) {
      struct token *tok = new_token(TOKEN_PUNCT, tokenizer->cur, punct_len,
                                    tokenizer->nline, tokenizer->ncol);
      INCR_COL(tokenizer, punct_len);
      tokenizer->cur += punct_len;
      cur_token->next = tok;
      cur_token = cur_token->next;
      continue;
    }

    if (*tokenizer->cur == '\0') {
      break;
    } else {
      panic("Received incorrect token at %d:%d\n", tokenizer->nline, tokenizer->ncol);
    }
  }

  struct token *tok = new_token(TOKEN_EOF, tokenizer->cur, 1,
                                tokenizer->nline, tokenizer->ncol);
  cur_token->next = tok;

  return head.next;
}

int tok_equals(struct token *tok, const char *s) {
  return !strncmp(tok->start, s, tok->len) && s[tok->len] == '\0';
}

void tok_skip(struct token **tok, const char *s) {
  if (tok_equals(*tok, s)) {
    *tok = (*tok)->next;
  }
}

struct token *tok_consume(struct token *tok, const char *s) {
  if (tok_equals(tok, s)) {
    return tok->next;
  }
  panic_tok(tok, "Expected `%s`, but received something else", s);
}
