#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct Node {
	int data;
	struct Node *next;
} *s_head = NULL;

int list_add(int data)
{
	struct Node *plNode = (struct Node *)malloc(sizeof(struct Node));

	plNode->data = data;
	plNode->next = s_head;
	s_head = plNode;

	return 0;
}

int list_del(int data)
{
	struct Node *plNode;
	struct Node *plPreNode;

	if (!s_head) {
		return -1;	
	}

	plNode = s_head;
	plPreNode = s_head;
	if (s_head->data == data) {
		s_head = s_head->next;
		free(plNode);
		return 0;
	}

	while (plNode = plNode->next) {
		if (plNode->data == data) {
			plPreNode->next = plNode->next;
			free(plNode);
			return 0;
		}
		plPreNode = plNode;
	}

	return -1;
}

void list_print(struct Node *head)
{
	printf("list mem:");
	while (head) {
		printf("%d ", head->data);
		head = head->next;
	}
	printf("\n");
}

struct Node *list_reverse(struct Node *head)
{
	struct Node *plPreNode, *plCurNode;

	if (!head || !head->next) 
		return head;

	plCurNode = head->next;
	head->next = NULL;
	while (plCurNode) {
		plPreNode = head;
		head = plCurNode;
		plCurNode = plCurNode->next;
		head->next = plPreNode;
	}

	return head;
}

int main(int argc, char *argv[])
{
	int i;

	for (i=0; i<8; i++) {
		list_add(i);
	}
	printf("=====add list!\n");
	list_print(s_head);
	for (i=2; i<=5; i++) {
		list_del(i);
	}
	printf("=====del list!\n");
	list_print(s_head);
	for (i=0; i<8; i++) {
		list_del(i);
	}
	printf("=====del all list!\n");
	list_print(s_head);

	for (i=0; i<8; i++) {
		list_add(i);
	}
	printf("=====add list!\n");
	list_print(s_head);
	printf("=====reverse list!\n");
	list_print(list_reverse(s_head));

	exit(0);
}
