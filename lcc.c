#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Token type values
enum {
  TK_NUM = 256,			/* number token */
  TK_EOF,			/* input terminal */
  ND_NUM = 256,			/* number node */
};

// Token type
typedef struct {
  int ty;			/* token type */
  int val;			/* token value; if its type is TK_NUM, val has its number. */
  char *input;			/* token string(error message) */
} Token;

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;			/* use this value if ty is ND_NUM */
} Node;

// Tokenized token array (max: 100)
Token tokens[100];

// tokenizer position
int pos;

Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int);
Node *term();
Node *mul();
Node *expr();
void tokenize(char *p);
void error();
void gen(Node *node);

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *term() {
  if (tokens[pos].ty == ND_NUM)
    return new_node_num(tokens[pos++].val);
  if (tokens[pos].ty == '(') {
    pos++;
    Node *node = expr();
    if (tokens[pos].ty != ')')
      error();
    pos++;
    return node;
  }
  error();
  return NULL;
}

Node *mul() {
  Node *lhs = term();
  int ty = tokens[pos].ty;
  if (ty == '*' || ty == '/') {
    pos++;
    return new_node(ty, lhs, mul());
  }
  return lhs;
}

Node *expr() {
  Node *lhs = mul();
  int ty = tokens[pos].ty;
  if (ty == '+' ||ty == '-') {
    pos++;
    return new_node(ty, lhs, expr());
  }
  return lhs;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    break;
  default:
    printf("  ??? %c\n", node->ty);
  }

  printf("  push rax\n");
}

// divide string argument into the tokens
void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // skip blank
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = ND_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "failed to tokenize: %s\n", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

void error() {
  fprintf(stderr, "unexpected token: %s\n",
	  tokens[pos].input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid the number of argument");
    return 1;
  }

  tokenize(argv[1]);
  Node* node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
