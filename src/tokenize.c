#include "zapp.h"
#include <ctype.h>

static struct token *new_token(token_kind kind, char *start, int len, int nline, int nrow) {
  struct token *tok = calloc(1, sizeof(*tok));
  tok->kind = kind;
  tok->start = start;
  tok->len = len;
  tok->nline = nline;
  tok->nrow = nrow;
  return tok;
}

void tokenizer_init(struct tokenizer *tokenizer, char *buf) {
  tokenizer->buf = tokenizer->cur = tokenizer->tok_start = tokenizer->tok_end = buf;
  tokenizer->nline = 0;
  tokenizer->nrow = 0;
}

static int punct_lookup(char *s) {
  static char *kw[] = {
    "<=", ">=", "!=", "==", "..", "+", "-", "/", "*", "<", ">", "(", ")", "{", "}", "="
  };
  for (int i = 0; i < sizeof(kw)/sizeof(*kw); ++i) {
    if (!strncmp(kw[i], s, strlen(kw[i]))) {
      return strlen(kw[i]);
    }
  }
  return 0;
}

struct token *tokenize(struct tokenizer *tokenizer) {
  struct token head = {};
  struct token *cur_token = &head;
  for (;;) {
    if (*tokenizer->cur == ' ' || *tokenizer->cur == '\t') {
      ++tokenizer->nrow;
      ++tokenizer->cur;
      continue;
    }

    if (*tokenizer->cur == '\n') {
      ++tokenizer->nline;
      ++tokenizer->cur;
      tokenizer->nrow = 0;
      continue;
    }

    char *start = tokenizer->cur;

    // [1-9][0-9]*(.[0-9])*
    if (*tokenizer->cur >= '0' && *tokenizer->cur <= '9') {
      ++tokenizer->cur;
      ++tokenizer->nrow;
      while (*tokenizer->cur >= '0' && *tokenizer->cur <= '9') {
        ++tokenizer->cur;
        ++tokenizer->nrow;
      }
      /* @@@ remove floating point numbers for now
      if (*tokenizer->cur == '.') {
        while (*tokenizer->cur >= '0' && *tokenizer->cur <= '9') {
          ++tokenizer->cur;
          ++tokenizer->nrow;
        }
      }
      */
      struct token *tok = new_token(TOKEN_NUM, start, tokenizer->cur - start,
                                    tokenizer->nline, tokenizer->nrow);
      cur_token->next = tok;
      cur_token = cur_token->next;
      continue;
    }

    // [a-zA-Z_][a-zA-Z0-9_]*
    if (IS_CHAR(*tokenizer->cur) || *tokenizer->cur == '_') {
      ++tokenizer->cur;
      ++tokenizer->nrow;
      while (IS_CHAR(*tokenizer->cur) || *tokenizer->cur == '_' ||
             (*tokenizer->cur >= '0' && *tokenizer->cur <= '9')) {
        ++tokenizer->cur;
        ++tokenizer->nrow;
      }
      struct token *tok = new_token(TOKEN_IDENT, start, tokenizer->cur - start,
                                    tokenizer->nline, tokenizer->nrow);
      cur_token->next = tok;
      cur_token = cur_token->next;
      continue;
    }

    int punct_len = punct_lookup(tokenizer->cur);
    if (punct_len) {
      struct token *tok = new_token(TOKEN_PUNCT, tokenizer->cur, punct_len,
                                    tokenizer->nline, tokenizer->nrow);
      tokenizer->nrow += punct_len;
      tokenizer->cur += punct_len;
      cur_token->next = tok;
      cur_token = cur_token->next;
      continue;
    }

    if (*tokenizer->cur == '\0') {
      break;
    } else {
      panic("Received incorrect token at %d:%d\n", tokenizer->nline, tokenizer->nrow);
    }
  }

  struct token *tok = new_token(TOKEN_EOF, tokenizer->cur, 1,
                                tokenizer->nline, tokenizer->nrow);
  cur_token->next = tok;

  return head.next;
}

int tok_equals(struct token *tok, const char *s) {
  return !strncmp(tok->start, s, tok->len) && s[tok->len] == '\0';
}

struct token *tok_consume(struct token *tok, const char *s) {
  if (tok_equals(tok, s)) {
    return tok->next;
  }
  panic_tok(tok, "Expected `%s`, but received something else", s);
}
