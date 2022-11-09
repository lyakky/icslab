#include "debug.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_PLUS,TK_MINUS, TK_ASTERISK, TK_SLASH,
  TK_LPAREN, TK_RPAREN, TK_INT10,  TK_INT16


  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"0[xX][0-9a-fA-F]{8}", TK_INT16},
  {"[0-9]+", TK_INT10},
  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},        // equal
  {"-", TK_MINUS},         // minus
  {"\\*", TK_ASTERISK},  //asterisk
  {"/", TK_SLASH},       //SLASH
  {"\\(", TK_LPAREN},      //(
  {"\\)", TK_RPAREN}       //)
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
  } Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static void generate_token(char *pos, size_t len, unsigned int type)
{

  tokens[nr_token].type = type;
  for (size_t i = 0; i < len && i < 32; i++)
  {
    tokens[nr_token].str[i] = *(pos + i);
  }
  nr_token++;
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_EQ:
          case TK_PLUS:
          case TK_INT16:
          case TK_INT10:
            generate_token(substr_start, substr_len, rules[i].token_type);
            Log("generate_token type:%d,content:%s", tokens[nr_token-1].type, tokens[nr_token-1].str);
            break;  

          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool check_parentheses(int p, int q){


  return true;
}

static uint32_t eval(int p, int q){
  if(p > q){
    return -1;//bad expression
  }else if(p == q){
    uint32_t v;
    switch(tokens[p].type){
      case TK_INT10:
        v = (uint32_t)atoi(tokens[p].str);
        break;
      case TK_INT16:
        v = (uint32_t)strtoul(tokens[p].str, NULL, 16);
        break;
    }
    Log("eval value:%d\n", v);
    return v;
  } else if(check_parentheses(p, q) == true){
    return eval(p+1, q-1);
  }else{




  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  uint32_t res;
  res = eval(0, nr_token-1);
  if(res < 0){
    *success = false;
    return -1;
  }
  
  *success = true;
  return res;
}

