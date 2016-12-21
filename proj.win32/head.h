#ifndef _MYHEAD_H
#define _MYHEAD_H

#include "pokemon.cpp"
#include <queue>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


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
#define STOP_TIMER 92
//战斗服务器状态
#define WAIT_FOR_ATTACK 91
#define WAIT_FOR_ANIMATION 90
#define WAIT_FOR_EXCHANGE 89
//客户端属性
#define NETWORK_ERROR 88


class listroom {
public:
	listroom() {
		;
	}
	string owner;
	unsigned int id;
	bool isDuel;
	bool isStarted = false;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		char c = isDuel ? 1 : 0;
		char d = isStarted ? 1 : 0;
		ar & owner;
		ar & id;
		ar & c;
		ar & d;
		isDuel = (c == 1 ? true : false);
		isStarted = (d == 1 ? true : false);
	}
	bool operator < (const listroom&b) {
		return id > b.id;//大到小
	}
};

class room : public listroom {
public:
	vector<string> reduser;
	vector<string> blueuser;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & reduser;
		ar & blueuser;
		ar & owner;
	}
	room() {
		;
	}
};

class virtualParameter {
public:
	int initpokemonid;
	int level;
	int room;
	bool isBlue;
	//sql::Connection* con;
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