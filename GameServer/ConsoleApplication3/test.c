#define LINEMAX 100

int linestate200[LINEMAX], linestate[LINEMAX]; //��·��0��ʼ��ţ�״̬��1���е�����0�޵�����

	enum UporOn { ehandup, ehandon }; //Ϊժ�һ��������ehandup��ʾժ����ehandon��ʾ�һ���

	struct UpOnnode          //ժ�һ����нڵ�ṹ
	{
		enum UporOn phonestate;      //ժ�һ��������
		int linenum;            //��·�ţ���0��ʼ����
		struct UpOnnode* next;  //ָ����һ�ڵ��ָ�룻
	};

	void scanfor200(int linestate200[LINEMAX], int linestate[LINEMAX], struct UpOnnode* head1, struct UpOnnode* end1) {
		int i;
		enum UporOn status;
		int changed;
		for (i = 0; i < LINEMAX; i++) {
			changed = 0;
			if (linestate200[i] == 0 && linestate[i] == 1) {
				status = ehandup;
				changed = 1;
			}
			else if (linestate200[i] == 1 && linestate[i] == 0) {
				status = ehandon;
				changed = 1;
			}
			if (changed == 1) {
				struct UpOnnode* ptr = (struct UpOnnode*)malloc(sizeof(struct UpOnnode));
				ptr->phonestate = status;
				ptr->linenum = i;
				ptr->next = 0;
				end1->next = ptr;
				end1 = ptr;
			}
			linestate200[i] = linestate[i];
		}
	}

