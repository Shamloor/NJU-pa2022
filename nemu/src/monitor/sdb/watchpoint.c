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

#include "sdb.h"

#define NR_WP 32

void expre(char *expression, bool *success);

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
	char *express;
	word_t res;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static int number = 1;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* Create a spare watchpoint from free_ linked list. */
void new_wp() {
	if (free_ == NULL) {
		assert(0);
	}

	WP *new_wp = free_;
	free_ = free_->next;
	new_wp->NO = number++;
	new_wp->next = head;
	head = new_wp;
}

/* Free watchpoint and return it to free_ linked list. */
void free_wp(int no) {
	WP *pre = NULL;
	WP *current = NULL;
	for (current = head;; current = current->next) {
		if (current == head && current->NO == no) {
			head = head->next;
			current->next = free_;
			free(current->express);
			free_ = current;
			break;
		} 
		else if (current != head && current->NO == no) {
			pre->next = current->next;
			current->next = free_;
			free(current->express);
			free_ = current;
			break;
		} 
		pre = current;
	}

	if (current == NULL) {
		printf("No number called %d exist.\n", no);
	}
}

/* Display information about head linked list. */
void display_wp() {
	if (head == NULL) {
		printf("No watchpoint exists.\n");
		return;
	}
	printf("Number	Expression	Result\n");
	for (WP *i = head; i != NULL; i = i->next) {
		printf("%d	%s	0x%x\n", i->NO, i->express, i->res);
	}
	return;
}

/* Compare res and result of expression. */
bool compare_res() {
	bool success = true;
	for (WP *i = head; i != NULL; i = i->next) {
		word_t res = expr(i->express, &success);
		if (i->res != res) {
			printf("Watchpoint %d has been triggered. \nThe value of %s : %x turned to %x\n", i->NO, i->express, i->res, res);
			i->res = res;
			return false;
		}
	}
	return true;
}

/* Set expression and res of head. */
void set_expre(char *expre) {
	head->express = (char *) malloc(15);
	strcpy(head->express, expre);
	bool success = true;
	head->res = expr(expre, &success);
}
