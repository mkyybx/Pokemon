#define WIN32_LEAN_AND_MEAN

//检查所有recv是否都做了异常处理
//检查所有send发送之前是否有标签
#include <windows.h>
#ifndef SOCKET_H
#define SOCKET_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "sha1.c"
#include "head.h"

#pragma comment(lib, "Ws2_32.lib")
using namespace std;


extern SOCKET __cdecl initSocket(void);
extern DWORD WINAPI fakeClient(LPVOID IpParam);
sql::Driver *dirver;
DWORD mCenter;

//数据长度应小于1024
int myrecv(SOCKET s, char *buf, int len, int flags) {
	int length = recv(s, buf, sizeof(int), flags);
	if (length <= 0) {
		return length;
	}
	else {
		int num = *(int*)buf;
		int state = recv(s, buf, num, flags);
		return state;
	}
}

int mysend(SOCKET s, const char *buf, int len, int flags) {
	char* length = (char*)(&len);
	send(s, length, sizeof(int), flags);
	int state = send(s, buf, len, flags);
	return state;
}


class room : public listroom{
public:
	void broadcast() {
		ostringstream os;
		boost::archive::text_oarchive oa(os);
		oa << *this;
		char c = ROOM_CHANGED;
		for (vector<player>::iterator it = red.begin(); it != red.end(); it++) {
			mysend(*(it->id.socket), &c, 1, 0);
			mysend(*(it->id.socket), os.str().c_str(), os.str().size(), 0);
		}
		for (vector<player>::iterator it = blue.begin(); it != blue.end(); it++) {
			mysend(*(it->id.socket), &c, 1, 0);
			mysend(*(it->id.socket), os.str().c_str(), os.str().size(), 0);
		}
	}
public:
	vector<player> red;
	vector<Pokemon*> redp;
	vector<player> blue;
	vector<Pokemon*> bluep;
	player p[10];//编号
	int plength;
	int redlength;
	int bluelength;
	map<SOCKET*, int> mp;//鉴别数据发送者
	

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		vector<string> reduser;
		for (vector<player>::iterator it = red.begin(); it != red.end(); it++)
			reduser.push_back(it->id.linfo->user);
		vector<string> blueuser;
		for (vector<player>::iterator it = blue.begin(); it != blue.end(); it++)
			blueuser.push_back(it->id.linfo->user);
		ar & reduser;
		ar & blueuser;
		string owner(this->owner.linfo == NULL ? "" : this->owner.linfo->user);
		ar & owner;
	}

	room(identity1 owner, int id, bool isDuel) {
		this->owner = owner;
		this->id = id;
		this->isDuel = isDuel;
	}

	bool enter(player id) {
		if (!isStarted) {
			if (red.size() < 5)
				red.push_back(id);
			else if (blue.size() < 5)
				blue.push_back(id);
			else return false;
			
			return true;
		}
		else return false;
	}

	void leave(identity1 id, bool isStarted) {
		for (vector<player>::iterator it = red.begin(); it != red.end(); it++) {
			if (it->id.socket == id.socket) {
				if (!isStarted) {
					if (owner.socket == id.socket)
						owner.linfo = NULL;
					red.erase(it);
					broadcast();
				}
				else {
					it->p->isDead = true;
				}
				return;
			}
		}

		for (vector<player>::iterator it = blue.begin(); it != blue.end(); it++) {
			if (it->id.socket == id.socket) {
				if (!isStarted) {
					if (owner.socket == id.socket)
						owner.linfo = NULL;
					blue.erase(it);
					broadcast();
				}
				else {
					it->p->isDead = true;
				}
				return;
			}
		}

	}

	void setNewOwner() {
		for (vector<player>::iterator it = red.begin(); it != red.end(); it++) {
			owner = it->id;
			return;
		}

		for (vector<player>::iterator it = blue.begin(); it != blue.end(); it++) {
			owner = it->id;
			return;
		}
	}

	bool hasPlayer() {
		return red.size() + blue.size() == 0 ? false : true;
	}

	bool change(identity1 id) {
		for (vector<player>::iterator it = red.begin(); it != red.end(); it++) {
			if (it->id.socket == id.socket) {
				if (blue.size() < 5) {
					blue.push_back(*it);
					red.erase(it);
					broadcast();
					return true;
				}
				return false;
			}
		}
		for (vector<player>::iterator it = blue.begin(); it != blue.end(); it++) {
			if (it->id.socket == id.socket) {
				if (red.size() < 5) {
					red.push_back(*it);
					blue.erase(it);
					broadcast();
					return true;
				}
				return false;
			}
		}
	}

	void changePokemon(SOCKET* old, Pokemon* fresh) {
		for (vector<player>::iterator it = red.begin(); it != red.end(); it++) {
			if (it->id.socket == old) {
				it->p = fresh;
				return;
			}
		}

		for (vector<player>::iterator it = blue.begin(); it != blue.end(); it++) {
			if (it->id.socket == old) {
				it->p = fresh;
				return;
			}
		}
	}

	void start() {
		if (isDuel && red.size() != blue.size()) {
			//决斗赛人数必须相等
			char c = TEAM_NOT_EQUAL;
			mysend(*(owner.socket), &c, 1, 0);
		}
		else {
			//和初始化
			int i = 0;
			for (vector<player>::iterator it = red.begin(); it != red.end(); it++) {
				mp.insert(pair<SOCKET*, int>(it->id.socket, i));
				redp.push_back(it->p);
				p[i++] = *it;
				it->p->isBlue = false;
				it->p->hp = it->p->maxHP;
				it->p->mp = it->p->maxMP;
				it->p->isDead = false;
			}
			redlength = i;
			for (vector<player>::iterator it = blue.begin(); it != blue.end(); it++) {
				mp.insert(pair<SOCKET*, int>(it->id.socket, i));
				bluep.push_back(it->p);
				p[i++] = *it;
				it->p->isBlue = true;
				it->p->hp = it->p->maxHP;
				it->p->mp = it->p->maxMP;
				it->p->isDead = false;
			}
			plength = i;
			bluelength = plength - redlength;
			//同步序号
			//头|个数|红队人数|(类型)数据|自身编号
			for (int j = 0; j < i; j++) {
				char temp1 = BATTLE_START;
				mysend(*(p[j].id.socket), &temp1, 1, 0);
				char temp2 = i;//i个人
				mysend(*(p[j].id.socket), &temp2, 1, 0);
				char temp00 = redlength;//红队人数
				mysend(*(p[j].id.socket), &temp00, 1, 0);
				for (int k = 0; k < i; k++) {
					ostringstream os;
					boost::archive::text_oarchive oa(os);
					oa << p[k].p->getCompact();
					char temp0 = p[k].p->type;
					mysend(*(p[j].id.socket), &temp0, 1, 0);
					mysend(*(p[j].id.socket), os.str().c_str(), os.str().length(), 0);
				}
				char temp3 = j;
				mysend(*(p[j].id.socket), &temp3, 1, 0);
			}
			isStarted = true;
		}
	}
};

void getRandomPokemon(LoginInfo login, sql::Connection *con, int num, bool sendBack, map<int,Pokemon&>* pokemon, int specified, int level) {
	int* id = (int*)malloc(sizeof(int)* num);
	ostringstream os1;
	for (int i = 0; i < num; i++) {
		sql::PreparedStatement* pstmt = con->prepareStatement("select * from initpokemon");
		sql::ResultSet* result = pstmt->executeQuery();
		result->last();
		int initnum = result->getRow();
		int choice;
		if (specified == -1)
			choice = rand() % initnum - 1;
		else choice = specified;
		result->first();
		for (int i = 0; i < choice; i++)
			result->next();
		int type = result->getInt("type");
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

		string temps = result->getString("name").c_str();
		temp->name = temps;
		temp->inittype = result->getInt("idinitpokemon");
		FloatNum ad(result->getInt("attackDamagel"), result->getInt("attackDamageh"));
		FloatNum ap(result->getInt("powerl"), result->getInt("powerh"));
		temp->type = type;
		temp->attackDamage = ad;
		temp->power = ap;
		temp->armor = result->getInt("armor");
		temp->mResist = result->getInt("mResist");
		temp->critical = result->getDouble("critical");
		temp->agile = result->getInt("agile");
		temp->maxHP = result->getInt("maxHP");
		temp->maxMP = result->getInt("maxMP");
		temp->avoidance = result->getDouble("avoidance");
		temp->level = 1;
		temp->currentExp = 0;
		delete pstmt;
		delete result;
		pstmt = con->prepareStatement("select * from levelexp where level = 1");
		result = pstmt->executeQuery();
		result->next();
		temp->targetExp = result->getInt("exp");
		temp->loality = 1;
		temp->skillPoints = 0;
		if (specified != -1) {
			for (int k = 0; k < level; k++)
				temp->levelUp();
		}
		ostringstream os;
		boost::archive::text_oarchive oa(os);
		oa << *temp;
		string p = os.str();
		p.append("\0");
		const char* tempc = p.c_str();
		delete pstmt;
		delete result;
		pstmt = con->prepareStatement("INSERT INTO pokemon (data) VALUES (?)");// select max(idpokemon) from pokemon;");
		//pstmt->setString(1, "123");
		pstmt->setString(1, tempc);
		pstmt->execute();
		
		delete pstmt;
		//pstmt = con->prepareStatement("select max(idpokemon) from pokemon;");
		pstmt = con->prepareStatement("SELECT @@IDENTITY");
		//pstmt->setString(1, tempc);
		result = pstmt->executeQuery();
		result->next();
		//id[i] = result->getInt("max(idpokemon)");
		id[i] = result->getInt("@@IDENTITY");
		if (sendBack) {
			pair<int, Pokemon&> temppair(id[i], *temp);
			pokemon->insert(temppair);
		}
		delete pstmt;
		delete result;
		if (!sendBack)
			delete temp;
		os1 << id[i] << ',';
	}
	string p = os1.str();
	p.append("\0");
	sql::PreparedStatement* pstmt = con->prepareStatement("update user set pokemon=concat(pokemon,?) where iduser = ?;");
	const char* tempc = p.c_str();
	pstmt->setString(1, tempc);
	pstmt->setString(2, login.user);
	pstmt->executeUpdate();
	delete pstmt;
}

map<int,Pokemon&> getPokemonMap(sql::Connection* con, LoginInfo login, identity1 id) {
	map<int, Pokemon&> returnResult;
	sql::PreparedStatement* pstmt;
	sql::ResultSet* result;
	pstmt = con->prepareStatement("select pokemon from user where iduser=?;");
	pstmt->setString(1, login.user);
	result = pstmt->executeQuery();
	result->next();
	string s = result->getString("pokemon").c_str();
	delete result;
	delete pstmt;
	vector<string> vs;
	boost::split(vs, s, boost::is_any_of(","), boost::token_compress_on);
	for (vector<string>::iterator it = vs.begin(); it != vs.end(); it++) {
		//没有0和最后
		if (*it != "" && *it != "0") {
			strengthPokemon* po = new strengthPokemon();
			istringstream is(*it);
			int poid;
			is >> poid;
			pstmt = con->prepareStatement("select data from pokemon where idpokemon=?;");
			pstmt->setInt(1, poid);
			result = pstmt->executeQuery();
			result->next();
			s = result->getString("data").c_str();
			istringstream iis(s);
			boost::archive::text_iarchive ia(iis);
			ia >> *po;
			Pokemon* finalpo;
			switch (po->type) {
			case AGILE:
			{
						  agilePokemon* apo = new agilePokemon();
						  istringstream iis(s);
						  boost::archive::text_iarchive ia(iis);
						  ia >> *apo;
						  finalpo = apo;
			}
				break;
			case MEET:
			{
						 meetPokemon* apo = new meetPokemon();
						 istringstream iis(s);
						 boost::archive::text_iarchive ia(iis);
						 ia >> *apo;
						 finalpo = apo;
			}
				break;
			case DEFENSE:
			{
							defensePokemon* apo = new defensePokemon();
							istringstream iis(s);
							boost::archive::text_iarchive ia(iis);
							ia >> *apo;
							finalpo = apo;
			}
				break;
			case STRENGTH:
			{
							 strengthPokemon* apo = new strengthPokemon();
							 istringstream iis(s);
							 boost::archive::text_iarchive ia(iis);
							 ia >> *apo;
							 finalpo = apo;
			}
				break;
			}
			delete po;
			pair<int, Pokemon&> pairp(poid, *finalpo);
			returnResult.insert(pairp);
			delete pstmt;
			delete result;
		}
	}
	return returnResult;
}

//停止？
DWORD WINAPI timer(LPVOID IpParam) {
	//ipparam是线程DOWRD
	Sleep(30000);
	PostThreadMessage((DWORD)IpParam, TIME_OUT+BASE, 0, GetCurrentThreadId());
	return 0;
}

DWORD WINAPI roomManager(LPVOID IpRoom) {
	//wparam参数，lparam socket*，外面不要free
	//room();enter()
	room* r = (room*)IpRoom;
	MSG m;
	vector<Attack> att;
	int round = 0;
	unsigned short isDead = 0;//用高位
	unsigned short attacked = 0xFFFF;
	unsigned short played = 0;
	sql::Connection* con = dirver->connect("localhost", "mky0", "123456");
	con->setSchema("pokemon");
	con->setClientOption("characterSetResults", "utf8");
	char state = 0;
	bool waitforexchange = false;
	int changedPokemon[10] = {0};
	bool isBlueWin;
	bool isTimerValid = false;
	while (true) {
		GetMessage(&m, 0, 0, 0);
		m.message -= BASE;
		switch (m.message) {
		case LIST_ROOM:
		{
						  ostringstream os;
						  boost::archive::text_oarchive oa(os);
						  oa << *((listroom*)r);
						  static const WCHAR a = '1';
						  static HANDLE mutex = CreateMutex(0, false, &a);
						  WaitForSingleObject(mutex, -1);
						  mysend(*((SOCKET*)(m.lParam)), os.str().c_str(), os.str().size(), 0);
						  ReleaseMutex(mutex);

						  break;
		}
		case ENTER_ROOM:
		{
						   //失败返回leaveroom，成功继续返回room序列化
						   bool entered = r->enter(*(player*)(m.wParam));
						   char c = (entered ? ENTER_ROOM : LEAVE_ROOM);
						   mysend(*(((player*)(m.wParam))->id.socket), &c, 1, 0);
						   if (entered) {
							   PostThreadMessage(((player*)(m.wParam))->tid, GetCurrentThreadId(), 0, 0);
							   r->broadcast();
						   }
						   else PostThreadMessage(((player*)(m.wParam))->tid, 0, 0, 0);
						   free((player*)m.wParam);
						   break;
		}
		case LEAVE_ROOM:
		{
						   //wparam为identity*
						   if (!r->isStarted) {
							   r->leave(*(identity1*)(m.wParam), false);
							   char cc = LEAVE_ROOM_ACK;
							   PostThreadMessage(m.lParam, LEAVE_ROOM_ACK + BASE, 0, 0);
							   mysend(*((identity1*)(m.wParam))->socket, &cc, 1, 0);
							   if (r->hasPlayer()) {
								   if (((identity1*)(m.wParam))->socket == r->owner.socket) {
									   r->setNewOwner();
									   char c = OWNER_CHANGED;
									   mysend(*(r->owner.socket), &c, 1, 0);
								   }
							   }
							   else {
								   //清除房间
								   PostThreadMessage(mCenter, ROOM_EMPTY+BASE, r->id, GetCurrentThreadId());
								   return 0;
							   }
						   }
						   else {
							   r->leave(*(identity1*)(m.wParam), true);
						   }
						   free((identity1*)(m.wParam));
						   break;
		}
		case CHANGE_TEAM:
		{
							if (r->change(*(identity1*)(m.wParam))) {
								char c = CHANGE_TEAM_ACK;
								mysend(*((identity1*)(m.wParam))->socket, &c, 1, 0);
							}
							else {
								char c = CHANGE_TEAM_NAK;
								mysend(*((identity1*)(m.wParam))->socket, &c, 1, 0);
							}
							free((identity1*)(m.wParam));
							break;
		}
		case CHANGE_POKEMON:
		{
							   if (!(r->isStarted)) {
								   r->changePokemon((SOCKET*)(m.lParam), (Pokemon*)(m.wParam));
								   char c = CHANGE_POKMON_ACK;
								   mysend(*(SOCKET*)(m.lParam), &c, 1, 0);
							   }
							   break;
		}
		case BATTLE_START:
		{
							 if (!(r->isStarted)&&(SOCKET*)(m.lParam) == r->owner.socket) {
								 r->start();
								 if (r->isStarted)
									goto out;
								 else break;
							 }
		}
		case ADD_AI:
		{
					   //wparam为virtual parameter
					   DWORD tid;
					   virtualParameter* tempv = (virtualParameter*)(m.wParam);
					   tempv->room = r->id;
					   CreateThread(0, 0, (LPTHREAD_START_ROUTINE)fakeClient, (LPVOID)(m.wParam), 0, &tid);
					   break;
		}
		default:
			break;
		}
	}
out:
	state = WAIT_FOR_ATTACK;
	DWORD tempd;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)timer, (LPVOID)GetCurrentThreadId(), 0, &tempd);
	isTimerValid = true;
	while (true) {
		GetMessage(&m, 0, 0, 0);
		m.message -= BASE;
		switch (m.message) {
		case ENTER_ROOM:
		{
						   //失败返回leaveroom，成功继续返回room序列化
						   bool entered = r->enter(*(player*)(m.wParam));
						   char c = (entered ? ENTER_ROOM : LEAVE_ROOM);
						   mysend(*(((player*)(m.wParam))->id.socket), &c, 1, 0);
						   if (entered) {
							   PostThreadMessage(((player*)(m.wParam))->tid, GetCurrentThreadId(), 0, 0);
							   r->broadcast();
						   }
						   else PostThreadMessage(((player*)(m.wParam))->tid, 0, 0, 0);
						   free((player*)m.wParam);
						   break;
		}
		case LIST_ROOM:
		{
						  ostringstream os;
						  boost::archive::text_oarchive oa(os);
						  oa << *((listroom*)r);
						  static const WCHAR a = '1';
						  static HANDLE mutex = CreateMutex(0, false, &a);
						  WaitForSingleObject(mutex, -1);
						  mysend(*((SOCKET*)(m.lParam)), os.str().c_str(), os.str().size(), 0);
						  ReleaseMutex(mutex);

						  break;
		}
		case LEAVE_ROOM:
			r->leave(*(identity1*)(m.wParam), true);
			free((identity1*)(m.wParam));
			break;
		case ATTACK:
			if (state == WAIT_FOR_ATTACK){
				//lParam传socket*
				//前16位攻击谁，后16位技能索引
				//为了配合isdead，attacked为0代表攻击过

				//重写isdead和attacked，用低位表示
				int sender = r->mp.find((SOCKET*)m.lParam)->second;
				if ((!(r->p[sender].p->isDead)) && ((attacked << sender) & 0x8000) == 0x8000) {
					unsigned short skill = m.wParam & 0x0000FFFF;
					unsigned short to = m.wParam >> 16;
					if (to < r->plength) {
						if (skill == 0) {
							//无技能
							Pokemon* pfrom = r->p[r->mp.find((SOCKET*)(m.lParam))->second].p;
							Pokemon* pto = r->p[to].p;
							att.push_back(pfrom->attack(pto).setseq(r->mp.find((SOCKET*)(m.lParam))->second,to));
							//没设置pseq！！！！！！！！！
						}
						else {
							//有技能，判断所有权
						}
					}
					attacked &= (~(((unsigned short)0x8000) >> sender));//置0
				}
				if ((isDead >> (16 - r->plength)) == (attacked >> (16 - r->plength))) {
					PostThreadMessage(tempd, STOP_TIMER + BASE, 0, 0);
					isTimerValid = false;
				attack_continue:
					//执行
					Attack::execute(att, r->bluep, r->redp);
					round++;
					//执行buff?
					unsigned short or = 0x8000;
					for (int i = 0; i < r->plength; i++) {
						if (r->p[i].p->isDead)
							isDead |= or;
						else isDead &= (~(((unsigned short)0x8000) >> i));
						or >>= 1;
					}
					//数据：头|p个数据|attack数据
					//向客户端发送数据
					for (int j = 0; j < r->plength; j++) {
						char temp1 = ATTACK_DATA;
						mysend(*(r->p[j].id.socket), &temp1, 1, 0);
						for (int k = 0; k < r->plength; k++) {
							ostringstream os;
							boost::archive::text_oarchive oa(os);
							oa << r->p[k].p->getCompact();
							mysend(*(r->p[j].id.socket), os.str().c_str(), os.str().length(), 0);
						}
						{
							ostringstream os;
							boost::archive::text_oarchive oa(os);
							oa << att;
							mysend(*(r->p[j].id.socket), os.str().c_str(), os.str().length(), 0);
						}
					}
					state = WAIT_FOR_ANIMATION;
					CreateThread(0, 0, (LPTHREAD_START_ROUTINE)timer, (LPVOID)GetCurrentThreadId(), 0, &tempd);
					isTimerValid = true;
					//红高位，蓝低位
					if ((((unsigned short)(~isDead) >> (16 - r->redlength)) == 0) || ((((unsigned short)((~isDead) << r->redlength)) >> (16 - r->bluelength)) == 0)) {
						if (((unsigned short)(~isDead) >> (16 - r->redlength)) == 0)
							isBlueWin = true;
						else isBlueWin = false;

						//一方全灭，游戏结束，向客户端发送数据
						for (int j = 0; j < r->plength; j++) {
							//增加经验
							int exp = 0;
							if (isBlueWin)
								exp = (r->p[j].p->isBlue ? round * 30 : round * 10);
							else
								exp = (r->p[j].p->isBlue ? round * 10 : round * 30);
							if (r->p[j].p->gainExp(exp)) {
								sql::PreparedStatement* pstmt = con->prepareStatement("select exp from levelexp where level = ?;");
								pstmt->setInt(1, r->p[j].p->level);
								sql::ResultSet* rs = pstmt->executeQuery();
								rs->next();
								r->p[j].p->targetExp = rs->getInt("exp");
								delete pstmt;
								delete rs;
							}
							//测试用
							r->p[j].p->loality = 1;
							//客户端同步，数据是小精灵
							char temp1 = BATTLE_FINISH;
							//是否为蓝队赢
							char cisbluewin = isBlueWin ? 1 : 0;
							mysend(*(r->p[j].id.socket), &temp1, 1, 0);
							mysend(*(r->p[j].id.socket), &cisbluewin, 1, 0);
							ostringstream os;
							boost::archive::text_oarchive oa(os);
							oa << *(r->p[j].p);

							//数据库同步
							sql::PreparedStatement* pstmt = con->prepareStatement("update pokemon set data=? where idpokemon = ?;");
							pstmt->setInt(2, r->p[j].pokemonId);
							pstmt->setString(1, os.str().c_str());
							pstmt->executeUpdate();
							//win和total
							delete pstmt;
							pstmt = con->prepareStatement("update user set total=total+1 where iduser = ?");
							pstmt->setString(1, r->p[j].id.linfo->user);
							pstmt->executeUpdate();
							if (!(isBlueWin ^ r->p[j].p->isBlue)) {
								delete pstmt;
								pstmt = con->prepareStatement("update user set win=win+1 where iduser = ?");
								pstmt->setString(1, r->p[j].id.linfo->user);
								pstmt->executeUpdate();
							}

						}
						//交换小精灵
						if (r->isDuel) {
							waitforexchange = true;
							char c = EXCHANGE_POKEMON;
							for (int j = 0; j < r->plength; j++)
								mysend(*(r->p[j].id.socket), &c, 1, 0);
						}
						else {
							char c = BATTLE_FINISH;
							for (int j = 0; j < r->plength; j++)
								mysend(*(r->p[j].id.socket), &c, 1, 0);
							goto cleanup;
						}

					}
					att.clear();//清理以便下次使用
					attacked = 0xFFFF;
				}
			}
			break;
		case ANIMATION_COMPLETE:
			if (state == WAIT_FOR_ANIMATION) {
				//用低位
				int i = r->mp.find((SOCKET*)(m.lParam))->second;
				unsigned short or = 1 << i;
				played |= or;
				if (played == ((unsigned short)0xFFFF) >> (16 - r->plength)) {
					PostThreadMessage(tempd, STOP_TIMER + BASE, 0, 0);
					isTimerValid = false;
				animation_contiune:
					CreateThread(0, 0, (LPTHREAD_START_ROUTINE)timer, (LPVOID)GetCurrentThreadId(), 0, &tempd);
					isTimerValid = true;
					waitforexchange ? state = WAIT_FOR_EXCHANGE : state = WAIT_FOR_ATTACK;
					played = 0;
					//广播同步信息
					for (int i = 0; i < r->plength; i++) {
						char c = ANIMATION_COMPLETE;
						mysend(*(r->p[i].id.socket), &c, 1, 0);
					}
				}
			}
			break;

		case EXCHANGE_POKEMON:
			if (state == WAIT_FOR_EXCHANGE) {
				int pokemonid = m.wParam;
				map<int, Pokemon&>* userpokemon = r->p[r->mp.find((SOCKET*)m.lParam)->second].userpokemon;
				if (userpokemon->find(pokemonid) != userpokemon->end()) {
					//通过验证
					if (changedPokemon[r->mp.find((SOCKET*)m.lParam)->second] == 0) {
						changedPokemon[r->mp.find((SOCKET*)m.lParam)->second] = pokemonid;
						//删除并重构
						Pokemon& exchanged = userpokemon->find(pokemonid)->second;
						userpokemon->erase(userpokemon->find(pokemonid));
						ostringstream os;
						for (map<int, Pokemon&>::iterator it1 = userpokemon->begin(); it1 != userpokemon->end(); it1++) {
							os << it1->first;
							os << ",";
						}
						//写入数据库
						sql::PreparedStatement* pstmt = con->prepareStatement("update user set pokemon=? where iduser = ?;");
						string p = os.str().append("\0");
						pstmt->setString(1, p.c_str());
						pstmt->setString(2, r->p[r->mp.find((SOCKET*)m.lParam)->second].id.linfo->user);
						pstmt->executeUpdate();
						delete pstmt;
						//如果数目不足一个
						if (userpokemon->size() < 1)
							getRandomPokemon(*(r->p[r->mp.find((SOCKET*)m.lParam)->second].id.linfo), con, 1, true, userpokemon, -1, 0);
						//交换
						int getter;
						if (isBlueWin)
							getter = r->mp.find((SOCKET*)m.lParam)->second + r->redlength;
						else getter = r->mp.find((SOCKET*)m.lParam)->second - r->redlength;
						//写入向量
						pair<int, Pokemon&> ppair(pokemonid, exchanged);
						r->p[getter].userpokemon->insert(ppair);
						//写入数据库
						{
							ostringstream os;
							os << pokemonid << ",";
							//写入数据库
							sql::PreparedStatement* pstmt = con->prepareStatement("update user set pokemon=concat(pokemon,?) where iduser = ?;");
							string p = os.str().append("\0");
							pstmt->setString(1, p.c_str());
							pstmt->setString(2, r->p[getter].id.linfo->user);
							pstmt->executeUpdate();
							delete pstmt;
						}
					}
				}

				//判断是否完成，并给予
				bool finished = true;
				if (!isBlueWin) {
					//红胜
					for (int i = r->redlength; i < r->plength; i++) {
						if (changedPokemon[i] == 0) {
							finished = false;
							break;
						}
					}
					if (finished) {
						PostThreadMessage(tempd, STOP_TIMER + BASE, 0, 0);
						isTimerValid = false;
						goto cleanup;
					}
				}
				else {
					for (int i = 0; i < r->redlength; i++) {
						if (changedPokemon[i] == 0) {
							finished = false;
							break;
						}
					}
					if (finished) {
						PostThreadMessage(tempd, STOP_TIMER + BASE, 0, 0);
						isTimerValid = false;
						goto cleanup;
					}
				}
			}
			break;
		case TIME_OUT:
			if (isTimerValid && tempd == m.lParam) {
				if (state == WAIT_FOR_ATTACK)
					goto attack_continue;
				else if (state == WAIT_FOR_ANIMATION)
					goto animation_contiune;
				else if (state == WAIT_FOR_EXCHANGE) {
					if (isBlueWin) {
						for (int i = 0; i < r->redlength; i++) {
							if (changedPokemon[i] == 0) {
								SOCKET* fakesocket = r->p[i].id.socket;
								int fakeid = rand() % (r->p[i].userpokemon->size());
								int j = 0;
								map<int, Pokemon&>::iterator it = r->p[i].userpokemon->begin();
								for (; j < fakeid; it++)
									j++;
								PostThreadMessage(GetCurrentThreadId(), EXCHANGE_POKEMON + BASE, it->first, (LPARAM)fakesocket);
							}
						}
					}
					else {
						for (int i = r->redlength; i < r->plength; i++) {
							if (changedPokemon[i] == 0) {
								SOCKET* fakesocket = r->p[i].id.socket;
								int fakeid = rand() % (r->p[i].userpokemon->size());
								int j = 0;
								map<int, Pokemon&>::iterator it = r->p[i].userpokemon->begin();
								for (; j < fakeid; it++)
									j++;
								PostThreadMessage(GetCurrentThreadId(), EXCHANGE_POKEMON + BASE, it->first, (LPARAM)fakesocket);
							}
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
		//清理工作
	cleanup:
		//发送SYNC
	for (int i = 0; i < r->plength; i++) {
		char c = SYNC_POKEMON;
		PostThreadMessage(r->p[i].tid, BATTLE_FINISH + BASE, 0, 0);
		PostThreadMessage(mCenter, ROOM_EMPTY + BASE, r->id, 0);
		mysend(*(r->p[i].id.socket), &c, 1, 0);
		//个数+3n个包，没有0和最后
		char cc = r->p[i].userpokemon->size();
		mysend(*(r->p[i].id.socket), &cc, 1, 0);
		ostringstream os;
		for (map<int, Pokemon&>::iterator it = r->p[i].userpokemon->begin(); it != r->p[i].userpokemon->end(); it++) {
			char ccc = it->second.type;
			mysend(*(r->p[i].id.socket), &ccc, 1, 0);
			ostringstream os;
			boost::archive::text_oarchive oa(os);
			oa << it->first;
			mysend(*(r->p[i].id.socket), os.str().c_str(), os.str().size(), 0);
			{
				ostringstream os;
				boost::archive::text_oarchive oa(os);
				oa << it->second;
				mysend(*(r->p[i].id.socket), os.str().c_str(), os.str().size(), 0);
			}
		}
	}
	delete r;
	delete con;
	return 0;
}

DWORD WINAPI messageCenter(LPVOID IpParam) {//消息处理
	//wparam参数，lparam线程id外面，不要free
	//重复登录问题，直接关闭socket
	MSG m;
	map<DWORD, identity1> list;
	map<int, DWORD> roomlist;
	while (true) {
		GetMessage(&m, 0, 0, 0);
		m.message -= BASE;
		switch (m.message) {
		case LOGIN_QUERY:
		{
							bool repeat = false;
							pair<DWORD, identity1>* id = (pair<DWORD, identity1>*)(m.wParam);
							string s1(id->second.linfo->user);
							for (auto it = list.begin(); it != list.end(); it++) {
								string s(it->second.linfo->user);
								if (s == s1) {
									PostThreadMessage(m.lParam, INVALID_USER_OR_PASS+BASE, 0, 0);
									repeat = true;
								}
							}
							if (!repeat) {
								list.insert(*id);
								PostThreadMessage(m.lParam, LOGIN_SUCCESS+BASE, 0, 0);
							}
							delete(id);
							break;
		}
		case LOGOUT:
		{
					   map<DWORD, identity1>::iterator it;
					   it = list.find(m.lParam);
					   if (it != list.end())
						list.erase(it);
		}
			break;
		case QUERY_ONLINE:
		{
							 map<DWORD, identity1>::iterator it;
							 vector<string> tempv;
							 for (it = list.begin(); it != list.end(); it++) {
								 tempv.push_back(it->second.linfo->user);
							 }
							 ostringstream os;
							 boost::archive::text_oarchive oa(os);
							 oa << tempv;
							 it = list.find(m.lParam);
							 char c = QUERY_ONLINE;
							 mysend(*(it->second.socket), &c, 1, 0);
							 mysend(*(it->second.socket), os.str().c_str(), os.str().size(), 0);
		}
			break;
		case LIST_ROOM:
		{
						  char flag = LIST_ROOM;
						  mysend(*(list.find(m.lParam)->second.socket), &flag, 1, 0);
						  int roomnum = roomlist.size();
						  mysend(*(list.find(m.lParam)->second.socket), (char*)&roomnum, sizeof(int), 0);
						  for (map<int, DWORD>::iterator it = roomlist.begin(); it != roomlist.end(); it++)
							  PostThreadMessage(it->second, LIST_ROOM+BASE, 0, (LPARAM)(list.find(m.lParam)->second.socket));
		}
			break;
		case CREATE_ROOM:
		{
							//lparam是player*
							int num;
							while (roomlist.find((num = rand()) % 99999) != roomlist.end());
							DWORD dw = *(DWORD*)((char*)(m.wParam) + 1);
							room* r = new room(list.find(dw)->second, num, (bool)(*(char*)(m.wParam)));
							r->enter(*(player*)(m.lParam));
							r->broadcast();
							DWORD tid;
							CreateThread(0, 0, (LPTHREAD_START_ROUTINE)roomManager, r, 0, &tid);
							pair<int, DWORD> temppair(num, tid);
							roomlist.insert(temppair);
							PostThreadMessage(dw, tid, 0, 0);
							free((player*)(m.lParam));
		}
			break;
		case ROOM_EMPTY:
		{
						   map<int, DWORD>::iterator it;
						   it = roomlist.find(m.wParam);
						   if (it != roomlist.end())
							 roomlist.erase(it);
		}
			break;
			//只有enter和leave交给mcenter处理，其余的直接交给roommanager
		case ENTER_ROOM:
		{
						   //lparam为player*
						   //wparam为id房间
						   map<int, DWORD>::iterator it = roomlist.find(m.wParam);
						   if (it != roomlist.end())
							   PostThreadMessage(it->second, ENTER_ROOM+BASE, m.lParam, (LPARAM)((player*)(m.lParam))->id.socket);
						   else {
							   char c = LEAVE_ROOM;
							   mysend(*(((player*)(m.lParam))->id.socket), &c, 1, 0);
							   PostThreadMessage(((player*)(m.lParam))->tid, 0, 0, 0);
						   }
		}
			break;
		default:
			break;
		}
	}
}

DWORD WINAPI subThread(LPVOID IpParam) {//处理玩家非游戏状态的请求
	srand(time(0));
	sql::Connection *con = NULL;
	sql::ResultSet* result = NULL;
	LoginInfo login;
	bool hasLogin = false;
	identity1 id;
	DWORD roomThread = 0;
	map<int, Pokemon&> pokemon;
	MSG outermsg;
	//连接数据库
	try {
		con = dirver->connect("localhost", "mky0", "123456");
	}
	catch (exception ex) {
		printf("SQL服务器未开启！");
		closesocket(*(SOCKET*)IpParam);
		con == NULL ? delete con : 0;

		return 1;
	}
	//选择mydata数据库
	con->setSchema("pokemon");
	con->setClientOption("characterSetResults", "utf8");
	//pstmt = con->prepareStatement("select");
	char recvbuf[1024];
	int recvLength;
	SOCKET* clientSocket = (SOCKET*)IpParam;
	
	char state = -1;//没状态
	sql::PreparedStatement *pstmt;
//	try {
		while (true) {
		here1:
			if (PeekMessage(&outermsg, 0, 0, 0, 0)) {
				if (outermsg.message == LEAVE_ROOM_ACK + BASE) {
					roomThread = 0;
					GetMessage(&outermsg, 0, 0, 0);
				}
				else if (outermsg.message == BATTLE_FINISH + BASE) {
					roomThread = 0;
					GetMessage(&outermsg, 0, 0, 0);
				}
			}
			memset(recvbuf, 0, 1024);
			recvLength = myrecv(*clientSocket, recvbuf, 1024, 0);
//			for (int i = 0; i < recvLength; i++)
//				printf("%c", recvbuf[i]);
			if (recvLength > 0) {
				istringstream is(recvbuf);
				pstmt = NULL;
				if (state == -1 && recvLength == 1) {
					if (recvbuf[0] == QUERY_ONLINE) 
						PostThreadMessage(mCenter, QUERY_ONLINE+BASE, 0, GetCurrentThreadId());
					else if (recvbuf[0] == LIST_ROOM)
						PostThreadMessage(mCenter, LIST_ROOM+BASE, 1, GetCurrentThreadId());
					else if (recvbuf[0] == LEAVE_ROOM) {
						if (roomThread != 0) {
							identity1* id1temp = (identity1*)malloc(sizeof(identity1));
							memcpy(id1temp, &id, sizeof(identity1));
							PostThreadMessage(roomThread, LEAVE_ROOM+BASE, (WPARAM)id1temp, GetCurrentThreadId());
						}
					}
					else if (recvbuf[0] == CHANGE_TEAM) {
						if (roomThread != 0) {
							identity1* id1temp = (identity1*)malloc(sizeof(identity1));
							memcpy(id1temp, &id, sizeof(identity1));
							PostThreadMessage(roomThread, CHANGE_TEAM+BASE, (WPARAM)id1temp, 0);
						}
					}
					else if (recvbuf[0] == ANIMATION_COMPLETE) {
						if (roomThread != 0)
							PostThreadMessage(roomThread, ANIMATION_COMPLETE+BASE, 0, (LPARAM)clientSocket);
					}
					else if (recvbuf[0] == BATTLE_START) {
						if (roomThread != 0)
							PostThreadMessage(roomThread, BATTLE_START + BASE, 0, (LPARAM)clientSocket);
					}
					else state = recvbuf[0];
				}
				else {
					if (state == LOGIN && !hasLogin) {
						try {
							boost::archive::text_iarchive ia(is);
							ia >> login;
							pstmt = con->prepareStatement("select * from user where iduser = ? and password = ?");
							pstmt->setString(1, login.user);
							pstmt->setString(2, login.pass);
							result = pstmt->executeQuery();
							if (!(result->next())) {//查不到
								char temp = INVALID_USER_OR_PASS;
								mysend(*clientSocket, &temp, 1, 0);
							}
							else {//登录成功
								id.linfo = &login;
								id.socket = clientSocket;
								pair<DWORD, identity1>* tempp = new pair<DWORD, identity1>(GetCurrentThreadId(), id);
								PostThreadMessage(mCenter, LOGIN_QUERY+BASE, (WPARAM)tempp, GetCurrentThreadId());
								MSG msg1;
								GetMessage(&msg1, 0, 0, 0);
								char temp = msg1.message -= BASE;
								mysend(*clientSocket, &temp, 1, 0);
								if (temp == LOGIN_SUCCESS) {
									hasLogin = true;

									//同步小精灵vector
									pokemon = getPokemonMap(con, login, id);
									//个数+3n个包，没有0和最后
									char cc = pokemon.size();
									mysend(*clientSocket, &cc, 1, 0);

									for (map<int, Pokemon&>::iterator it = pokemon.begin(); it != pokemon.end(); it++) {
										char ccc = it->second.type;
										mysend(*clientSocket, &ccc, 1, 0);
										ostringstream os;
										boost::archive::text_oarchive oa(os);
										oa << it->first;
										mysend(*clientSocket, os.str().c_str(), os.str().size(), 0);
										{
											ostringstream os;
											boost::archive::text_oarchive oa(os);
											oa << it->second;
											mysend(*clientSocket, os.str().c_str(), os.str().size(), 0);
										}
									}
								}
							}
						}
						catch (exception ex) {
							cout << ex.what();
						}
						delete result;
						delete pstmt;
					}
					else if (state == REGISTER && !hasLogin) {
				//		try {
							boost::archive::text_iarchive ia(is);
							LoginInfo reg;
							ia >> reg;
							pstmt = con->prepareStatement("select * from user where iduser = ?");
							pstmt->setString(1, reg.user);
							result = pstmt->executeQuery();
							if (!(result->next())) {//查不到
								delete pstmt;
								reg.pass[40] = 0;
								pstmt = con->prepareStatement("INSERT INTO user (iduser,password) VALUES (?, ?)");
								pstmt->setString(1, reg.user);
								pstmt->setString(2, reg.pass);
								try {
									pstmt->execute();
								}
								catch (sql::SQLException ex) {
									std::cout << endl;
									std::cout << ex.what() << endl;
								}
								//分配小精灵
								getRandomPokemon(reg, con, 3, false, 0, -1, 0);
								char temp = REGISTER_SUCCESS;
								mysend(*clientSocket, &temp, 1, 0);
							}
							else {//重复
								char temp = REGISTER_FAIL;
								mysend(*clientSocket, &temp, 1, 0);
							}
			//			}
			//			catch (exception ex) {
			//				;
			//			}
							delete pstmt;
							delete result;
					}
					else if (state == QUERY_POKEMON) {
						recvbuf[recvLength] = 0;
						pstmt = con->prepareStatement("select * from user where iduser = ?;");
						pstmt->setString(1, recvbuf);
						result = pstmt->executeQuery();
						if (!result->next()) {
							char temp = NO_SUCH_USER;
							mysend(*clientSocket, &temp, 1, 0);
						}
						else {
							char temp = QUERY_POKEMON;
							mysend(*clientSocket, &temp, 1, 0);
							string s = result->getString("pokemon").c_str();
							vector<string> vs;
							boost::split(vs, s, boost::is_any_of(","), boost::token_compress_on);
							for (vector<string>::iterator it = vs.begin(); it != vs.end(); it++) {
								if (*it != "" && *it != "0") {
									sql::PreparedStatement* pstmt1 = con->prepareStatement("select data from pokemon where idpokemon = ?;");
									pstmt1->setString(1, it->c_str());
									sql::ResultSet* result1 = pstmt1->executeQuery();
									result1->next();
									string s = result1->getString("data").c_str();
									istringstream is(s);
									boost::archive::text_iarchive ia(is);
									Pokemon* p = new strengthPokemon();
									ia >> *p;
									char ccc = p->type;
									mysend(*clientSocket, &ccc, 1, 0);
									mysend(*clientSocket, s.c_str(), s.length(), 0);
									delete pstmt1;
									delete result1;
								}
							}
							char tempc = RETURN_USER_INFO;
							mysend(*clientSocket, &tempc, 1, 0);
							//返回两个int，win和total
							mysend(*clientSocket, result->getString("iduser").c_str(), result->getString("iduser").length(), 0);
							mysend(*clientSocket, result->getString("win").c_str(), sizeof(int), 0);
							mysend(*clientSocket, result->getString("total").c_str(), sizeof(int), 0);
							delete pstmt;
							delete result;
						}
					}
					
					else if (state == CREATE_ROOM && roomThread == 0) {
						//wparam是否是决斗模式
						//lpparam决定player*
						//recvbuf是是否为决斗模式
						player* p = (player*)malloc(sizeof(player));
						p->id = id;
						p->p = &(pokemon.begin()->second);
						p->pokemonId = pokemon.begin()->first;
						p->tid = GetCurrentThreadId();
						p->userpokemon = &pokemon;
						*(DWORD*)(recvbuf + 1) = GetCurrentThreadId();
						PostThreadMessage(mCenter, CREATE_ROOM+BASE, (WPARAM)recvbuf, (LPARAM)p);
						//PostThreadMessage(mCenter, ENTER_ROOM, recvbuf[0], (LPARAM)p);?????????????????????
						MSG msg;
						GetMessage(&msg, 0, 0, 0);
						roomThread = msg.message;//0代表没进去，leave的时候改回
					}
					else if (state == ENTER_ROOM && roomThread == 0) {
						//lparam为player*
						//wparam为房间号
						//recvbuf为房间编号
						player* p = (player*)malloc(sizeof(player));
						p->id = id;
						p->p = &(pokemon.begin()->second);
						p->pokemonId = pokemon.begin()->first;
						p->tid = GetCurrentThreadId();
						p->userpokemon = &pokemon;
						int roomnum = *(int*)recvbuf;
						PostThreadMessage(mCenter, ENTER_ROOM+BASE, roomnum, (LPARAM)p);
						MSG msg;
						GetMessage(&msg, 0, 0, 0);
						roomThread = msg.message;//0代表没进去，leave的时候改回
					}
					else if (state == CHANGE_POKEMON && roomThread != 0) {
						//传编号，注意应该是int*
						Pokemon* tempp;
						if (pokemon.find((*(int*)recvbuf)) != pokemon.end() && roomThread != 0) {
							tempp = &(pokemon.find((*(int*)recvbuf))->second);
							PostThreadMessage(roomThread, CHANGE_POKEMON+BASE, (WPARAM)tempp, (LPARAM)clientSocket);
						}
					}
					else if (state == ADD_POINT && roomThread == 0) {
						//recvbuf0是加点类型，1是小精灵编号
						int *i = (int*)(recvbuf + 1);
						pokemon.find(*i)->second.addPoint(recvbuf[0]);
					}
					else if (state == ATTACK) {
						//lParam传socket*
						//高16位攻击谁，低16位技能索引
						//暂时先算8位,recvbuf0是to，1是skill
						if (roomThread != 0) {
							unsigned int i = 0;
							unsigned int temp = recvbuf[0];
							temp <<= 16;
							i |= (temp);
							temp = recvbuf[1];
							i |= temp;
							PostThreadMessage(roomThread, ATTACK+BASE, i, (LPARAM)clientSocket);
						}
					}
					else if (state == EXCHANGE_POKEMON && roomThread != 0) {
						//recvbuf0是交换的精灵id不是战斗id
						int *a = (int*)recvbuf;
						PostThreadMessage(roomThread, EXCHANGE_POKEMON+BASE, *a, (LPARAM)clientSocket);
					}
					else if (state == ADD_AI && roomThread != 0) {
						string sss = is.str();
						virtualParameter* tempv = new virtualParameter;
						boost::archive::text_iarchive ia(is);
						ia >> *tempv;
						tempv->con = con;
						PostThreadMessage(roomThread, ADD_AI+BASE, (WPARAM)(tempv), (LPARAM)clientSocket);
					}
					else {

					}
					state = -1;
				}
			}
			else {
				printf("断开连接");//显示断线信息
				if (roomThread != 0) {
					identity1* id1temp = (identity1*)malloc(sizeof(identity1));
					memcpy(id1temp, &id, sizeof(identity1));
					if (PostThreadMessage(roomThread, LEAVE_ROOM + BASE, (WPARAM)id1temp, 0) == 0)
						roomThread = 0;
				}
				while (roomThread != 0) {
					Sleep(3000);
					goto here1;
					
				}
				PostThreadMessage(mCenter, LOGOUT + BASE, 0, GetCurrentThreadId());
				closesocket(*clientSocket);
				Sleep(2500);
				break;
			}
		}
//	}
//	catch (exception ex) {
//		closesocket(*clientSocket);
//		result == NULL ? delete result : 0;
//		con == NULL ? delete con : 0;
//		return 0;
//	}

	//关闭当前socket
	con == NULL ? 0 : delete con;
	for (map<int, Pokemon&>::iterator it = pokemon.begin(); it != pokemon.end(); it++)
		delete(&(it->second));
	return 0;
}

int main(int argc, char* argv) {
	unsigned short isDead = 32768;
	cout << ~isDead << endl;
	cout << (((unsigned short)(~isDead)) >> (16 - 1)) << endl;
	cout << ((((~isDead) >> (16 - 1)) == 0) || ((((~isDead) << 1) >> (16 - 1)) == 0)) << endl;

	dirver = get_driver_instance();
	SOCKET serverSocket = initSocket();
	if (serverSocket == 1)
		exit(1);

	vector<DWORD> threadID;
	vector<HANDLE> threadHandle;
	DWORD tID;
	//HANDLE* hID;
	//接受连接
	CreateThread(NULL, 0, messageCenter, 0, 0, &mCenter);
	while (true) {//跳出？鉴权用户？
		SOCKET* clientSocket = (SOCKET*)malloc(sizeof(SOCKET));
		*clientSocket = INVALID_SOCKET;
		*clientSocket = accept(serverSocket, NULL, NULL);
		if (*clientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
		}
		else CreateThread(NULL, 0, subThread, clientSocket, 0, &tID);
	}

	WSACleanup();
	return 0;
}

