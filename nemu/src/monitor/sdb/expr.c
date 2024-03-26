/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
	TK_DINT, TK_LBR, TK_RBR,
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
	{"[0-9]+", TK_DINT},			// decimal integer	
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // add
	{"\\-", '-'},					// sub
	{"\\*", '*'},					// mul
	{"\\/", '/'},					//div
  {"==", TK_EQ},        // equal
	{"\\(", TK_LBR},				// left bra
	{"\\)", TK_RBR},				// right bra	
	
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

// Set size of tokens to 200, may cause segmentation fault, watch out.
static Token tokens[200] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

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

			
				// Unknown reason for segmentation fault, use Log.
        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
					case '+': 
					case '-':
					case '*':
					case '/':
					case TK_EQ:
					case TK_LBR:
					case TK_RBR:
						tokens[nr_token].type = rules[i].token_type;
						nr_token += 1;
						break;
					case TK_DINT:
						tokens[nr_token].type = rules[i].token_type;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						nr_token += 1;
						break;
        }
        break;
			} 
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", 
					position, e, position, "");
      return false;
    }
  }

	return true;
}




static bool check_parentheses(int p, int q) {

	int i = 0;
	int check = 0;
	int max = 0;
	if (tokens[p].type == TK_LBR && tokens[q].type == TK_RBR) {
		for (i = p; i <= q; i ++) {
			if (tokens[i].type == TK_LBR) {
				check += 1;
				if (check > max) max = check;
			} 
			else if (tokens[i].type == TK_RBR) {
				check -= 1;
			}
		
			if ((check < 0) || (i == q && check != 0)) {
				Log("The bracket format is wrong, please check.");
				assert(0);
				return false;
			}
			
			if (max != 0 && check == 0 && i != q) {
				return false;
			}
		}

		return true;
	}
	
	return false;
}

static int priop(int p, int q) {
	// variable checking whether be in bracket.	
	int check = 0;
	// varuable representing primary position.
	int pr_pos = 0;

	// for loop searching
	for (int i = p; i <= q; i ++) {
		// pre extract
		int itype = tokens[i].type;
		int ptype;
		if (pr_pos != 0) {
			ptype = tokens[pr_pos].type;
		}

		// for bracket
		if (itype == TK_LBR) {
			check += 1;
		}
		else if (itype == TK_RBR) {
			check -= 1;
		}
		

		// if is operator type
		else if ((itype == '+' || itype == '-' || 
			itype == '*' || itype == '/') && check == 0) {
			// initial assignment
			if (pr_pos == 0) {
				pr_pos = i;
			}
			// compare ptype and itype
			else {
				// pre assignment
				bool pcond1 = (ptype == '+' || ptype == '-');
				bool pcond2 = (ptype == '*' || ptype == '/');
				bool icond1 = (itype == '+' || itype == '-');
				bool icond2 = (itype == '/' || itype == '*');
				
				if ((pcond1 && icond1) || (pcond2 && icond2) 
						|| (pcond2 && icond1)) {
					pr_pos = i;
				}
			}
		}
	}	
	return pr_pos;
}

static int eval(int p, int q) {
	if (p > q) {
		Assert(0, "between %d and %d", p, q);
	} 
	else if (p == q) {
		return atoi(tokens[p].str);
	} 
	else if (check_parentheses(p, q)) {
		return eval(p + 1, q - 1);
	}
	else {
		int op = priop(p, q);
		int val1 = eval(p, op - 1);
		int val2 = eval(op + 1, q);

		// combination
		switch(tokens[op].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default: assert(0);
		}
	}
}

word_t expr(char *e, bool success) {
	memset(tokens, 0, sizeof(tokens));

	init_regex();

  if (!make_token(e)) {
    success = false;
    return 0;
  }
	
  /* TODO: Insert codes to evaluate the expression. */
	word_t result = eval(0, nr_token - 1); 
	printf("%u\n", result);
  return result;
}
