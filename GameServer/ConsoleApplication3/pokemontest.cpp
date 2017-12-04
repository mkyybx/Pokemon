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
		cout << "������С������Ϣ��\n���ͣ�1�����ͣ�2����ͣ�3�����ͣ�4�����ͣ�:";
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
		cout << "���������֣�";
		string name;
		cin >> name;
		temp->name = name;
		cout << "�����빥����(10 20)��";
		int adl;
		int adh;
		cin >> adl;
		cin >> adh;
		FloatNum ad(adl, adh);
		temp->attackDamage = ad;
		cout << "�����뷨ǿ(10 20)��";
		int apl;
		int aph;
		cin >> apl;
		cin >> aph;
		FloatNum ap(apl, aph);
		temp->power = ap;
		cout << "�����뻤�ף�";
		int armor;
		cin >> armor;
		temp->armor = armor;
		cout << "������ħ����";
		int mResist;
		cin >> mResist;
		temp->mResist = mResist;
		cout << "�����뱩���ʣ�0-1����";
		float critical;
		cin >> critical;
		temp->critical = critical;
		cout << "���������ݣ�";
		int agile;
		cin >> agile;
		temp->agile = agile;
		cout << "������HP���ޣ�";
		int maxHP;
		cin >> maxHP;
		temp->maxHP = maxHP;
		cout << "������MP���ޣ�";
		int maxMP;
		cin >> maxMP;
		temp->maxMP = maxMP;
		cout << "������ر��ʣ�0-1����";
		float avoidance;
		cin >> avoidance;
		temp->avoidance = avoidance;
		cout << "�����뵱ǰ���飺";
		int curexp;
		cin >> curexp;
		temp->currentExp = curexp;
	rep1:
		cout << "������Ŀ�꾭�飺";
		int tarexp;
		cin >> tarexp;
		if (tarexp < curexp) {
			cout << "��ǰ���鲻�ܶ���Ŀ�꾭��\n";
			goto rep1;
		}
		temp->targetExp = tarexp;
		temp->level = 1;
		temp->loality = 1;
		cout << "��ţ�" << i << "\n" << "���֣�\t\t\t\t" << temp->name << "\n�ȼ���\t\t\t\t" << temp->level << "\n����������\t" << temp->attackDamage <<
			"\nħ����������\t" << temp->power << "\n���ף�\t\t\t\t" << temp->armor << "\nħ�����ԣ�\t\t" << temp->mResist <<
			"\nHtemp���ޣ�\t\t\t" << temp->maxHP << "\nMtemp���ޣ�\t\t\t" << temp->maxMP << "\n�����ʣ�\t\t\t" << temp->critical * 100 <<
			"%\n�ر��ʣ�\t\t\t" << temp->avoidance * 100 << "%\n���ݣ�\t\t\t\t" << temp->agile << "\nʣ�༼�ܵ㣺\t" << temp->skillPoints << endl;
		pokemon.push_back(temp);
		cout << "����������������#�������������ַ���";
		char test;
		cin >> test;
		flushall();
		if (test == '#')
			break;
	}
	while (true) {
		cout << "����(1 ? ?)����������δ�������ݿ⣬���ܶ�ȡ�ȼ�������Ϣ����ֻ��������\n���루2 ? ?����ʼս�����ڴ�ֻ��1v1��";
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
						  cout << pokemon[from]->name << "ʣ��" << pokemon[from]->hp << "��HP" << endl;
						  cout << pokemon[to]->name << "ʣ��" << pokemon[to]->hp << "��HP" << endl;
					  }
					  cout << (pokemon[from]->isDead ? pokemon[from]->name : pokemon[to]->name) << "����" << endl;
				  }

				  else
					  cout << "���벻����\n";
				  break;
		}
		default:
			break;
		}
	}
	return 0;
}