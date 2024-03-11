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

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static uint32_t choose(uint32_t n) {
	/* to generate a 
	 * different rand number in one second. */
	return rand() % n;
} 

//check division before zero, can just exclude literal division.
static bool check_legal_zero() {
	int pos = strlen(buf) - 1;

	if (buf[pos] == '/') {
		return false;
	} else if (buf[pos] == ' ') {
		return check_legal_zero();
	}
	return true;
}

static void gen_num() {
	char *tmp = {};
	int digit = rand() % 100;
	if (digit == 0) {
		if(!check_legal_zero()) {
			gen_num();
			return;
		}
	}
	sprintf(tmp, "%d", digit);
	strcat(buf, tmp);
}

static void gen(char *str) {
	strcat(buf, str);
}

static void gen_rand_op() {
	if (strlen(buf) > 65500) {
		return;
	}
	switch(choose(4)) {
		case 0: gen("+"); break;
		case 1: gen("-"); break;
		case 2: gen("*"); break;
		default: gen("/"); break;
	}
}

static void gen_rand_expr() {
	if (buf)
	switch(choose(4)) {
		case 0: gen_num(); break;
		case 1: gen("("); gen_rand_expr(); gen(")"); break;
		case 2: gen(" ");
		default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;	
	}
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
		// read from argv[1] to loop variable.
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
		// insert buf to code_fromat and store them in code_buf.
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
		
		// shell command, return 0 if success.
    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;
		
		// opens a process, readable.
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
