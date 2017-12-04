#include <iostream>
#include <vector>
#include <string>
#include "head.h"

using namespace std;

int main1() {
	srand(time(0));
	vector<Pokemon*> pokemon;
	int i = 0;
	while (true) {
		cout << "请输入小精灵信息：\n类型（1力量型，2肉盾型，3防御型，4敏捷型）:";
		int type;
		cin >> type;
		Pokemon* temp;
		switch (type) {
		case STRENGTH:
			temp = new strengthPokemon();
			break;
		case AGILE:
			temp = new agilePokemon();
			break;
		case MEET:
			temp = new meetPokemon();
			break;
		case DEFENSE:
			temp = new defensePokemon();
			break;
		default:
			break;
		}
		cout << "请输入名字：";
		string name;
		cin >> name;
		temp->name = name;
		cout << "请输入攻击力(10 20)：";
		int adl;
		int adh;
		cin >> adl;
		cin >> adh;
		FloatNum ad(adl, adh);
		temp->attackDamage = ad;
		cout << "请输入法强(10 20)：";
		int apl;
		int aph;
		cin >> apl;
		cin >> aph;
		FloatNum ap(apl, aph);
		temp->power = ap;
		cout << "请输入护甲：";
		int armor;
		cin >> armor;
		temp->armor = armor;
		cout << "请输入魔抗：";
		int mResist;
		cin >> mResist;
		temp->mResist = mResist;
		cout << "请输入暴击率（0-1）：";
		float critical;
		cin >> critical;
		temp->critical = critical;
		cout << "请输入敏捷：";
		int agile;
		cin >> agile;
		temp->agile = agile;
		cout << "请输入HP上限：";
		int maxHP;
		cin >> maxHP;
		temp->maxHP = maxHP;
		cout << "请输入MP上限：";
		int maxMP;
		cin >> maxMP;
		temp->maxMP = maxMP;
		cout << "请输入回避率（0-1）：";
		float avoidance;
		cin >> avoidance;
		temp->avoidance = avoidance;
		cout << "请输入当前经验：";
		int curexp;
		cin >> curexp;
		temp->currentExp = curexp;
	rep1:
		cout << "请输入目标经验：";
		int tarexp;
		cin >> tarexp;
		if (tarexp < curexp) {
			cout << "当前经验不能多于目标经验\n";
			goto rep1;
		}
		temp->targetExp = tarexp;
		temp->level = 1;
		temp->loality = 1;
		cout << "编号：" << i << "\n" << "名字：\t\t\t\t" << temp->name << "\n等级：\t\t\t\t" << temp->level << "\n物理攻击力：\t" << temp->attackDamage <<
			"\n魔法攻击力：\t" << temp->power << "\n护甲：\t\t\t\t" << temp->armor << "\n魔法抗性：\t\t" << temp->mResist <<
			"\nHtemp上限：\t\t\t" << temp->maxHP << "\nMtemp上限：\t\t\t" << temp->maxMP << "\n暴击率：\t\t\t" << temp->critical * 100 <<
			"%\n回避率：\t\t\t" << temp->avoidance * 100 << "%\n敏捷：\t\t\t\t" << temp->agile << "\n剩余技能点：\t" << temp->skillPoints << endl;
		pokemon.push_back(temp);
		cout << "若不再输入请输入#否则输入任意字符：";
		char test;
		cin >> test;
		flushall();
		if (test == '#')
			break;
	}
	while (true) {
		cout << "输入(1 ? ?)跳过几级（未连接数据库，不能读取等级经验信息，故只能升级）\n输入（2 ? ?）开始战斗（在此只能1v1）";
		int choice;
		cin >> choice;
		switch (choice) {
		case 1:
		{
				  int i;
				  int level;
				  cin >> i;
				  cin >> level;
				  for (; level > 0; level--)
					  pokemon[i]->levelUp();
				  break;
		}
		case 2:
		{
				  
				  int from;
				  int to;
				  cin >> from;
				  cin >> to;
				  pokemon[from]->isDead = false;
				  pokemon[from]->hp = pokemon[from]->maxHP;
				  pokemon[from]->mp = pokemon[from]->maxMP;
				  pokemon[from]->isBlue = true;
				  pokemon[to]->isDead = false;
				  pokemon[to]->hp = pokemon[to]->maxHP;
				  pokemon[to]->mp = pokemon[to]->maxMP;
				  pokemon[to]->isBlue = false;
				  vector<Pokemon*> red;
				  red.push_back(pokemon[from]);
				  vector<Pokemon*> blue;
				  blue.push_back(pokemon[to]);
				  if (from < pokemon.size() && to < pokemon.size() && from != to) {
					  while (!(pokemon[from]->isDead || pokemon[to]->isDead)) {
						  vector<Attack> att;
						  att.push_back(pokemon[from]->attack(pokemon[to]).setseq(from, to));
						  att.push_back(pokemon[to]->attack(pokemon[from]).setseq(to, from));
						  Attack::execute(att, blue, red);
						  cout << pokemon[from]->name << "剩余" << pokemon[from]->hp << "点HP" << endl;
						  cout << pokemon[to]->name << "剩余" << pokemon[to]->hp << "点HP" << endl;
					  }
					  cout << (pokemon[from]->isDead ? pokemon[from]->name : pokemon[to]->name) << "死亡" << endl;
				  }

				  else
					  cout << "输入不合理\n";
				  break;
		}
		default:
			break;
		}
	}
	return 0;
}