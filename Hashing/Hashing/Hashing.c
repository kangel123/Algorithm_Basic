#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define   Tbl_size  37533	// 총 회사명 수의 1.1 배 정도.

typedef struct ty_record {
	char name[300];
	int monincome;  // 단위: 천원.
	int link;
} type_record;

type_record Hashtable[Tbl_size];
int LAST = Tbl_size - 1; // 가장 높은 빈자리로 초기화.
float average_num_probe;
FILE* fp;

int hash(char recname[]) {
	unsigned char u; int HA, j, leng, halfleng;
	long sum = 0;
	int A[300];
	// name 의 글자 j 에 대하여 다음을 수행한다:
	leng = strlen(recname);
	for (j = 0; j < leng; j++) {
		u = recname[j];
		A[j] = u;
	}
	halfleng = leng / 2;
	for (j = 0; j < halfleng; j++)
		sum = sum + (A[j] * A[leng - 1 - j] * A[(leng - 1) / 2]);
	if (leng % 2 == 1)
		sum = sum + A[halfleng] * A[halfleng + 1] * A[(leng - 1) / 2];
	HA = sum % Tbl_size;  // HA is a home address given by the hash function.
	return HA;
} // end of hash

// 빈자리 찾기. 전역변수 LAST 를 이용한다.
// LAST 이하에 빈자리가 있을 수 있다. 즉 LAST 보다 큰 위치에는 빈자리가 없다.
// 다음 함수는 LAST 이하에서 빈자리가 있는 가장 높은 주소를 찾아 준다. (LAST 도 변경).
int find_empty_location() {
	int curr;
	curr = LAST;
	while (Hashtable[curr].name[0] != '\0')  // curr 가 빈자리가 아니면 내린다.
		curr -= 1;
	return curr;
} // end of find_empty_location.

// return value: 넣는 동안의 레코드들을 조사해 본 수. (-1 이면 입력 실패).
int insert_rec(type_record rec) {
	int HA, curr, nprove = 0, empty_loc;
	HA = hash(rec.name);
	if (Hashtable[HA].name[0] == '\0') { // 홈주소가 비어 있음. 여기에 넣고 종료.
		Hashtable[HA] = rec; // HA 위치의 link 는 이미 -1 임.
		return 1; // probe 수는 1.
	}
	else { // 홈 주소에 이미 레코드가 들어 있음.
		curr = HA;
		// 링크의 마지막까지 따라 간다.
		do {
			if (strcmp(Hashtable[curr].name, rec.name) == 0)  // 동일한 것이 이미 들어 있음.
			{
				printf("insertion failed since same key already exists.\n");
				return -1;
			}
			nprove += 1;
			if (Hashtable[curr].link == -1)
				break;
			else
				curr = Hashtable[curr].link;
		} while (1);

		// curr 는 마지막 원소를 가리킨다. 빈자리를 가져 온다.
		empty_loc = find_empty_location();
		// 여기에 레코드를 넣고, chain을 연결한다.
		
		Hashtable[empty_loc] = rec;	
		Hashtable[curr].link = empty_loc;
		return nprove;
	} //else.
} // end of insert_rec

// 반환값: -1 이면 찾기 실패, 아니면 찾은 위치.
// probe: 찾는데 사용한 레코드 프로브 수.
int retrieve_rec(char* key, int* probe) {
	int curr;
	curr = hash(key);
	if (Hashtable[curr].name[0] == '\0') // home address is empty.
		return -1;

	*probe = 0;
	do {
		*probe += 1;
		if (strcmp(Hashtable[curr].name, key) == 0) {
			return curr;
		}
		else
			curr = Hashtable[curr].link;
	} while (curr != -1);
	// 여기에 오면 찾기를 실패한 것임.
	return -1;
} // retrieve_rec 

float compute_average_number_of_probes_per_search() {
	char line[300], * res;
	int nprobe, loc, leng, n_search_done = 0, total_probes = 0;
	float average_probe;

	fp = fopen("Companies_Korean.txt", "r");
	while (1)
	{
		res = fgets(line, 300, fp);
		if (!res)
			break;
		leng = strlen(line);
		line[leng - 1] = '\0';  // 마지막 newline 글자를 지운다.
		loc = retrieve_rec(line, &nprobe);
		if (loc != -1) {
			n_search_done += 1;
			total_probes += nprobe;
		}
	}
	average_probe = (double)total_probes / (double)n_search_done;
	fclose(fp);
	return average_probe;
} // end of compute_average_number_of_probes_for_search


// return value: number of relocations of records.
// chain_split: 체인분리 회수를 표시할 변수에 대한 주소.
int del_start(int s, int* chain_split)
{
	int curr, prev, nmove, found = -1, prev_found;

	// s 다음 위치부터 체인 내에서 홈주소가 s 인 맨 나중의 레코드를 찾는다.
	curr = Hashtable[s].link;	//curr=2
	prev = s;	//s=1
	while (curr != -1)
	{
		if (hash(Hashtable[curr].name) == s) {
			prev_found = prev; // 찾은 곳의 선행 주소도 저장.
			found = curr; // 찾은 장소 저장.
		}
		// 다음 주소로 전진한다.
		prev = curr;
		curr = Hashtable[curr].link;

	} //while.

	if (found == -1) { // 홈주소가 s 인 레코드가 발견되지 않음.
		// 위치 s (즉 체인의 첫 원소)를 체인에서 제거한다. 단지 s 에 더미레코드를 저장하면 됨.
		Hashtable[s].name[0] = '\0';
		return 0; // no moves of records have happened.
	}
	else {  // found 에서 발견됨. found 의 레코드를 s 로 이동함.
		strcpy(Hashtable[s].name, Hashtable[found].name);
		Hashtable[s].monincome = Hashtable[found].monincome;
		// found 의 레코드를 지우는 새로운 문제가 생긴다. 아래 1 은 위의 한번 이동을 반영함.
		nmove = 1 + del_middle(found, prev_found, &chain_split);
		return nmove;
	}
} // end of del_starat.

// return value: number of relocations of records.
// chain_split: 체인분리 여부를 표시할 변수에 대한 주소.
int del_middle(int s, int p, int* chain_split)
{
	int curr, prev, HA_curr, HA_curr_belongs_to_D_s, element_of_D_s, nmove, found, prev_found;

	// s 다음 위치부터 체인 내에서 홈주소가 s 인 맨 나중의 레코드를 찾는다.
	curr = Hashtable[s].link;	//curr=3
	prev = s;	//s=2
	found = -1;
	while (curr != -1)
	{
		if (hash(Hashtable[curr].name) == s) {
			prev_found = prev; // 찾은 곳의 선행 주소도 저장.
			found = curr; // 찾은 장소 저장.
		}
		// 다음 주소로 전진한다.
		prev = curr;
		curr = Hashtable[curr].link;

	} //while.

	if (found == -1) { // 찾기 실패. 위치 s 를 체인에서 skip 하고, s 의 레코드를 제거한다.
		Hashtable[p].link = Hashtable[s].link;  // skip s.
		Hashtable[s].name[0] = '\0'; // s 에 더미 레코드를 넣어 제거.
		return 0; // no relocations of records occurred.
	}
	else { // s 를 home address 로 하는 원소가 s 이후의 체인에 존재한다.
		// D(s): 위치 s 를 포함하여 이하 체인의 모든 주소들의 집합. 
		//       이것은 사실은 위치 s 부터 이하의 체인의 원소의 link 필드의 내용들이다(-1은 제외).
		//       s 의 후행자부터 시작하여 HA 가 D(s) 에 속하지 않는 마지막 레코드를 찾자.
		found = -1; // found will have the address that meets the condition.
		curr = Hashtable[s].link; // curr scans elements of chain after location s.
		prev = s; // this is predecessor of curr.
		while (curr != -1) {
			HA_curr = hash(Hashtable[curr].name); // home address of curr.
			HA_curr_belongs_to_D_s = 0; // 초기화. 0 는 curr 의 홈주소가 D(s)에 속하지 않음을 말함.
			element_of_D_s = s; // element_of_D_s 로 D(s) 의 원소를 스캔할 것임.
			do { // scan D(s)
				if (element_of_D_s == HA_curr) {
					HA_curr_belongs_to_D_s = 1;
					break;
				}
				else
					element_of_D_s = Hashtable[element_of_D_s].link;	
			} while (element_of_D_s != -1);  // -1 을 curr로 변경해도 좋으며 시간이 덜 걸린다.
			// 
			if (HA_curr_belongs_to_D_s == 0) {  // D(s) 에 속하지 않는 HA 를 가진 원소 발견!
				found = curr;
				prev_found = prev;
			}

			prev = curr;
			curr = Hashtable[curr].link;
		}
		// found 에 찾은 결과가 있다.
		if (found != -1) { // D(s)에 HA가 속하지 않는 원소가 존재함. 
			strcpy(Hashtable[s].name, Hashtable[found].name);
			Hashtable[s].monincome = Hashtable[found].monincome;
			nmove = 1 + del_middle(found, prev_found, &chain_split);
			return nmove;
		}
		else { // s 다음 부터의 모든 원소는 home address 가 D(s)에 속함.
			// s 의 선행자에서 체인을 분리해도 된다.
			Hashtable[p].link = -1; // 체인 분리.
			*chain_split += 1; // 체인 분리 횟수 증가.
			nmove = del_start(s ,&chain_split);  // s에 있는 레코드 삭제. s 는 체인의 시작.
			return nmove;
		}
	}
} // end of del_middle.

// return value: number of record relocations
// parameter chain_split: used for relecting chain splits
int delete_rec(char* dkey, int* chain_split) {
	int found = -1, h, curr, prev, nmove = 0;

	h = hash(dkey);
	if (Hashtable[h].name[0] == '\0')
	{
		printf("There are no records with such key.\n");
		return -1;
	}

	if (strcmp(dkey, Hashtable[h].name) == 0)  // dkey is at its home address.
		nmove = del_start(h, &chain_split);
	else {
		// search the record whose key is equal to dkey from the next of address h.
		curr = Hashtable[h].link;
		prev = h;
		found = -1;
		while (curr != -1) {
			if (strcmp(Hashtable[curr].name, dkey) == 0) {
				found = curr;
				break;
			}
			else {
				prev = curr;
				curr = Hashtable[curr].link;
			}
		} //while.

		if (found == -1)
		{
			printf("A record with such key does not exist.\n");
			return -1;
		}
		else
			nmove = del_middle(curr, prev, &chain_split);
	}//else.

	return nmove;
} // end of delete_rec

void delete_multiple(int num_del_req) {
	char line[300], * res;
	int cnt_lines = 0, num_split = 0, nmove, leng, num_deletion_success = 0, num_relocated_deletions = 0;

	fp = fopen("Companies_Korean.txt", "r");
	while (1)
	{
		res = fgets(line, 300, fp);
		if (!res)
			break;
		cnt_lines += 1;
		leng = strlen(line);
		line[leng - 1] = '\0';  // 마지막 newline 글자를 지운다.
		nmove = delete_rec(res ,&num_split);
		if (nmove >= 0)
			num_deletion_success += 1;
		if (nmove > 0)
			num_relocated_deletions += nmove;
		if (cnt_lines == num_del_req)
			break;
	}
	printf("num_deletion_success=%d, num_relocated_deletions=%d\n", num_deletion_success, num_relocated_deletions);
	fclose(fp);
	return;
}

int check_num_links(char key[]) {
	int count = 0, curr, flag = -1;
	int HA = hash(key);
	if (Hashtable[HA].name[0] == '\0') {
		printf("There are no records with such key.\n");
		return -1;
	}

	curr = HA;

	// move the curr, and fine the key. if the key doesn't exist in hashtable but has hash links,
	// we have to use flag.
	while (curr != -1) {
		if (strcmp(Hashtable[curr].name, key) == 0) {
			flag = 1;
		}
		count += 1;
		curr = Hashtable[curr].link;
	}
	if (flag != 1) {
		printf("There are no records with such key.\n");
		return -1;
	}
	return count;
}

int main(void)
{
	type_record a_rec;
	int cnt_lines = 0, cnt_insertion = 0, pos, n_probe, i, k, leng, m_income, n_chain_split, n_move, num_deletions_to_do;
	int count;
	char line[300], name[1300], * res;

	// 테이블 초기화. 모든 위치를 빈자리로 함.
	for (i = 0; i < Tbl_size; i++) {
		Hashtable[i].name[0] = '\0';
	}

	// 파일에서 모든 이름을 읽어서 해시테이블에 넣는다.
	fp = fopen("Companies_Korean.txt", "r");
	while (1)
	{
		res = fgets(line, 300, fp);
		if (!res)
			break;
		cnt_lines += 1;
		leng = strlen(line);
		line[leng - 1] = '\0';  // 마지막 newline 글자를 지운다.
		m_income = rand() % 50000; // 단위는 천원
		strcpy(a_rec.name, line);
		a_rec.monincome = m_income;
		a_rec.link = -1;
		n_probe = insert_rec(a_rec);
		if (n_probe != -1)
			cnt_insertion += 1; // 삽입성공 횟수 증가
	}
	fclose(fp);
	printf("num of line=%d,  num of inserted records=%d\n\n", cnt_lines, cnt_insertion);

	// 명령문 루프
	while (1) {
		printf("명령(i,r,v,d,q,c,e)> ");
		res = gets(line);
		if (!res)
		{
			printf("wrong command.\n");
			continue;
		}
		if (strlen(line) < 1)
			continue;

		if (line[0] == 'e')  // exit command.
			break;

		if (line[0] == 'v')  // a command to verify average number of probes of a search.
		{
			average_num_probe = compute_average_number_of_probes_per_search();
			printf("average number of probes per search = %f\n", average_num_probe);
			continue;
		}

		// read a name.
		k = 2; i = 0;
		while (line[k] != '\0' && line[k] == ' ') k++; // 공백문자는 건너간다.
		while (line[k] != '\0') { // read one key which is a company name(including a blank).
			name[i] = line[k];
			k++;  i++;
		}
		name[i] = '\0';

		switch (line[0]) {
		case 'i': // insert command:  i 학진상회
			strcpy(a_rec.name, name); a_rec.monincome = rand() % 50000; a_rec.link = -1;
			n_probe = insert_rec(a_rec);
			if (n_probe == -1)
				printf("insertion failed by duplicate key.\n");
			else
				printf("insertion succeeded. num probe = %d\n", n_probe);
			break;
		case 'r': // search command: r 학진상회
			n_probe = 0;
			pos = retrieve_rec(name, &n_probe);
			if (pos == -1)
				printf("No such record exists.\n");
			else
				printf("Search succeeded. loc_found=%d, n_probes=%d\n", pos, n_probe);
			break;
		case 'd':  // deletion command:  d 학진상회
			n_chain_split = 0;
			n_move = delete_rec(name, &n_chain_split);
			if (n_move >= 0)
				printf("Deletion succeeded. num. relocations=%d, num chain splits=%d\n", n_move, n_chain_split);
			break;
		case 'q':  // multiple deletion command:  q  20000   (deletion of the first 20,000 names in the data file).
			for (k = 0; k < i; k++) { // check if name is a number string.
				if (!isdigit(name[k])) {
					printf("number should be given after q.\n");  break;
				}
			}
			num_deletions_to_do = atoi(name);
			delete_multiple(num_deletions_to_do);
			break;

		case 'c':  // check the number of links related to the input
			count = check_num_links(name);
			if (count != -1) {
				printf("number of links : %d\n", count);
			}
			break;

		default:
			printf("wrong command\n");
		}// switch.
	} // while(1)
	return 0;
} // end of main.

