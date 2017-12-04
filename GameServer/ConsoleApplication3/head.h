#ifndef HEAD_H
#define HEAD_H
#ifndef SOCKET_H
#define SOCKET_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include "pokemon.cpp"
#include<cppconn\driver.h>
#include<cppconn\exception.h>
#include <cppconn/resultset.h> 
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include<mysql_connection.h>
#include <queue>


//房间外属性
#define LOGIN 127
#define INVALID_USER_OR_PASS 126
#define LOGIN_SUCCESS 125
#define REGISTER 124
#define REGISTER_SUCCESS 123
#define REGISTER_FAIL 122
#define LOGOUT 121
#define QUERY_ONLINE 120
#define QUERY_POKEMON 119
#define RETURN_USER_INFO 118
#define NO_SUCH_USER 117
#define SYNC_POKEMON 116
#define ADD_POINT 115//没写
//房间属性
#define ENTER_ROOM 114
#define LEAVE_ROOM 113
#define LEAVE_ROOM_ACK 112
#define CHANGE_TEAM 111
#define CHANGE_TEAM_ACK 110
#define CHANGE_TEAM_NAK 109
#define CHANGE_POKEMON 108
#define CHANGE_POKMON_ACK 107
#define BATTLE_START 106
#define OWNER_CHANGED 105
#define TEAM_NOT_EQUAL 104
#define ADD_AI 103
#define ROOM_CHANGED 102

//房间列表属性
#define LIST_ROOM 101
#define CREATE_ROOM 100
#define ROOM_EMPTY 99
//战斗属性
#define ATTACK 98
#define ATTACK_DATA 97
#define ANIMATION_COMPLETE 96
#define BATTLE_FINISH 95
#define EXCHANGE_POKEMON 94
#define TIME_OUT 93
#define STOP_TIMER 192
//战斗服务器状态
#define WAIT_FOR_ATTACK 91
#define WAIT_FOR_ANIMATION 90
#define WAIT_FOR_EXCHANGE 89
#define LOGIN_QUERY 88

#define BASE 5000

class LoginInfo {
public:
	char user[20];
	char pass[41];

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & user;
		ar & pass;
	}
};

struct identity1 {
	SOCKET* socket;
	LoginInfo* linfo;
};

struct player {
	identity1 id;
	Pokemon* p;
	int pokemonId;
	DWORD tid;
	map<int, Pokemon&>* userpokemon;
};

class listroom {
public:
	listroom() {
		;
	}
	identity1 owner;
	unsigned int id;
	bool isDuel;
	bool isStarted = false;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		char c = isDuel ? 1 : 0;
		char d = isStarted ? 1 : 0;
		string owner = this->owner.linfo->user;
		ar & owner;
		ar & id;
		ar & c;
		ar & d;
		isDuel = (c == 1 ? true : false);
		isStarted = (d == 1 ? true : false);
	}
};

class virtualParameter {
public:
	int initpokemonid;
	int level;
	int room;
	bool isBlue;
	sql::Connection* con;
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		char c = isBlue ? 1 : 0;
		ar & initpokemonid;
		ar & level;
		ar & c;
		isBlue = (c == 1 ? true : false);
	}
};
#endif
