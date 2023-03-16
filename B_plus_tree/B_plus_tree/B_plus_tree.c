#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAXD_idx 4		
#define MAXD_data 4
#define D_idx 2	// �̰��� �ε��� �����  capacity order  �̴�.
#define D_data 2   //   "    ����Ÿ �����    "       "
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

typedef struct big_node_index {	// �����庸�� ������ Ű, ������ ��� 1 ����.
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
void insert_btree();  // file ��ü�� �����Ѵ�. �� �Լ��� insert_arec�� ȣ���Ѵ�. 
void retrieve(char*);
int insert_arec_b_plus_tree(type_rec in_rec);      // ���ڵ带 �ϳ� �����Ѵ�. 

int main(int argc, char* argv[])
{
	char* r_gets, line[200], name[100], ename[100];
	char command[3];
	int lleng, i, k, cnt;

	insert_btree();
	printf("Insertion from files has finished.\n");

	while (1) {
		printf("���>");
		r_gets = gets(line);
		if (!r_gets) {
			printf("��� �Է� ����\n");
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

		// ó�� �̸��� �д´�.
		k = 3; i = 0;
		while (line[k] != '\0' && line[k] == ' ') k++; // ���鹮�ڴ� �ǳʰ���.
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
			// �ι�° �̸� �� ����Ű�� �д´�.
			while (line[k] != '\0' && line[k] == ' ') k++;  // ������ڸ� �ǳʰ���.

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

			printf("����� �� ���ڵ� �� = %d\n\n", cnt);
		}
		else
			printf("���� ��ɾ��Դϴ�.\n");
	}

	return 0;
}// main

// �Ķ��Ÿ name �� ��� ���� ����Ÿ ��带 Ž���Ͽ� �� ����Ÿ����� �����͸� ��ȯ.
type_ptr_datanode get_datanode(char* name)
{
	int i, j;
	type_ptr_idxnode parent = NULL, curr = NULL;
	type_ptr_datanode curr_d = NULL;

	type_key in_key;
	strcpy(in_key, name);
	curr = ROOT;	//root�� ���� curr�� �ִ´�. 

	// �ε��� ���� ������̼�.
	top = -1;	// ������ empty �� �ʱ�ȭ�Ѵ�.
	if (ROOT->ptri[0] != NULL) {	// ROOT �� ������尡 �ƴ�. 
		// �ε��������� navigation �ؼ� leaf level ���� ���� ���� ��
		do {
			for (i = 0; i < curr->fill_cnt; i++) {   // �ڽ����� ������ �����͸� ã��.
				if (strcmp(in_key, curr->key[i]) <= 0)
					break; // �ڽ����� ���� ���� ����
			}
			push(curr); // ���߿� �θ� ã�� ���� ������.
			curr = curr->ptri[i];	// �ڽ����� ���� ��.
			if (curr->ptri[0] == NULL)
				break;	// ���� ��  �� curr ��尡 ���������.
		} while (1);
	}	// �ε������� ������̼�.

	// curr �� �ε��������� ���������. ���⼭�� ����Ÿ���� �������� ��.
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
		printf("����Ű�� ����Ű���� �� �ڶ� �Ұ����մϴ�.\n");
		return 0;
	}

	// start key �� ������ �� �ִ� ����Ÿ ��忡 ���� �����͸� ���Ѵ�.
	ptr1 = get_datanode(key1);
	ptr2 = get_datanode(key2);

	fp = fopen("range_search_output.txt", "w");

	if (ptr1 == ptr2) { // start key �� end key �� ���� ����Ÿ��忡 ���Ѵ�.
		// ��ĵ�ϸ鼭 start key �� �� �ٷ� �������� �����.
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

	// ptr1 �� ptr2 �� �ٸ���.
	// �׷��� ���� ptr1 ��忡�� key1 ������ ���ڵ���� ����Ѵ�.
	for (i = 0; i < ptr1->fill_cnt; i++) { // ��峻���� key1 ���� ������ ��������.
		if (strcmp(key1, ptr1->rec[i].name) <= 0)
			break;
	}

	for (j = i; j < ptr1->fill_cnt; j++) { // i ������ ��� ���ڵ���� ����Ѵ�.
		fprintf(fp, "%s %d\n", ptr1->rec[j].name, ptr1->rec[j].leng);
		fflush(fp);
		count++;
	}

	// ptr2 ���� ���� ������  ��峻�� ��� ���ڵ���� ��� ����Ѵ�.
	tptr = ptr1->link; // ���� ���� ����.
	while (tptr != ptr2) {
		// tptr ��� ���� ��� ���ڵ���� ����Ѵ�.
		for (i = 0; i < tptr->fill_cnt; i++) {
			fprintf(fp, "%s %d\n", tptr->rec[i].name, tptr->rec[i].leng);
			fflush(fp);
			count++;
		}
		// tptr �� ���� ��带 ����Ű�� �Ѵ�.
		tptr = tptr->link;
	}

	// ptr2 ����� ������ ó���� �����̴�. key2 ����(key2 ����)�� ���ڵ���� ����Ѵ�.
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
	curr = ROOT;	//root�� ���� curr�� �ִ´�. 

	// �ε��� ���� ������̼�.
	top = -1;	// ������ empty �� �ʱ�ȭ�Ѵ�.
	if (ROOT->ptri[0] != NULL) {	// ROOT �� ������尡 �ƴ�. 
		// �ε��������� navigation �ؼ� leaf level ���� ���� ���� ��
		do {
			for (i = 0; i < curr->fill_cnt; i++) {   // �ڽ����� ������ �����͸� ã��.
				if (strcmp(in_key, curr->key[i]) <= 0)
					break; // �ڽ����� ���� ���� ����
			}
			push(curr); // ���߿� �θ� ã�� ���� ������.
			curr = curr->ptri[i];	// �ڽ����� ���� ��.
			if (curr->ptri[0] == NULL)
				break;	// ���� ��  �� curr ��尡 ���������.
		} while (1);
	}	// �ε������� ������̼�.

	// curr �� �ε��������� ���������. ���⼭�� ����Ÿ���� �������� ��.
	for (i = 0; i < curr->fill_cnt; i++) {
		if (strcmp(in_key, curr->key[i]) <= 0)
			break;
	}
	parent = curr; // ������� ����. (����: ���ÿ� ������ ����.)
	curr_d = curr->ptrd[i]; // ����Ÿ���� ���� ��. curr_d �� ����Ÿ��带 ����Ŵ.

	// ����Ÿ��忡�� �ش� Ű�� ���� ���ڵ带 ã�´�
	for (i = 0; i < curr_d->fill_cnt; i++) {
		if (strcmp(in_key, curr_d->rec[i].name) < 0) {
			printf("�������� �ʽ��ϴ�.\n");
			break;  // �� ���ڵ忡�� �߰� �ȵ�.
		}
		else if (strcmp(in_key, curr_d->rec[i].name) == 0)
		{
			printf("Ž������. �̸�= %s, ����=%d\n", curr_d->rec[i].name, curr_d->rec[i].leng);
			break;
		}
		else; // try next i.
	}

	return;
}

/*******************************************************/
int insert_arec_b_plus_tree(type_rec in_rec) {	//�ϳ��� ���ڵ带 ����.

	int i, j;
	type_ptr_idxnode parent = NULL, curr = NULL, child = NULL, new_ptri = NULL, tptr = NULL;
	type_ptr_datanode curr_d = NULL, new_ptrd = NULL, child_d = NULL;
	big_node_index bnode_index;
	big_node_data bnode_data;
	int fc, down_idx;

	type_key in_key;
	strcpy(in_key, in_rec.name);  // �� ����Ÿ�� ��� �̸��� Ű��.

	if (!ROOT)
	{	// ó������ �ϳ��� ���ڵ带 �����ϰ� �ȴ�.
		ROOT = (type_ptr_idxnode)malloc(sizeof(type_idx_node));
		HEAD = (type_ptr_datanode)malloc(sizeof(type_data_node));

		HEAD->rec[0] = in_rec; // data node �� ù ���ڵ带 ����.
		HEAD->fill_cnt = 1;
		HEAD->link = NULL;

		ROOT->ptri[0] = NULL; // ����������� ��Ÿ��.
		ROOT->ptrd[0] = HEAD;
		ROOT->ptrd[1] = NULL;  //  �ʱ��Ȳ���� ��Ÿ��.
		strcpy(ROOT->key[0], in_rec.name);		// Ű�� �ε��� ��忡  ����.
		ROOT->fill_cnt = 1;
		return 1;  // ù ���ڵ��� �ֱ� ����.
	}
	else if (!ROOT->ptri[0] && !ROOT->ptrd[1])
	{   // �ʱ��Ȳ��.
		fc = HEAD->fill_cnt;
		if (fc < MAXD_data) {	// ���� full �� �ƴ�. ������ ���� ���� ��ġ�� ����.
			for (i = 0; i < fc; i++) {
				if (strcmp(in_key, HEAD->rec[i].name) < 0)
					break;  // i �� ������.
				else if (strcmp(in_key, HEAD->rec[i].name) == 0) {
					printf("����Ű �̹� ����� ���� ����! \n"); return 0;
				}
				else; // try next i.
			}
			// i ���� ������ ��ĭ�� �������� ����Ʈ��.
			for (j = fc; j > i; j--) HEAD->rec[j] = HEAD->rec[j - 1];
			HEAD->rec[i] = in_rec; // �� ���ڵ带 ����.
			HEAD->fill_cnt++;
			strcpy(ROOT->key[0], HEAD->rec[fc].name);	// ����Ÿ����� ������ ���ڵ��� Ű�� �θ𿡰� �־���. ���� �߿�!!
			return 1; // �������� ����.
		}
		else
		{   // ù ����Ÿ��尡 full ��. split �� �ʿ���.
			// big data node �� �غ���
			for (i = 0; i < MAXD_data; i++)
			{
				if (strcmp(in_key, HEAD->rec[i].name) < 0) break;
				else if (strcmp(in_key, HEAD->rec[i].name) == 0) { printf("����Ű �̹� ����� ���� ����! \n"); return 0; }
				else; // try next i.
			}
			for (j = 0; j < i; j++) // i ���� �κ��� ����� ������.
				bnode_data.rec[j] = HEAD->rec[j];
			bnode_data.rec[j] = in_rec;
			j++;
			while (i < MAXD_data)
			{
				bnode_data.rec[j] = HEAD->rec[i];
				j++;		//i�� j�� ���� ��Ų��. 
				i++;
			}
			// index of center record is at D_data. center �� �� ������ ��� HEAD �� �ű��.
			for (i = 0; i <= D_data; i++)
				HEAD->rec[i] = bnode_data.rec[i];
			HEAD->fill_cnt = D_data + 1;

			
			new_ptrd = (type_ptr_datanode)malloc(sizeof(type_data_node));
			for (j = 0; i <= MAXD_data; i++, j++) 
				new_ptrd->rec[j] = bnode_data.rec[i];
			new_ptrd->fill_cnt = D_data;
		
			strcpy(ROOT->key[0], HEAD->rec[D_data].name); // �� ����Ÿ����� �� �� ���ڵ��� Ű�� �θ� ����
			ROOT->ptrd[1] = new_ptrd;	// �� �۾����� ���ݺ��� �������� B+  Ʈ���� ��.

			HEAD->link = new_ptrd;	// ���Ḯ��Ʈ�� �� ��带 ������.
			new_ptrd->link = NULL;	// �� ����Ÿ ��尡 ���Ḯ��Ʈ�� ������ �����.
			return 1; // �������� ����.
		}
	} // �ʱ��Ȳ ���� ����.

	// (ROOT�� NULL �̰ų� �ʱ��Ȳ��)�� �ƴϸ� ����� �´�. �� B-tree �ε����� ���� B-tree ��.
	// ���� Ž���� ���Ͽ� in_rec �� �� ����Ÿ��带 ã�´�. 

	curr = ROOT;	//root�� ���� curr�� �ִ´�. 

	// �ε��� ���� ������̼�.
	top = -1;	// ������ empty �� �ʱ�ȭ�Ѵ�.
	if (ROOT->ptri[0] != NULL) {	// ROOT �� ������尡 �ƴ�. 
		do {
			for (i = 0; i < curr->fill_cnt; i++) {   // �ڽ����� ������ �����͸� ã��.
				if (strcmp(in_key, curr->key[i]) <= 0)
					break; // �ڽ����� ���� ���� ����
			}
			push(curr); // ���߿� �θ� ã�� ���� ������.
			curr = curr->ptri[i];	// �ڽ����� ���� ��.
			if (curr->ptri[0] == NULL)
				break;	// ���� ��  �� curr ��尡 ���������.
		} while (1);
	}	// �ε������� ������̼�.

	// curr �� �ε��������� ���������. ���⼭�� ����Ÿ���� �������� ��.
	for (i = 0; i < curr->fill_cnt; i++)
		if (strcmp(in_key, curr->key[i]) <= 0)
			break;

	parent = curr; // ������� ����. (����: ���ÿ� ������ ����.)
	curr_d = curr->ptrd[i]; // ����Ÿ���� ���� ��. curr_d �� ����Ÿ��� ����Ŵ.
	down_idx = i;	// �θ𿡼� �ڽ����� ������ �ε��� ����. ���߿� �̿��� ����.

	// �� ����Ÿ���(curr_d)�� in_rec ���ڵ带 �־�� ��.

	fc = curr_d->fill_cnt;
	if (fc < MAXD_data) 	// ���� full �� �ƴ�. ������ ���� ���� ��ġ�� ����.
	{
		for (i = 0; i < fc; i++) {
			if (strcmp(in_key, curr_d->rec[i].name) < 0)
				break;  // i �� ������.
			else if (strcmp(in_key, curr_d->rec[i].name) == 0)
			{
				printf("����Ű �̹� ����� ���� ����!\n");
				return 0;
			}
			else; // try next i.
		}
		// i ���� ������ ��ĭ�� �������� ����Ʈ��.
		for(j=fc;j>i;j--)
			curr_d->rec[j] = curr_d->rec[j - 1];

		curr_d->rec[i] = in_rec; // �� ���ڵ带 ����.
		curr_d->fill_cnt++;

		if (down_idx < parent->fill_cnt)	// ������ �����ͷ� ���� �� ���� �θ� �ݿ����� ����.
			strcpy(parent->key[down_idx], curr_d->rec[fc].name);// ����Ÿ����� ������ ���ڵ��� Ű�� �θ𿡰� �־���. ���� �߿�!!
		return 1; // �������� ����.
	}
	else
	{ 	// ù ����Ÿ��尡 full ��. split �� �ʿ���. big data node �� �غ���
		for (i = 0; i < MAXD_data; i++) {
			if (strcmp(in_key, curr_d->rec[i].name) < 0) break;
			else if (strcmp(in_key, curr_d->rec[i].name) == 0)
			{
				printf("����Ű �̹� ����� ���� ����!\n");
				return 0;
			}
			else; // try next i.
		}
		for (j = 0; j < i; j++) // i ���� �κ��� ����� ������.
			bnode_data.rec[j] = curr_d->rec[j];
		bnode_data.rec[j] = in_rec;
		j++;
		while (i < MAXD_data) {
			bnode_data.rec[j] = curr_d->rec[i];
			j++;		//i�� j�� ���� ��Ų��. 
			i++;
		}
		// center record is at index D_data. center �� �� ������ ��� curr_d �� �ű��.
		for (i = 0; i <= D_data; i++) curr_d->rec[i] = bnode_data.rec[i];
		curr_d->fill_cnt = D_data + 1;

		new_ptrd = (type_ptr_datanode)malloc(sizeof(type_data_node));
		for (i = 0; i < D_data; i++)	// ������ �ڴ� ���ο� ��忡 ����.
			new_ptrd->rec[i] = bnode_data.rec[i + 1 +D_data];
		new_ptrd->fill_cnt = D_data;
		new_ptrd->link = NULL;

		// �������� ���Ḯ��Ʈ�� �� ���ο� ��带 curr ���� ��尡 �ǰ� ���� �־�� �Ѵ�.
		new_ptrd->link = curr_d->link;
		curr_d->link = new_ptrd;

		// �� [centerŰ, new_ptrd] �� �ε��������� ��������� �θ� ��忡 �ִ� ���ο� ������ �����.
		// �Ʒ� 3 �ٷ� ���� �����Ŀ� �Ʒ��� b-tree ���� ������ ����.
		curr = parent;
		strcpy(in_key, curr_d->rec[D_data].name);	// ���� ���� 1.
		child_d = new_ptrd;					// ���� ���� 2.
	} // ����Ÿ��尡 full �� ���� ����.

	// ���⼭ ���ʹ� B-tree �� �ϳ��� ���� �ִ� �۾��� �Ѵ�.

	do {
		// ���⿡ ���� ���� ���� in_key �� ���� Ű,  ���� child_d �Ǵ� child �� �����͸� �����ϰ� �´�.
		// The pair <in_key, child_d> �Ǵ� <in_key, child>  �� curr �� �־�� �Ѵ�. 
		// child_d: ����Ÿ��带 ����Ʈ,  child: �ε�����带 ����Ʈ. �� �� ������ �̿������� curr�� ����/���� ���η� ����.

		if (curr->fill_cnt < MAXD_idx)	// curr �� ���ڸ��� �ִ�. �� curr ��忡 �ִ´�.
		{
			for (i = 0; i < curr->fill_cnt; i++) //i�� curr->fill_cnt���� �����ϸ鼭 �����Ѵ�.
				if (strcmp(in_key, curr->key[i]) < 0) //i ������ Ű�� in_key ���� �� ũ�� �����. 
					break;

			for (j = curr->fill_cnt; j > i; j--) { // i ���� ���� �κ��� �����Ϳ� Ű��  ��ĭ�� ��� ����Ʈ �Ѵ�.
				curr->ptrd[j + 1] = curr->ptrd[j];
				curr->ptri[j + 1] = curr->ptri[j];
				strcpy(curr->key[j], curr->key[j - 1]);
			}

			strcpy(curr->key[i], in_key);	// Ű ���� 
			if (curr->ptri[0] == NULL) {	// curr �� ��������̹Ƿ� type-1 �����̴�.
				curr->ptrd[i + 1] = child_d;	// �ڽ� �����͸� ptrd �� ���� 
				curr->ptri[i + 1] = NULL;
			}
			else { // curr �� ������尡 �ƴϴ�. ��� type-2 �����̴�. �ڽ� �����͸� ptri �� �ִ´�.
				curr->ptrd[i + 1] = NULL;
				curr->ptri[i + 1] = child;
			}
			curr->fill_cnt++;         // fill ī��Ʈ ������Ŵ 

			return 1; // ���� �������� ����.
		}
		else 	//   curr �� ���ڸ��� ����. �ε�������� ���� �̿��Ͽ� split & elevation �� �����Ѵ�.
		{
			for (i = 0; i < MAXD_idx; i++) {
				if (strcmp(in_key, curr->key[i]) < 0) // ���� i �� Ű�� in_key�� ũ�� ����
					break;
			}

			bnode_index.ptri[0] = curr->ptri[0]; // ���� ������ 0 �� �ű�.
			bnode_index.ptrd[0] = curr->ptrd[0]; //   "   "

			for (j = 0; j < i; j++) {	// i ������ curr �� ������ �ű�.
				strcpy(bnode_index.key[j], curr->key[j]);
				bnode_index.ptri[j + 1] = curr->ptri[j + 1];
				bnode_index.ptrd[j + 1] = curr->ptrd[j + 1];
			}

			strcpy(bnode_index.key[j], in_key); // ���� ù ���� in_key �� �־� �ش�.
			// ���� ����2 ��  �����͸��־� �ش�.
			if (curr->ptri[0] == NULL) { // curr �� ��������̹Ƿ� type-1 �����̴�.
				bnode_index.ptrd[j + 1] = child_d;   // �ڽ� �����͸� ptrd �� ���� 
				bnode_index.ptri[j + 1] = NULL;  // �̰��� ���� �ʾƵ� ������ ����.
			}
			else { // curr �� ������尡 �ƴϴ�. ��� type-2 �����̴�. �ڽ� �����͸� ptri �� �ִ´�.
				bnode_index.ptri[j + 1] = child;
				bnode_index.ptrd[j + 1] = NULL;  // �̰��� ���� �ʾƵ� ������ �� �Ѵ�.
			}

			j++; // j ������Ŵ 

			while (i < MAXD_idx) {
				strcpy(bnode_index.key[j], curr->key[i]);  //curr�� ���ڵ带 bnode �� ����
				bnode_index.ptri[j + 1] = curr->ptri[i + 1]; // �� ������ �����͵� ����.
				bnode_index.ptrd[j + 1] = curr->ptrd[i + 1];
				j++;		//i�� j�� ���� ��Ų��. 
				i++;
			}

			// ���带 ������ ������ ���ݺθ� curr �� �ִ´�.
			for (i = 0; i < D_idx; i++) {
				curr->ptri[i] = bnode_index.ptri[i];    //������ ���� ���� ���� ��忡 �־��ش�. [���ڵ�, ������]
				curr->ptrd[i] = bnode_index.ptrd[i];
				strcpy(curr->key[i], bnode_index.key[i]);
			}

			curr->ptri[i] = bnode_index.ptri[i];		//bnode.ptr[i]�� curr.ptr[i]�� ����Ų��. 
			curr->ptrd[i] = bnode_index.ptrd[i];
			curr->fill_cnt = D_idx;

			new_ptri = (type_ptr_idxnode)malloc(sizeof(type_idx_node)); // ������ �Ĺݺθ� ���ο� ��忡 �ִ´�. 

			for (i = 0; i < D_idx; i++) {	//���� ���� ����忡 ���� 
				new_ptri->ptri[i] = bnode_index.ptri[i + 1 + D_idx];
				new_ptri->ptrd[i] = bnode_index.ptrd[i + 1 + D_idx];
				strcpy(new_ptri->key[i], bnode_index.key[i + 1 + D_idx]);
			}
			new_ptri->ptri[i] = bnode_index.ptri[i + 1 + D_idx];	// �� ������ �����͸� �ű��.
			new_ptri->ptrd[i] = bnode_index.ptrd[i + 1 + D_idx];
			new_ptri->fill_cnt = D_idx;

			strcpy(in_key, bnode_index.key[D_idx]);	// ������ �߰� Ű�� elevation �� Ű�� ��
			child = new_ptri;					// ���� �Ҵ��� ����� �����͸� elevation �� �����ͷ� ��.

			if (top >= 0) {	// curr�� �θ� ����.  
				curr = pop();	//  curr �� �θ� ���ÿ��� ���� �ͼ� �װ����� curr �� ����.
				// ���ο� curr �� �� [in_key, child] �� �ִ� �۾��� ������ ������.
			}
			else {    // curr �� ROOT �̴�. �θ� ����. �θ� ����� �װ��� �θ� �Ǿ�� �Ѵ�.
				tptr = (type_ptr_idxnode)malloc(sizeof(type_idx_node));
				strcpy(tptr->key[0], in_key);
				tptr->ptri[0] = curr;
				tptr->ptri[1] = new_ptri;
				tptr->ptrd[0] = NULL;
				tptr->ptrd[1] = NULL;
				tptr->fill_cnt = 1;
				ROOT = tptr;	// ��Ʈ�� ����.
				return 1; // ���� �������� ����.
			} // else.
		} // else.
	} while (1);

} //�Լ� insert_arec_b_plus_tree

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
	
}   //  �Լ� insert_btree

void push(type_ptr_idxnode node) {
	if (top > MAX) ///>>>  MAX-1 �̻�( >= ) �̾�� ���� �ʳ�?
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

	temp = top; ///>>> ���� top �� ����Ű�� ���� ���� �ֱ��� ����Ÿ�� ����Ǿ� �ִ�. ����,
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