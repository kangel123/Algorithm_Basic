#include <stdio.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS

typedef char BTData;
typedef struct _BinTree{
	BTData key;
	struct _BinTree* left;
	struct _BinTree* right;
} BinTree;

// BST�˻�
BinTree* searchBST(BinTree* bt, BTData x) {
	BinTree* p = bt;

	while (p != NULL){
		if (p->key == x)
			return p;
		else if (p->key > x)
			p = p->left;
		else
			p = p->right;
	}
	return NULL;
}

// ��� ���
BinTree* insertBST(BinTree* bt, BTData x) {
	BinTree* p = bt;
	BinTree* root = NULL;

	while(p != NULL){
		root = p;
		if (p->key == x)
			return p;
		else if (p->key > x)
			p = p->left;
		else
			p = p->right;
	}

	BinTree* newNode = (BinTree*)malloc(sizeof(BinTree));
	newNode->key = x;
	newNode->left = NULL;
	newNode->right = NULL;

	if (root != NULL) {
		if (root->key > newNode->key)
			root->left = newNode;
		else
			root->right = newNode;
	}
	return newNode;
}

// ��� ����
BinTree* deleteBST(BinTree* bt, BTData x) {
	// p : ������ ���
	// root : p�� �θ���
	BinTree* p = bt;
	BinTree* root = NULL;
	
	while((p != NULL)&&(p->key != x)){
		root = p;
		if (p->key > x)
			p = p->left;
		else
			p = p->right;
	}
	if (p == NULL) {
		printf("������ Ű�� �������� �ʽ��ϴ�.\n");
		return bt;
	}
	
	//������ 0�� ���
	if ((p->left == NULL)&&(p->right == NULL)){
		if (root == NULL)
			bt = NULL;
		else {
			if (root->left == p)
				root->left = NULL;
			else
				root->right = NULL;
		}
			
	}
	//������ 1�� ���
	else if ((p->left == NULL)||(p->right == NULL)) {
		BinTree* child = (p->left != NULL) ? p->left : p->right;
		if (root == NULL)
			bt = child;
		else {
			if (root->left == p)
				root->left = child;
			else
				root->right = child;
		}
	}
	//������ 2�� ���
	else {
		//succ : ������ ����� ���� ����Ʈ�� �� ���� ���� ���
		//succ_root : succ����� �θ���

		BinTree* succ_root = p;
		BinTree* succ = p->right;

		while (succ->left != NULL) {
			succ_root = succ;
			succ = succ->left;
		}
		
		p->key = succ->key;
		
		//������ succ����� �θ���� �ڽĳ�� ����
		if (succ_root->left == succ)
			succ_root->left = succ->right;
		else
			succ_root->right = succ->right;

		p = succ;
	}
	free(p);
	return bt;
}

void retrieval(BinTree* bt)  {
	BinTree* p = bt;

	if (p == NULL)
		return;

	retrieval(p->left);
	printf("%c ", p->key);
	retrieval(p->right);
}

int main() {
	// �����˻�Ʈ�� BST����
	BinTree* bt = insertBST(NULL, 'M');
	insertBST(bt, 'I');
	insertBST(bt, 'R');
	insertBST(bt, 'A');
	insertBST(bt, 'E');
	retrieval(bt); printf("\n");

	// Ʈ�� ����
	bt = deleteBST(bt, 'R');
	retrieval(bt); printf("\n");

	// Ʈ�� �˻�
	char keyword = NULL;
	printf("ã�� ���� �Է��ϼ��� -->"); scanf_s("%c", &keyword);

	if (searchBST(bt, keyword))
		printf("ã�ҽ��ϴ�.\n");
	else
		printf("�����ϴ�.\n");

	printf("�����Ϸ��� �ƹ�Ű�� �Է��ϼ���.\n");

	return 0;
}