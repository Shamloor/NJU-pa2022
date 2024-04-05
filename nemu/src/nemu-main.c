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

#include <common.h>
void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char *e, bool success);
void init_regex();
void test_expr();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
//	test_expr();
	return 0;
}

//void test_expr() {
//	bool success = true;
//	char *filename = "/home/pa/ics2022/nemu/tools/gen-expr/input";
//	FILE *fp = fopen(filename, "r");
//	
//	char *line = NULL;
//	size_t len = 0;
//	int count = 1;
//
//	while (getline(&line, &len, fp) != -1) {
//		printf("The %d times: \n", count);
//		char *expre = strchr(line, ' ');
//		expre[strcspn(expre, "\n")] = 0;
//		word_t result1 = atoi(line);
//		//printf("%u %s\n", result1, expre + 1);
//		init_regex();
//		word_t result2 = expr(expre + 1, success);
//		if (result1 != result2) {
//			Assert(0, "Not equal: %u and %u\n", result1, result2);
//		}
//		if (success == false) {
//			assert(0);
//		}
//		count += 1;	
//	}
//
//	fclose(fp);
//	if (line) free(line);
//}


