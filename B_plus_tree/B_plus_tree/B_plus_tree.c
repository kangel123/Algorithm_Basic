#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAXD_idx 4		
#define MAXD_data 4
#define D_idx 2	// 이것이 인덱스 노드의  capacity order  이다.
#define D_data 2   //   "    데이타 노드의    "       "
#define MAX 100

typedef char type_key[100];

typedef struct record {
	type_key name;
	int leng;
} type_rec;

typedef struct idxnode* type_ptr_idxnode;
typedef struct datanode* type_ptr_datanode;
typedef struct idxnode {  // index node
	type_key key[MAXD_idx];
	type_ptr_idxnode ptri[MAXD_idx + 1];
	type_ptr_datanode ptrd[MAXD_idx + 1];
	int fill_cnt;
} type_idx_node;

typedef struct datanode { // data node
	type_rec rec[MAXD_data];
	type_ptr_datanode link;
	int fill_cnt;
} type_data_node;

typedef struct big_node_index {	// 정상노드보다 공간이 키, 포인터 모두 1 많다.
	type_key key[MAXD_idx + 1];
	type_ptr_idxnode ptri[MAXD_idx + 2];
	type_ptr_datanode ptrd[MAXD_idx + 2];
}big_node_index;

typedef struct big_node_data {
	type_rec rec[MAXD_data + 1];
}big_node_data;

type_ptr_idxnode ROOT = NULL;
type_ptr_datanode HEAD = NULL;

type_ptr_idxnode stack[MAX];
int top = -1;

void push(type_ptr_idxnode nodeptr);
type_ptr_idxnode pop();
void insert_btree();  // file 전체를 삽입한다. 이 함수는 insert_arec을 호출한다. 
void retrieve(char*);
int insert_arec_b_plus_tree(type_rec in_rec);      // 레코드를 하나 삽입한다. 

int main(int argc, char* argv[])
{
	char* r_gets, line[200], name[100], ename[100];
	char command[3];
	int lleng, i, k, cnt;

	insert_btree();
	printf("Insertion from files has finished.\n");

	while (1) {
		printf("명령>");
		r_gets = gets(line);
		if (!r_gets) {
			printf("명령 입력 오류\n");
			getchar();
		}
		//scanf("%c %s", &command, name);

		// read a command consisting of two letters.
		if (strlen(line) < 2)
			continue;

		command[0] = line[0];
		command[1] = line[1];
		command[2] = '\0';

		if (strcmp(command, "ex") == 0)
			break;
		else if (strcmp(command, "sp") == 0) {
			sequential_print(HEAD);
			continue;
		}

		// 처음 이름을 읽는다.
		k = 3; i = 0;
		while (line[k] != '\0' && line[k] == ' ') k++; // 공백문자는 건너간다.
		while (line[k] != '\0' && line[k] != ' ') // read one key.
		{
			name[i] = line[k];
			k++;
			i++;
		}
		name[i] = '\0';

		if (strlen(name) < 1)
			continue;

		if (strcmp(command, "se") == 0) { // search one key.
			retrieve(name);
			printf("\n");
		}
		else if (strcmp(command, "rs") == 0) { // range search.
			// 두번째 이름 즉 최종키를 읽는다.
			while (line[k] != '\0' && line[k] == ' ') k++;  // 공백믄자를 건너간다.

			i = 0;
			while (line[k] != '\0' && line[k] != ' ') // read second key.
			{
				ename[i] = line[k];
				k++;
				i++;
			}
			ename[i] = '\0';
			if (strlen(ename) < 1)
				continue;

			cnt = range_search(name, ename);

			printf("출력한 총 레코드 수 = %d\n\n", cnt);
		}
		else
			printf("없는 명령어입니다.\n");
	}

	return 0;
}// main

// 파라메타 name 이 들어 있을 데이타 노드를 탐색하여 이 데이타노드의 포인터를 반환.
type_ptr_datanode get_datanode(char* name)
{
	int i, j;
	type_ptr_idxnode parent = NULL, curr = NULL;
	type_ptr_datanode curr_d = NULL;

	type_key in_key;
	strcpy(in_key, name);
	curr = ROOT;	//root의 값을 curr에 넣는다. 

	// 인덱스 구조 내비게이션.
	top = -1;	// 스택을 empty 로 초기화한다.
	if (ROOT->ptri[0] != NULL) {	// ROOT 가 리프노드가 아님. 
		// 인덱스구조를 navigation 해서 leaf level 까지 내려 가야 함
		do {
			for (i = 0; i < curr->fill_cnt; i++) {   // 자식으로 내려갈 포인터를 찾음.
				if (strcmp(in_key, curr->key[i]) <= 0)
					break; // 자식으로 내려 가기 위함
			}
			push(curr); // 나중에 부모를 찾기 위해 저장함.
			curr = curr->ptri[i];	// 자식으로 내려 감.
			if (curr->ptri[0] == NULL)
				break;	// 내려 간  새 curr 노드가 리프노드임.
		} while (1);
	}	// 인덱스구조 내비게이션.

	// curr 는 인덱스구조의 리프노드임. 여기서는 데이타노드로 내려가야 함.
	for (i = 0; i < curr->fill_cnt; i++) {
		if (strcmp(in_key, curr->key[i]) <= 0)
			break;
	}
	curr_d = curr->ptrd[i];
	return curr_d;

} // get_datanode

int range_search(char* key1, char* key2)
{
	int i, j, count = 0;
	FILE* fp;

	type_ptr_datanode ptr1, ptr2, tptr;
	if (strcmp(key1, key2) > 0) {
		printf("시작키가 종료키보다 더 뒤라서 불가능합니다.\n");
		return 0;
	}

	// start key 가 존재할 수 있는 데이타 노드에 대한 포인터를 구한다.
	ptr1 = get_datanode(key1);
	ptr2 = get_datanode(key2);

	fp = fopen("range_search_output.txt", "w");

	if (ptr1 == ptr2) { // start key 와 end key 가 동일 데이타노드에 속한다.
		// 스캔하면서 start key 나 그 바로 다음에서 멈춘다.
		for (i = 0; i < ptr1->fill_cnt; i++) {
			if (strcmp(key1, ptr1->rec[i].name) <= 0)
				break;
		}

		for (j = i; j < ptr1->fill_cnt && strcmp(ptr1->rec[j].name, key2) <= 0; j++) {
			fprintf(fp, "%s %d\n", ptr1->rec[j].name, ptr1->rec[j].leng);
			count++;
		}
		fclose(fp);
		return 0;
	}

	// ptr1 과 ptr2 가 다르다.
	// 그러면 먼저 ptr1 노드에서 key1 이후의 레코드들을 출력한다.
	for (i = 0; i < ptr1->fill_cnt; i++) { // 노드내에서 key1 도달 때까지 지나간다.
		if (strcmp(key1, ptr1->rec[i].name) <= 0)
			break;
	}

	for (j = i; j < ptr1->fill_cnt; j++) { // i 이후의 모든 레코드들을 출력한다.
		fprintf(fp, "%s %d\n", ptr1->rec[j].name, ptr1->rec[j].leng);
		fflush(fp);
		count++;
	}

	// ptr2 보다 앞의 노드들은  노드내의 모든 레코드들을 모두 출력한다.
	tptr = ptr1->link; // 다음 노드로 간다.
	while (tptr != ptr2) {
		// tptr 노드 내의 모든 레코드들을 출력한다.
		for (i = 0; i < tptr->fill_cnt; i++) {
			fprintf(fp, "%s %d\n", tptr->rec[i].name, tptr->rec[i].leng);
			fflush(fp);
			count++;
		}
		// tptr 을 다음 노드를 가리키게 한다.
		tptr = tptr->link;
	}

	// ptr2 노드의 내용을 처리할 차례이다. key2 이전(key2 포함)의 레코드들을 출력한다.
	for (i = 0; i < ptr2->fill_cnt && strcmp(ptr2->rec[i].name, key2) <= 0; i++) {
		fprintf(fp, "%s %d\n", ptr2->rec[i].name, ptr2->rec[i].leng);
		fflush(fp);
		count++;
	}

	fclose(fp);
	return count;
} // end of range search

void retrieve(char* name)
{
	int i, j;
	type_ptr_idxnode parent = NULL, curr = NULL;
	type_ptr_datanode curr_d = NULL;

	type_key in_key;
	strcpy(in_key, name);
	curr = ROOT;	//root의 값을 curr에 넣는다. 

	// 인덱스 구조 내비게이션.
	top = -1;	// 스택을 empty 로 초기화한다.
	if (ROOT->ptri[0] != NULL) {	// ROOT 가 리프노드가 아님. 
		// 인덱스구조를 navigation 해서 leaf level 까지 내려 가야 함
		do {
			for (i = 0; i < curr->fill_cnt; i++) {   // 자식으로 내려갈 포인터를 찾음.
				if (strcmp(in_key, curr->key[i]) <= 0)
					break; // 자식으로 내려 가기 위함
			}
			push(curr); // 나중에 부모를 찾기 위해 저장함.
			curr = curr->ptri[i];	// 자식으로 내려 감.
			if (curr->ptri[0] == NULL)
				break;	// 내려 간  새 curr 노드가 리프노드임.
		} while (1);
	}	// 인덱스구조 내비게이션.

	// curr 는 인덱스구조의 리프노드임. 여기서는 데이타노드로 내려가야 함.
	for (i = 0; i < curr->fill_cnt; i++) {
		if (strcmp(in_key, curr->key[i]) <= 0)
			break;
	}
	parent = curr; // 리프노드 저장. (주의: 스택에 넣지는 않음.)
	curr_d = curr->ptrd[i]; // 데이타노드로 내려 감. curr_d 은 데이타노드를 가리킴.

	// 데이타노드에서 해당 키를 가진 레코드를 찾는다
	for (i = 0; i < curr_d->fill_cnt; i++) {
		if (strcmp(in_key, curr_d->rec[i].name) < 0) {
			printf("존재하지 않습니다.\n");
			break;  // 이 레코드에서 발견 안됨.
		}
		else if (strcmp(in_key, curr_d->rec[i].name) == 0)
		{
			printf("탐색성공. 이름= %s, 길이=%d\n", curr_d->rec[i].name, curr_d->rec[i].leng);
			break;
		}
		else; // try next i.
	}

	return;
}

/*******************************************************/
int insert_arec_b_plus_tree(type_rec in_rec) {	//하나의 레코드를 삽입.

	int i, j;
	type_ptr_idxnode parent = NULL, curr = NULL, child = NULL, new_ptri = NULL, tptr = NULL;
	type_ptr_datanode curr_d = NULL, new_ptrd = NULL, child_d = NULL;
	big_node_index bnode_index;
	big_node_data bnode_data;
	int fc, down_idx;

	type_key in_key;
	strcpy(in_key, in_rec.name);  // 이 데이타의 경우 이름이 키임.

	if (!ROOT)
	{	// 처음으로 하나의 레코드를 삽입하게 된다.
		ROOT = (type_ptr_idxnode)malloc(sizeof(type_idx_node));
		HEAD = (type_ptr_datanode)malloc(sizeof(type_data_node));

		HEAD->rec[0] = in_rec; // data node 에 첫 레코드를 넣음.
		HEAD->fill_cnt = 1;
		HEAD->link = NULL;

		ROOT->ptri[0] = NULL; // 리프노드임을 나타냄.
		ROOT->ptrd[0] = HEAD;
		ROOT->ptrd[1] = NULL;  //  초기상황임을 나타냄.
		strcpy(ROOT->key[0], in_rec.name);		// 키를 인덱스 노드에  넣음.
		ROOT->fill_cnt = 1;
		return 1;  // 첫 레코드의 넣기 종료.
	}
	else if (!ROOT->ptri[0] && !ROOT->ptrd[1])
	{   // 초기상황임.
		fc = HEAD->fill_cnt;
		if (fc < MAXD_data) {	// 아직 full 은 아님. 적당한 오름 차순 위치에 넣음.
			for (i = 0; i < fc; i++) {
				if (strcmp(in_key, HEAD->rec[i].name) < 0)
					break;  // i 가 결정됨.
				else if (strcmp(in_key, HEAD->rec[i].name) == 0) {
					printf("동일키 이미 존재로 삽입 실패! \n"); return 0;
				}
				else; // try next i.
			}
			// i 부터 우측을 한칸씩 우측으로 시프트함.
			for (j = fc; j > i; j--) HEAD->rec[j] = HEAD->rec[j - 1];
			HEAD->rec[i] = in_rec; // 새 레코드를 넣음.
			HEAD->fill_cnt++;
			strcpy(ROOT->key[0], HEAD->rec[fc].name);	// 데이타노드의 마지막 레코드의 키를 부모에게 넣어줌. 여기 중요!!
			return 1; // 성공으로 종료.
		}
		else
		{   // 첫 데이타노드가 full 임. split 가 필요함.
			// big data node 를 준비함
			for (i = 0; i < MAXD_data; i++)
			{
				if (strcmp(in_key, HEAD->rec[i].name) < 0) break;
				else if (strcmp(in_key, HEAD->rec[i].name) == 0) { printf("동일키 이미 존재로 삽입 실패! \n"); return 0; }
				else; // try next i.
			}
			for (j = 0; j < i; j++) // i 이전 부븐을 빅노드로 가져옴.
				bnode_data.rec[j] = HEAD->rec[j];
			bnode_data.rec[j] = in_rec;
			j++;
			while (i < MAXD_data)
			{
				bnode_data.rec[j] = HEAD->rec[i];
				j++;		//i와 j를 증가 시킨다. 
				i++;
			}
			// index of center record is at D_data. center 및 그 좌측은 노드 HEAD 로 옮긴다.
			for (i = 0; i <= D_data; i++)
				HEAD->rec[i] = bnode_data.rec[i];
			HEAD->fill_cnt = D_data + 1;

			
			new_ptrd = (type_ptr_datanode)malloc(sizeof(type_data_node));
			for (j = 0; i <= MAXD_data; i++, j++) 
				new_ptrd->rec[j] = bnode_data.rec[i];
			new_ptrd->fill_cnt = D_data;
		
			strcpy(ROOT->key[0], HEAD->rec[D_data].name); // 앞 데이타노드의 맨 뒤 레코드의 키를 부모에 넣음
			ROOT->ptrd[1] = new_ptrd;	// 이 작업으로 지금부터 정상적인 B+  트리가 됨.

			HEAD->link = new_ptrd;	// 연결리스트에 새 노드를 연결함.
			new_ptrd->link = NULL;	// 새 데이타 노드가 연결리스트의 마지막 노드임.
			return 1; // 성공으로 종료.
		}
	} // 초기상황 삽입 종료.

	// (ROOT가 NULL 이거나 초기상황임)이 아니면 여기로 온다. 즉 B-tree 인덱스는 정상 B-tree 임.
	// 먼저 탐색을 통하여 in_rec 이 들어갈 데이타노드를 찾는다. 

	curr = ROOT;	//root의 값을 curr에 넣는다. 

	// 인덱스 구조 내비게이션.
	top = -1;	// 스택을 empty 로 초기화한다.
	if (ROOT->ptri[0] != NULL) {	// ROOT 가 리프노드가 아님. 
		do {
			for (i = 0; i < curr->fill_cnt; i++) {   // 자식으로 내려갈 포인터를 찾음.
				if (strcmp(in_key, curr->key[i]) <= 0)
					break; // 자식으로 내려 가기 위함
			}
			push(curr); // 나중에 부모를 찾기 위해 저장함.
			curr = curr->ptri[i];	// 자식으로 내려 감.
			if (curr->ptri[0] == NULL)
				break;	// 내려 간  새 curr 노드가 리프노드임.
		} while (1);
	}	// 인덱스구조 내비게이션.

	// curr 는 인덱스구조의 리프노드임. 여기서는 데이타노드로 내려가야 함.
	for (i = 0; i < curr->fill_cnt; i++)
		if (strcmp(in_key, curr->key[i]) <= 0)
			break;

	parent = curr; // 리프노드 저장. (주의: 스택에 넣지는 않음.)
	curr_d = curr->ptrd[i]; // 데이타노드로 내려 감. curr_d 은 데이타노들 가리킴.
	down_idx = i;	// 부모에서 자식으로 내려간 인덱스 저장. 나중에 이용할 것임.

	// 이 데이타노드(curr_d)에 in_rec 레코드를 넣어야 함.

	fc = curr_d->fill_cnt;
	if (fc < MAXD_data) 	// 아직 full 은 아님. 적당한 오름 차순 위치에 넣음.
	{
		for (i = 0; i < fc; i++) {
			if (strcmp(in_key, curr_d->rec[i].name) < 0)
				break;  // i 가 결정됨.
			else if (strcmp(in_key, curr_d->rec[i].name) == 0)
			{
				printf("동일키 이미 존재로 삽입 실패!\n");
				return 0;
			}
			else; // try next i.
		}
		// i 부터 우측을 한칸씩 우측으로 시프트함.
		for(j=fc;j>i;j--)
			curr_d->rec[j] = curr_d->rec[j - 1];

		curr_d->rec[i] = in_rec; // 새 레코드를 넣음.
		curr_d->fill_cnt++;

		if (down_idx < parent->fill_cnt)	// 마지막 포인터로 내려 온 경우는 부모에 반영하지 않음.
			strcpy(parent->key[down_idx], curr_d->rec[fc].name);// 데이타노드의 마지막 레코드의 키를 부모에게 넣어줌. 여기 중요!!
		return 1; // 성공으로 종료.
	}
	else
	{ 	// 첫 데이타노드가 full 임. split 가 필요함. big data node 를 준비함
		for (i = 0; i < MAXD_data; i++) {
			if (strcmp(in_key, curr_d->rec[i].name) < 0) break;
			else if (strcmp(in_key, curr_d->rec[i].name) == 0)
			{
				printf("동일키 이미 존재로 삽입 실패!\n");
				return 0;
			}
			else; // try next i.
		}
		for (j = 0; j < i; j++) // i 이전 부분을 빅노드로 가져옴.
			bnode_data.rec[j] = curr_d->rec[j];
		bnode_data.rec[j] = in_rec;
		j++;
		while (i < MAXD_data) {
			bnode_data.rec[j] = curr_d->rec[i];
			j++;		//i와 j를 증가 시킨다. 
			i++;
		}
		// center record is at index D_data. center 및 그 좌측은 노드 curr_d 로 옮긴다.
		for (i = 0; i <= D_data; i++) curr_d->rec[i] = bnode_data.rec[i];
		curr_d->fill_cnt = D_data + 1;

		new_ptrd = (type_ptr_datanode)malloc(sizeof(type_data_node));
		for (i = 0; i < D_data; i++)	// 나머지 뒤는 새로운 노드에 넣음.
			new_ptrd->rec[i] = bnode_data.rec[i + 1 +D_data];
		new_ptrd->fill_cnt = D_data;
		new_ptrd->link = NULL;

		// 시퀀스셋 연결리스트에 이 새로운 노드를 curr 다음 노드가 되게 끼워 넣어야 한다.
		new_ptrd->link = curr_d->link;
		curr_d->link = new_ptrd;

		// 쌍 [center키, new_ptrd] 을 인덱스구조의 리프노드인 부모 노드에 넣는 새로운 문제가 생긴다.
		// 아래 3 줄로 변수 설정후에 아래의 b-tree 삽입 루프로 간다.
		curr = parent;
		strcpy(in_key, curr_d->rec[D_data].name);	// 쌍의 원소 1.
		child_d = new_ptrd;					// 쌍의 원소 2.
	} // 데이타노드가 full 인 곳에 삽입.

	// 여기서 부터는 B-tree 에 하나의 쌍을 넣는 작업을 한다.

	do {
		// 여기에 오기 전에 변수 in_key 에 넣을 키,  변수 child_d 또는 child 에 포인터를 설정하고 온다.
		// The pair <in_key, child_d> 또는 <in_key, child>  를 curr 에 넣어야 한다. 
		// child_d: 데이타노드를 포인트,  child: 인덱스노드를 포인트. 둘 중 무엇을 이용할지는 curr의 리프/비리프 여부로 결정.

		if (curr->fill_cnt < MAXD_idx)	// curr 에 빈자리가 있다. 이 curr 노드에 넣는다.
		{
			for (i = 0; i < curr->fill_cnt; i++) //i를 curr->fill_cnt까지 증가하면서 조사한다.
				if (strcmp(in_key, curr->key[i]) < 0) //i 에서의 키가 in_key 보다 더 크면 멈춘다. 
					break;

			for (j = curr->fill_cnt; j > i; j--) { // i 부터 우측 부분을 포인터와 키를  한칸씩 우로 시프트 한다.
				curr->ptrd[j + 1] = curr->ptrd[j];
				curr->ptri[j + 1] = curr->ptri[j];
				strcpy(curr->key[j], curr->key[j - 1]);
			}

			strcpy(curr->key[i], in_key);	// 키 삽입 
			if (curr->ptri[0] == NULL) {	// curr 는 리프노드이므로 type-1 삽입이다.
				curr->ptrd[i + 1] = child_d;	// 자식 포인터를 ptrd 에 삽입 
				curr->ptri[i + 1] = NULL;
			}
			else { // curr 는 리프노드가 아니다. 고로 type-2 삽입이다. 자식 포인터를 ptri 에 넣는다.
				curr->ptrd[i + 1] = NULL;
				curr->ptri[i + 1] = child;
			}
			curr->fill_cnt++;         // fill 카운트 증가시킴 

			return 1; // 삽입 성공으로 종료.
		}
		else 	//   curr 에 빈자리가 없다. 인덱스노드의 빅노드 이용하여 split & elevation 을 수행한다.
		{
			for (i = 0; i < MAXD_idx; i++) {
				if (strcmp(in_key, curr->key[i]) < 0) // 만약 i 의 키가 in_key가 크면 멈춤
					break;
			}

			bnode_index.ptri[0] = curr->ptri[0]; // 먼저 포인터 0 을 옮김.
			bnode_index.ptrd[0] = curr->ptrd[0]; //   "   "

			for (j = 0; j < i; j++) {	// i 전까지 curr 의 내용을 옮김.
				strcpy(bnode_index.key[j], curr->key[j]);
				bnode_index.ptri[j + 1] = curr->ptri[j + 1];
				bnode_index.ptrd[j + 1] = curr->ptrd[j + 1];
			}

			strcpy(bnode_index.key[j], in_key); // 쌍의 첫 원소 in_key 를 넣어 준다.
			// 쌍의 원소2 인  포인터를넣어 준다.
			if (curr->ptri[0] == NULL) { // curr 는 리프노드이므로 type-1 삽입이다.
				bnode_index.ptrd[j + 1] = child_d;   // 자식 포인터를 ptrd 에 삽입 
				bnode_index.ptri[j + 1] = NULL;  // 이것은 넣지 않아도 문제는 없다.
			}
			else { // curr 는 리프노드가 아니다. 고로 type-2 삽입이다. 자식 포인터를 ptri 에 넣는다.
				bnode_index.ptri[j + 1] = child;
				bnode_index.ptrd[j + 1] = NULL;  // 이것은 넣지 않아도 동작은 잘 한다.
			}

			j++; // j 증가시킴 

			while (i < MAXD_idx) {
				strcpy(bnode_index.key[j], curr->key[i]);  //curr의 레코드를 bnode 로 복사
				bnode_index.ptri[j + 1] = curr->ptri[i + 1]; // 그 우측의 포인터도 복사.
				bnode_index.ptrd[j + 1] = curr->ptrd[i + 1];
				j++;		//i와 j를 증가 시킨다. 
				i++;
			}

			// 빅노드를 반으로 나누어 전반부를 curr 에 넣는다.
			for (i = 0; i < D_idx; i++) {
				curr->ptri[i] = bnode_index.ptri[i];    //빅노드의 앞의 반을 현재 노드에 넣어준다. [레코드, 포인터]
				curr->ptrd[i] = bnode_index.ptrd[i];
				strcpy(curr->key[i], bnode_index.key[i]);
			}

			curr->ptri[i] = bnode_index.ptri[i];		//bnode.ptr[i]는 curr.ptr[i]를 가르킨다. 
			curr->ptrd[i] = bnode_index.ptrd[i];
			curr->fill_cnt = D_idx;

			new_ptri = (type_ptr_idxnode)malloc(sizeof(type_idx_node)); // 빅노드의 후반부를 새로운 노드에 넣는다. 

			for (i = 0; i < D_idx; i++) {	//뒤의 반을 새노드에 저장 
				new_ptri->ptri[i] = bnode_index.ptri[i + 1 + D_idx];
				new_ptri->ptrd[i] = bnode_index.ptrd[i + 1 + D_idx];
				strcpy(new_ptri->key[i], bnode_index.key[i + 1 + D_idx]);
			}
			new_ptri->ptri[i] = bnode_index.ptri[i + 1 + D_idx];	// 맨 마지막 포인터를 옮긴다.
			new_ptri->ptrd[i] = bnode_index.ptrd[i + 1 + D_idx];
			new_ptri->fill_cnt = D_idx;

			strcpy(in_key, bnode_index.key[D_idx]);	// 빅노드의 중간 키를 elevation 할 키로 함
			child = new_ptri;					// 새로 할당한 노드의 포인터를 elevation 할 포인터로 함.

			if (top >= 0) {	// curr의 부모가 있음.  
				curr = pop();	//  curr 의 부모를 스택에서 가져 와서 그것으로 curr 를 변경.
				// 새로운 curr 에 쌍 [in_key, child] 을 넣는 작업을 수행할 예정임.
			}
			else {    // curr 가 ROOT 이다. 부모가 없다. 부모를 만들고 그것이 부모가 되어야 한다.
				tptr = (type_ptr_idxnode)malloc(sizeof(type_idx_node));
				strcpy(tptr->key[0], in_key);
				tptr->ptri[0] = curr;
				tptr->ptri[1] = new_ptri;
				tptr->ptrd[0] = NULL;
				tptr->ptrd[1] = NULL;
				tptr->fill_cnt = 1;
				ROOT = tptr;	// 루트를 변경.
				return 1; // 삽입 성공으로 종료.
			} // else.
		} // else.
	} while (1);

} //함수 insert_arec_b_plus_tree

void insert_btree() {
	FILE* fp;
	type_rec data;
	char name_i[100];
	int n = 0;
	int check, count = 0;
	int success;

	fp = fopen("Com_names1.txt", "r");
	if (fp == NULL) {
		printf("Cannot open this file : Com_names1.txt\n");
		scanf("%d", &check);

	} //if 


	while (1) {
		n = fscanf(fp, "%s", name_i);
		if (n < 1)
			break;
		strcpy(data.name, name_i);
		data.leng = strlen(name_i);
		success = insert_arec_b_plus_tree(data);
		if (success)
			count++; // increment insertion count.

	} //while 
	fclose(fp);

	fp = fopen("Com_names2.txt", "r");
	if (fp == NULL) {
		printf("Cannot open this file : Com_names2.txt\n");
		scanf("%d", &check);

	} //if 


	while (1) {
		n = fscanf(fp, "%s", name_i);
		if (n < 1)
			break;
		strcpy(data.name, name_i);
		data.leng = strlen(name_i);
		success = insert_arec_b_plus_tree(data);
		if (success)
			count++; // increment insertion count.

	} //while 
	fclose(fp);
	printf("data input is finished.\n");
	printf("record count stored = %d \n", count);
	
}   //  함수 insert_btree

void push(type_ptr_idxnode node) {
	if (top > MAX) ///>>>  MAX-1 이상( >= ) 이어야 하지 않나?
	{
		printf("stack is full\n");
		return;
	}
	top++;
	stack[top] = node;

}

type_ptr_idxnode pop() {
	int temp = 0;
	if (top < 0)
	{
		printf("stack is empty.\n");
		getchar();
	}

	temp = top; ///>>> 현재 top 이 가리키는 곳에 가장 최근의 데이타가 저장되어 있다. 따라서,
	top--;
	return stack[temp];
}

int sequential_print(type_ptr_datanode HEAD) {
	int i;
	int count = 0;
	FILE* fp;

	fp = fopen("sequential_print .txt", "w");

	while (HEAD != NULL) {
		for (i = 0; i < HEAD->fill_cnt; i++) {
			fprintf(fp, "%s %d\n", HEAD->rec[i].name, HEAD->rec[i].leng);
			fflush(fp);
			count++;
		}
		HEAD = HEAD->link;
	}
	fclose(fp);
	return count;
}