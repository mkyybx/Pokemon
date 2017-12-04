#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <string>
#include <iostream>
#define MAX_LEVEL 15
#define STRENGTH 1
#define MEET 2
#define DEFENSE 3
#define AGILE 4

#define AD 1
#define AP 2
#define ARMOR 3
#define MRESIST 4
#define CRITICAL 5
#define AG 6
#define HP 7
#define MP 8
#define AVOID 9

using namespace std;
typedef short buff;//增益buf正数，减益是负数，0代表没有

class FloatNum {
private:
	int from;
	int to;
	void check(int* from, int* to) {
		if (*to < *from) {
			*from = *from ^ *to;
			*to = *from ^ *to;
			*from = *from ^ *to;
		}
	}

public:
	void setValue(int from, int to) {
		check(&from, &to);
		this->from = from;
		this->to = to;
	}
	FloatNum(int from, int to) {
		check(&from, &to);
		this->from = from;
		this->to = to;
	}
	FloatNum(void) {
		;
	}
	void addValue(int from, int to) {
		this->from += from;
		this->to += to;
		check(&(this->from), &(this->to));
	}
	int returnValue() {
		return rand() % (to - from + 1) + to;
	}
	friend ostream &operator<<(ostream &os, const FloatNum &c) {
		os << c.from << '-' << c.to;
		return os;
	}

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & from;
		ar & to;
	}
};


class Skill {
public:
	virtual ~Skill();
protected:
	Skill(int cd, bool isSingle, bool isEnemy, buff buf) :cd(cd), isSingle(isSingle), isEnemy(isEnemy), buf(buf){}
	int cd;
	bool isSingle;
	bool isEnemy;
	buff buf;
};

class AttackSkill :public Skill {
public:
	AttackSkill(int cd, bool isSingle, bool isEnemy, buff buf, bool isPhysical, float gain) : Skill(cd, isSingle, isEnemy, buf), isPhysical(isPhysical), gain(gain){}
	virtual ~AttackSkill();
	bool isPhysical;
	float gain;
};

class CureSkill :public Skill {
public:
	int value;
	float percent;
	virtual ~CureSkill();
	CureSkill(int cd, bool isSingle, bool isEnemy, buff buf, int value, float percent) : Skill(cd, isSingle, isEnemy, buf), value(value), percent(percent){}
};

class BuffSkill :public Skill {
public:
	BuffSkill(int cd, bool isSingle, bool isEnemy, buff buf) : Skill(cd, isSingle, isEnemy, buf){}
};

//设置继承
class fightingPokemon {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		char c = isDead ? 1 : 0;
		char d = isBlue ? 1 : 0;
		ar & hp;
		ar & mp;
		ar & maxHP;
		ar & maxMP;
		ar & c;
		ar & d;
		ar & buffChain;
		ar & inittype;
		ar & name;
		isDead = (c == 1 ? true : false);
		isBlue = (d == 1 ? true : false);
	}
public:
	int hp;
	int maxHP;
	int mp;
	int maxMP;
	bool isDead;
	bool isBlue;
	int inittype;
	vector<buff> buffChain;
	string name;

	fightingPokemon() {
		;
	}
};

class Pokemon : public fightingPokemon {
public:
	Pokemon(){
		;
	}
	//战斗属性，可以在战斗中直接减，不写入数据库
	int type;
	//	int inittype;
	//	string name;
	FloatNum attackDamage;
	FloatNum power;
	int armor;
	int mResist;
	float critical;
	int agile;
	//	int maxHP;
	//	int maxMP;
	float avoidance;
	vector<int> skill;//索引值
	//自身属性
	float rarity;
	int level;
	int currentExp;
	int targetExp;
	float loality;
	int skillPoints;
	//内部属性(不存入数据库)
	//	bool isBlue;//是否在蓝方
	//	int hp;
	//	int mp;
	//	bool isDead;
	//	vector<buff> buffChain;

	bool addPoint(char type) {
		if (skillPoints > 0) {
			if (type == AD)
				attackDamage.addValue(1, 2);
			else if (type == AP)
				power.addValue(1, 1);
			else if (type == ARMOR)
				armor += 1;
			else if (type == MRESIST)
				mResist += 1;
			else if (type == CRITICAL) {
				if (critical < 0.98)
					critical += 0.02;
				else return false;
			}
			else if (type == AG)
				agile += 5;
			else if (type == HP)
				maxHP += 5;
			else if (type == MP)
				maxMP += 3;
			else if (type == AVOID) {
				if (avoidance < 0.98)
					avoidance += 0.02;
				else return false;
			}
			return true;
		}
		else return false;
	}

	fightingPokemon getCompact() {
		return (fightingPokemon)*this;
	}



	friend class Attack;

	//virtual void attack() = 0;
	virtual Attack attack(Pokemon* to) = 0;

	virtual bool damage(int value) {
		if (isDead)
			return false;
		else {
			if (hp > value)
				hp -= value;
			else {
				hp = 0;
				dead();
			}
			return true;
		}
	}

	virtual void revive() {
		//复活？
	}

	virtual bool cure(int value) {
		if (isDead)
			return false;
		else if (hp + value >= maxHP) {
			hp = maxHP;
			return true;
		}
		else {
			hp += value;
			return true;
		}
	}

	virtual ~Pokemon() {

	}


	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & type;
		ar & inittype;
		ar & name;
		ar & attackDamage;
		ar & power;
		ar & armor;
		ar & mResist;
		ar & critical;
		ar & agile;
		ar & maxHP;
		ar & maxMP;
		ar & avoidance;
		ar & skill;
		//自身属性
		ar & rarity;
		ar & level;
		ar & currentExp;
		ar & targetExp;
		ar & loality;
		ar & skillPoints;
	}

	//protected: 
	virtual void levelUp() {
		currentExp = currentExp % targetExp;
		//只通过增加经验调用
		if (level < MAX_LEVEL) {
			level++;
			//加点，包括技能
			skillPoints += 3;
			//继承成长
			//数据库同步
		}
		std::cout << "小精灵 " << name << " 升级啦，当前等级 " << level <<
			" 当前经验 " << currentExp << " 目标经验 " << targetExp << endl;
	}

	bool gainExp(int num) {
		currentExp += num;
		if (currentExp > targetExp) {
			currentExp -= targetExp;
			levelUp();
			return true;
		}
		return false;
	}

	virtual void dead() {
		isDead = true;
		//做点别的？
	}


};

class Attack {
private:
	Pokemon* from;//from = null应该被清除之前，代表没攻击
	int fromseq;
	Pokemon* to;//null代表全体
	int toseq;
	bool isBlue;//是否在蓝方
	int damage;//不计算回避，负数代表治疗
	bool isPhysical;
	buff buf;//0代表没有
	int agile;
	bool isCritical;
	bool isAvoided = false;
	int skillid;
	Attack();

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		char a = isBlue ? 1 : 0;
		char b = isPhysical ? 1 : 0;
		char c = isCritical ? 1 : 0;
		char d = isAvoided ? 1 : 0;
		ar & fromseq;
		ar & toseq;
		ar & a;
		ar & damage;
		ar & b;
		ar & buf;
		ar & c;
		ar & d;
		ar & skillid;
		isBlue = (a == 1 ? true : false);
		isPhysical = (b == 1 ? true : false);
		isCritical = (c == 1 ? true : false);
		isAvoided = (d == 1 ? true : false);
	}

public:
	Attack(Pokemon* from, Pokemon* to, int damage, bool isPhysical, buff buf, int agile, bool isCritical, int skillid) :from(from), to(to), damage(damage), isPhysical(isPhysical), buf(buf), agile(agile), isCritical(isCritical), skillid(skillid){}
	virtual ~Attack() {
		;
	}
	Attack setseq(int fromseq, int toseq) {
		this->fromseq = fromseq;
		this->toseq = toseq;
		return *this;
	}
	static bool compare(const Attack& a, const Attack&b) {
		return a.from->agile < b.from->agile;//大到小
		//？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
	}
	static void execute(vector<Attack>& a, vector<Pokemon*>& IpbludSide, vector<Pokemon*>& IpredSide) {
		if (!(a.empty())) {
			//排序，按敏捷
			sort(a.begin(), a.end(), compare);
			//qsort(a., size, sizeof(int), &compare);
			for (int i = 0; i < a.size(); i++) { //挨个判断
				if (!(a[i].from->isDead || a[i].to->isDead)) {
					if (a[i].to == NULL) {//全体攻击！！！！！！！！！！！！！！！！！！注意damage取值，需要改
						vector<Pokemon*>& temp = a[i].isBlue ? IpbludSide : IpredSide;
						for (int j = 0; j < temp.size(); j++) {
							a[i].to = temp[j];
							if (a[i].damage > 0) {
								int damage;
								if (a[i].isPhysical)
									damage = a[i].damage * 100 / (100.0 + a[i].to->armor);
								else
									damage = a[i].damage * 100 / (100.0 + a[i].to->mResist);
								if (rand() % 1000 > a[i].to->avoidance * 1000)//未闪避
									a[i].to->damage(damage);//注意返回值，死了就跳过
								else a[i].isAvoided = true;
							}
							else if (a[i].damage < 0)
								a[i].to->cure(-a[i].damage);
							if (!(a[i].to->isDead) && a[i].buf != 0)
								a[i].to->buffChain.push_back(a[i].buf);
						}
					}
					else {
						if (a[i].damage > 0) {
							if (a[i].isPhysical) {
								cout << "来自" << a[i].from->name << "的" << a[i].damage << "点物理输出";
								a[i].damage = a[i].damage * 100 / (100.0 + a[i].to->armor);
								cout << "被" << a[i].to->name << "的" << a[i].to->armor << "点护甲削减后，造成了" <<
									a[i].damage << "点伤害";
							}
							else {
								cout << "来自" << a[i].from->name << "的" << a[i].damage << "点魔法输出";
								a[i].damage = a[i].damage * 100 / (100.0 + a[i].to->mResist);
								cout << "被" << a[i].to->name << "的" << a[i].to->armor << "点魔抗削减后，造成了" <<
									a[i].damage << "点伤害";
							}
							if (rand() % 1000 > a[i].to->avoidance * 1000) {//未闪避
								a[i].to->damage(a[i].damage);//注意返回值，死了就跳过
								cout << endl;
							}
							else {
								a[i].isAvoided = true;
								cout << "，且被躲避" << endl;
							}
						}
						else if (a[i].damage < 0)
							a[i].to->cure(-a[i].damage);
						if (!(a[i].to->isDead) && a[i].buf != 0)
							a[i].to->buffChain.push_back(a[i].buf);
					}
				}
				else a.erase(a.begin() + i);
			}
		}
	}
};

class strengthPokemon :public Pokemon {
protected:
	void levelUp() {
		Pokemon::attackDamage.addValue(level * 3, level * 4);
		power.addValue(3, 3);
		armor += level * 0.7;
		mResist += level / 3;
		critical *= 1.1;
		agile += 20;
		maxHP *= 1.1;
		maxMP += 20;
		avoidance = avoidance;
		Pokemon::levelUp();
	}
	Attack attack(Pokemon* to) {
		cout << "力量型小精灵" << name << "对" << to->name <<"发起了攻击(攻击特效：普通攻击)";
		if (rand() % 1000 < loality * 1000) {
			if (rand() % 1000 < critical * 1000) {
				int damage = attackDamage.returnValue() * 2;
				Attack temp((Pokemon*)this, to, damage, true, 0, agile, true, -1);
				cout << "，造成了暴击，输出" << damage << "点物理伤害" << endl;
				return temp;
			}
			else {
				int damage = attackDamage.returnValue();
				Attack temp((Pokemon*)this, to, damage, true, 0, agile, false, -1);
				cout << "，输出" << damage << "点物理伤害" << endl;
				return temp;
			}
		}
		else {
			Attack temp(NULL, NULL, 0, false, 0, 0, false, -1);
			return temp;
		}
	}
};

class meetPokemon :public Pokemon {
protected:
	void levelUp() {
		Pokemon::attackDamage.addValue(level * 0.5, level * 0.7);
		power.addValue(4, 6);
		armor += level;
		mResist += level;
		critical = critical;
		agile += 5;
		maxHP += level * 10;
		maxMP += 20;
		avoidance = avoidance * 1.05;
		Pokemon::levelUp();
	}
	Attack attack(Pokemon* to) {
		cout << "肉盾型小精灵" << name << "对" << to->name << "发起了攻击(攻击特效：物魔混伤)";
		if (rand() % 1000 < loality * 1000) {
			int damage;
			bool isCritical;
			if (rand() % 1000 < critical * 1000) {
				damage = attackDamage.returnValue() * 2;
				isCritical = true;
				cout << "，造成了暴击";
			}
			else {
				damage = attackDamage.returnValue();
				isCritical = false;
			}
			damage = damage * 0.5 + power.returnValue() * 0.5;
			cout << "，输出" << damage << "点魔法伤害" << endl;
			Attack temp((Pokemon*)this, to, damage, true, 0, agile, isCritical, -1);
			return temp;
		}
		else {
			Attack temp(NULL, NULL, 0, false, 0, 0, false, -1);
			return temp;
		}
	}
};

class defensePokemon :public Pokemon {
protected:
	void levelUp() {
		Pokemon::attackDamage.addValue(1, 10);
		power.addValue(1, 20);
		armor += level * 0.8;
		mResist += level * 0.8;
		critical = critical;
		agile += 5;
		maxHP *= 2;
		maxMP += 30;
		avoidance = avoidance;
		Pokemon::levelUp();
	}
	Attack attack(Pokemon* to) {
		cout << "防御型小精灵" << name << "对" << to->name << "发起了攻击(攻击特效：HP上限加成)";
		if (rand() % 1000 < loality * 1000) {
			int damage;
			bool isCritical;
			if (rand() % 1000 < critical * 1000) {
				damage = attackDamage.returnValue() * 2;
				isCritical = true;
				cout << "，造成了暴击";
			}
			else {
				damage = attackDamage.returnValue();
				isCritical = false;
			}
			damage += hp / 100;
			cout << "，输出" << damage << "点魔法伤害" << endl;
			Attack temp((Pokemon*)this, to, damage, true, 0, agile, isCritical, -1);
			return temp;
		}
		else {
			Attack temp(NULL, NULL, 0, false, 0, 0, false, -1);
			return temp;
		}
	}
};

class agilePokemon :public Pokemon {
	//protected:
public:
	void levelUp() {
		Pokemon::attackDamage.addValue(level, level);
		power.addValue(level * 2, level * 3);
		armor += 5;
		mResist += 4;
		critical *= 1.3;
		agile += level;
		maxHP += level;
		maxMP += level;
		avoidance *= 1.1;
		Pokemon::levelUp();
	}
	Attack attack(Pokemon* to) {
		cout << "敏捷型小精灵" << name << "对" << to->name << "发起了攻击(攻击特效：魔法攻击)";
		if (rand() % 1000 < loality * 1000) {
			int damage;
			bool isCritical;
			if (rand() % 1000 < critical * 1000) {
				damage = power.returnValue() * 2;
				isCritical = true;
				cout << "，造成了暴击";
			}
			else {
				damage = power.returnValue();
				isCritical = false;
				cout << "，输出" << damage << "点物理伤害" << endl;
			}
			Attack temp((Pokemon*)this, to, damage, false, 0, agile, isCritical, -1);
			return temp;
		}
		else {
			Attack temp(NULL, NULL, 0, false, 0, 0, false, -1);
			return temp;
		}
	}
};
