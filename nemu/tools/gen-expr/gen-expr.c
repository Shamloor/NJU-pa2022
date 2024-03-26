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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

static char buf[65536] = {};
static char code_buf[65536 + 128] = {};
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


static int choose(int limit) {
	return rand() % limit;
}

static void gen_num() {
	char str[10];
	int digit = rand() % 20;
	sprintf(str, "%d", digit);
	if (strlen(buf) > 60000) {
		return;
	}
	strcat(buf, str);
}

static void gen(char str) {
	char nstr[2];
	nstr[0] = str; nstr[1] = '\0';
	strcat(buf, nstr);
}

static void gen_rand_op() {
	switch(choose(4)) {
		case 0: gen('+'); break;
		case 1: gen('-'); break;
		case 2: gen('*'); break;
		default: gen('/'); break;
	}
}

static void gen_rand_expr(int depth) {
	if (depth > 10) {
		gen('('); gen_num(); gen(')');
		return;
	}

	switch(choose(3)) {
		case 0: gen(' '); gen_num();gen(' '); break;
		case 1: gen('('); gen_rand_expr(depth + 1); gen(')'); break;
		default: 
			gen_rand_expr(depth + 1); 
			gen_rand_op(); 
			gen_rand_expr(depth + 1); break;	
	}
}

// Tested.
static int find_expre(int start) {
	int check = 0;
	for (int i = start;; i ++) {
		if (buf[i] == '(') {
			check += 1;
		}
		else if (buf[i] == ')') {
			check -= 1;
			if (check == 0) {
				return i;
			} 
			else if (check == -1) {
				return i - 1;
			}
		}
		else if (check == 0) {
			if (buf[i] == '+' || buf[i] == '-' 
				|| buf[i] == '*' || buf[i] == '/') {
				return i - 1;
			} 
		}
	}
	assert(0);
	return 0;
}
// Tested.
static int calc_substr(char *str) {
	sprintf(code_buf, code_format, str);
	
	FILE *tfp = fopen("/tmp/.check.c", "w");
	assert(tfp != NULL);
	fputs(code_buf, tfp);
	fclose(tfp);

	int ret = system("gcc /tmp/.check.c -o /tmp/.calc");
	assert(ret == 0);

	tfp = popen("/tmp/.calc", "r");
	assert(tfp != NULL);

	int result;
	fscanf(tfp, "%d", &result);
	pclose(tfp);

	return result;
}

// Tested.
static int clear_zero() {
	for (int i = strlen(buf) - 1; i >= 0; i --) {
		if (buf[i] == '/') {
			int len = find_expre(i + 1) - i;
			
			char nstr[len + 1];
			strncpy(nstr, buf + i + 1, len);
			nstr[len] = '\0';
			
			int result = calc_substr(nstr);
			if (result == 0) {
				return 1;
			}
		}
	}
	return 0;
} 

int main(int argc, char *argv[]) {
  int seed = time(NULL);
  srand(seed);

  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }

  int i;
  for (i = 0; i < loop; i ++) {
		// clear char array.
		strcpy(buf, "");

    gen_rand_expr(0);
	
		// prevent zero division.
		if (clear_zero() == 1) {
			continue;
		}

		// insert buf to code_fromat and store them in code_buf.
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
		
		// shell command, return 0 if success.
    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;
		
		// open a process for input or output.
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
		// read from stream fp, store them in result.
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

		printf("%u %s\n", result, buf);
  }
  return 0;
}
