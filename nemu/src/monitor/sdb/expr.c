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
#include <memory/paddr.h>

enum {
  TK_NOTYPE = 256, 
	TK_DINT,
	TK_HEX,
	TK_REG,
	TK_LBR, 
	TK_RBR,
	TK_EQ,
	TK_NEQ,
	TK_AND,
	TK_DEREF,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
	// Precedence? Why?
	// Because hex is more higher than dec.
	
	{"^0x[0-9a-fA-F]+$", TK_HEX}, // hexadecimal number
	{"[0-9]+", TK_DINT},			// decimal integer	
	{"^\\$", TK_REG},			// register name															
  {" +", TK_NOTYPE},    // spaces
	{"\\(", TK_LBR},				// left bra
	{"\\)", TK_RBR},				// right bra
  {"\\+", '+'},         // add
	{"\\-", '-'},					// sub
	{"\\*", '*'},					// mul
	{"\\/", '/'},					//div
  {"==", TK_EQ},        // equal
	{"!=", TK_NEQ},				// not equal
	{"&&", TK_AND},				// and
	
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
					case TK_NEQ:
					case TK_LBR:
					case TK_RBR:
					case TK_REG:
						tokens[nr_token].type = rules[i].token_type;
						nr_token += 1;
						break;
					case TK_DINT:
					case TK_HEX:
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

	for (int i = p; i <= q; i ++) {
		int itype = tokens[i].type;
		// recording position
		int ptype;
		if (pr_pos != 0) {
			ptype = tokens[pr_pos].type;
		}

		if (itype == TK_LBR) {
			check += 1;
		}
		else if (itype == TK_RBR) {
			check -= 1;
		}
		
		// if is operator type
		else if (check == 0) {
			switch (itype) {
				case '+':
				case '-':
				case '*':
				case '/':
				case TK_EQ:
				case TK_NEQ:
				case TK_AND:
					// initial assignment
					if (pr_pos == 0) {
						pr_pos = i;
					}
					// comparison
					else {
						// Operator precedence.
						// */ > +- > ==!= > &&
						bool pcond1 = (ptype == '*' || ptype == '/');
						bool pcond2 = (ptype == '+' || ptype == '-');
						bool pcond3 = (ptype == TK_EQ || ptype == TK_NEQ);
						bool pcond4 = (ptype == TK_AND);

						bool icond1 = (itype == '*' || itype == '/');
						bool icond2 = (itype == '+' || itype == '-');
						bool icond3 = (itype == TK_EQ || itype == TK_NEQ);
						bool icond4 = (itype == TK_AND);

						if ((pcond1 && (icond1 || icond2 || icond3 || icond4)) ||
								(pcond2 && (icond2 || icond3 || icond4)) ||
								(pcond3 && (icond3 || icond4)) ||
								(pcond4 && (icond4))) {
							pr_pos = i;
						}
					}
					break;
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
		if (tokens[p].type == TK_DINT) {
			return atoi(tokens[p].str);
		} else if (tokens[p].type == TK_HEX) {
			return (int)strtol(tokens[p].str, NULL, 0);
		}
		
	} 
	else if ((p == q - 1) && tokens[p].type == TK_DEREF) {
		return paddr_read((word_t)strtol(tokens[q].str, NULL, 16), 4);
	}
	else if ((p == q - 1) && tokens[p].type == TK_REG) {
		bool success = true;
		return isa_reg_str2val(tokens[q].str, &success);
	}
	else if (check_parentheses(p, q)) {
		return eval(p + 1, q - 1);
	}
	else {
		int op = priop(p, q);
		int val1 = eval(p, op - 1);
		int val2 = eval(op + 1, q);

		switch(tokens[op].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case TK_EQ: return val1 == val2;
			case TK_NEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
			//TODO : can add more rules.
			default: assert(0);
		}
	}
	return 0;
}

word_t expr(char *e, bool *success) {
	memset(tokens, 0, sizeof(tokens));

  if (!make_token(e)) {
    success = false;
    return 0;
  }

	// Whether '*' is derefrence operator.
	for (int i = 0; i < nr_token; i ++) {
				if (tokens[i].type == '*' && (i == 0 || 
					tokens[i - 1].type == '+' ||
					tokens[i - 1].type == '-' ||
					tokens[i - 1].type == '*' ||
					tokens[i - 1].type == '/' ||
					tokens[i - 1].type == TK_LBR ||
					tokens[i - 1].type == TK_EQ ||
					tokens[i - 1].type == TK_NEQ ||
					tokens[i - 1].type == TK_AND)) {
			tokens[i].type = TK_DEREF;
		}
	}
	
	word_t result = eval(0, nr_token - 1); 
  return result;
}
