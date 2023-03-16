#include <stdio.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS

typedef char BTData;
typedef struct _BinTree{
	BTData key;
	struct _BinTree* left;
	struct _BinTree* right;
} BinTree;

// BST검색
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

// 노드 등록
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

// 노드 삭제
BinTree* deleteBST(BinTree* bt, BTData x) {
	// p : 삭제할 노드
	// root : p의 부모노드
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
		printf("삭제할 키가 존재하지 않습니다.\n");
		return bt;
	}
	
	//차수가 0인 경우
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
	//차수가 1인 경우
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
	//차수가 2인 경우
	else {
		//succ : 삭제할 노드의 왼쪽 서브트리 중 가장 작은 노드
		//succ_root : succ노드의 부모노드

		BinTree* succ_root = p;
		BinTree* succ = p->right;

		while (succ->left != NULL) {
			succ_root = succ;
			succ = succ->left;
		}
		
		p->key = succ->key;
		
		//삭제할 succ노드의 부모노드와 자식노드 연결
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
	// 이진검색트리 BST생성
	BinTree* bt = insertBST(NULL, 'M');
	insertBST(bt, 'I');
	insertBST(bt, 'R');
	insertBST(bt, 'A');
	insertBST(bt, 'E');
	retrieval(bt); printf("\n");

	// 트리 삭제
	bt = deleteBST(bt, 'R');
	retrieval(bt); printf("\n");

	// 트리 검색
	char keyword = NULL;
	printf("찾을 값을 입력하세요 -->"); scanf_s("%c", &keyword);

	if (searchBST(bt, keyword))
		printf("찾았습니다.\n");
	else
		printf("없습니다.\n");

	printf("종료하려면 아무키나 입력하세요.\n");

	return 0;
}