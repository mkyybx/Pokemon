#define LINEMAX 100

int linestate200[LINEMAX], linestate[LINEMAX]; //线路从0开始编号；状态：1：有电流，0无电流；

	enum UporOn { ehandup, ehandon }; //为摘挂机区别符：ehandup表示摘机，ehandon表示挂机；

	struct UpOnnode          //摘挂机队列节点结构
	{
		enum UporOn phonestate;      //摘挂机区别符；
		int linenum;            //线路号（从0开始）；
		struct UpOnnode* next;  //指向下一节点的指针；
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

