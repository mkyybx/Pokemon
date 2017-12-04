#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#ifndef SOCKET_H
#define SOCKET_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>
#include "head.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;
extern void getRandomPokemon(LoginInfo login, sql::Connection *con, int num, bool sendBack, map<int, Pokemon&>* pokemon, int specified, int level);
extern int myrecv(SOCKET s, char *buf, int len, int flags);
extern int mysend(SOCKET s, const char *buf, int len, int flags);
extern sql::Driver *dirver;

class clientRoom {
public:
	vector<string> reduser;
	vector<string> blueuser;
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & reduser;
		ar & blueuser;
	}
};

void addPokemonMap(map<int, Pokemon&>* returnResult, int id, const char* recvbuf, char type) {
	string str(recvbuf);
	Pokemon* finalpo;
	switch (type) {
	case AGILE:
	{
				  agilePokemon* apo = new agilePokemon();
				  istringstream iis(str);
				  boost::archive::text_iarchive ia(iis);
				  ia >> *apo;
				  finalpo = apo;
	}
		break;
	case MEET:
	{
				 meetPokemon* apo = new meetPokemon();
				 istringstream iis(str);
				 boost::archive::text_iarchive ia(iis);
				 ia >> *apo;
				 finalpo = apo;
	}
		break;
	case DEFENSE:
	{
					defensePokemon* apo = new defensePokemon();
					istringstream iis(str);
					boost::archive::text_iarchive ia(iis);
					ia >> *apo;
					finalpo = apo;
	}
		break;
	case STRENGTH:
	{
					 strengthPokemon* apo = new strengthPokemon();
					 istringstream iis(str);
					 boost::archive::text_iarchive ia(iis);
					 ia >> *apo;
					 finalpo = apo;
	}
		break;
	}
	pair<int, Pokemon&> pairp(id, *finalpo);
	returnResult->insert(pairp);
}

void beforeExit(sql::Connection* con, LoginInfo login, map<int, Pokemon&>* pokemon, SOCKET* socket, virtualParameter* param, const char* sendbuf) {
	for (map<int, Pokemon&>::iterator it = pokemon->begin(); it != pokemon->end(); it++) {
		sql::PreparedStatement* pstmt = con->prepareStatement("DELETE FROM pokemon WHERE idpokemon = ?;");
		pstmt->setInt(1, it->first);
		pstmt->execute();
		delete pstmt;
		delete &(it->second);
	}
	sql::PreparedStatement* pstmt = con->prepareStatement("DELETE FROM user WHERE iduser = ?;");
	pstmt->setString(1, login.user);
	pstmt->execute();
	if (shutdown(*socket, SD_SEND) == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(*socket);
	}
	delete param;
	delete (sendbuf);
	delete con;
}

DWORD WINAPI fakeClient(LPVOID IpParam) {
	//IpParam应包括，initpokemon编号，等级，房间id，红蓝队，里面free，随机带技能（体现在getrandom里）
	virtualParameter* param = (virtualParameter*)IpParam;
	sql::Connection* con = dirver->connect("localhost", "mky0", "123456");
	con->setSchema("pokemon");
	con->setClientOption("characterSetResults", "utf8");
	LoginInfo login;
	try {
		sql::PreparedStatement* pstmt = con->prepareStatement("select name from initpokemon where idinitpokemon = ?;");
		pstmt->setInt(1, param->initpokemonid);
		sql::ResultSet* result = pstmt->executeQuery();
		result->next();
		string temps = result->getString("name").c_str();
		memcpy(login.user, temps.c_str(), temps.size());
		boost::uuids::random_generator rgen;
		boost::uuids::uuid uu = rgen();
		ostringstream ss;
		ss << uu;
		memcpy(login.user + temps.size(), ss.str().c_str(), 20 - temps.size());
		login.user[19] = 0;
		login.pass[0] = '1';
		login.pass[1] = 0;
		pstmt = con->prepareStatement("INSERT INTO user (iduser,password) VALUES (?, ?)");
		pstmt->setString(1, login.user);
		pstmt->setString(2, login.pass);
		pstmt->execute();
		delete pstmt;
	}
	catch (sql::SQLException ex){
		cout << ex.what();
	}
	getRandomPokemon(login, con, 2, false, 0, param->initpokemonid - 1, param->level);
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char* sendbuf = (char*)malloc(sizeof(char)* 1024);// [1024];
	char recvbuf[1024];
	int sendbuflen;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port

	iResult = getaddrinfo("127.0.0.1", "9638", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	printf("connected");
	char flag;
	char num;
	bool hasStarted = false;
	map<int, Pokemon&> pokemon;
	flag = LOGIN;
	mysend(ConnectSocket, &flag, 1, 0);
	ostringstream os;
	boost::archive::text_oarchive oa(os);
	oa << login;
	mysend(ConnectSocket, os.str().c_str(), os.str().size(), 0);
	//登录成功
	myrecv(ConnectSocket, recvbuf, 1024, 0);
	//接收小精灵
	myrecv(ConnectSocket, recvbuf, 1024, 0); 
	{
		num = recvbuf[0];
		for (int i = 0; i < num; i++) {
			myrecv(ConnectSocket, recvbuf, 1024, 0);
			char type = recvbuf[0];
			int id;
			myrecv(ConnectSocket, recvbuf, 1024, 0);
			istringstream is(recvbuf);
			boost::archive::text_iarchive ia(is);
			ia >> id;
			myrecv(ConnectSocket, recvbuf, 1024, 0);
			addPokemonMap(&pokemon, id, recvbuf, type);
		}
	}
	//进入房间
//	system("pause");
	flag = ENTER_ROOM;
	mysend(ConnectSocket, &flag, 1, 0);
	int roomnum = param->room;
	mysend(ConnectSocket, (char*)&roomnum, sizeof(int), 0);
	//如果没进去
	myrecv(ConnectSocket, recvbuf, 1, 0);
	if (recvbuf[0] == LEAVE_ROOM) {
		beforeExit(con, login, &pokemon, &ConnectSocket, param, sendbuf);
		return -1;
	}
	//进去了
	while (true) {
		int recvlength = myrecv(ConnectSocket, recvbuf, 1, 0);
		if (recvlength == 1 && recvbuf[0] == OWNER_CHANGED) {
			//得到房主后证明没人了，退出
			flag = LEAVE_ROOM;
			mysend(ConnectSocket, &flag, 1, 0);
			beforeExit(con, login, &pokemon, &ConnectSocket, param, sendbuf);
			return -1;
		}
		else if (recvlength == 1 && recvbuf[0] == ROOM_CHANGED) {
			//看队对不对
			clientRoom r;
			myrecv(ConnectSocket, recvbuf, 1024, 0);
			istringstream is(recvbuf);
			boost::archive::text_iarchive ia(is);
			ia >> r;
			if (((find(r.reduser.begin(), r.reduser.end(), login.user) != r.reduser.end()) && param->isBlue) || ((find(r.blueuser.begin(), r.blueuser.end(), login.user) != r.blueuser.end()) && !(param->isBlue))) {
				//要改
				flag = CHANGE_TEAM;
				mysend(ConnectSocket, &flag, 1, 0);
				while (true) {
					//等ACK
					int recvlength = myrecv(ConnectSocket, recvbuf, 1, 0);
					if (recvlength == 1 && (recvbuf[0] == CHANGE_TEAM_ACK || BATTLE_START))
						break;
					else if (recvlength == 1 && recvbuf[0] == CHANGE_TEAM_NAK) {
						flag = LEAVE_ROOM;
						mysend(ConnectSocket, &flag, 1, 0);
						beforeExit(con, login, &pokemon, &ConnectSocket, param, sendbuf);
						return -1;
					}
				}
			}
			break;
		}
	}
	//等待接收开始信息
	while (!hasStarted) {
		int recvlength = myrecv(ConnectSocket, recvbuf, 1, 0);
		if (recvlength == 1 && recvbuf[0] == BATTLE_START) {
			hasStarted = true;
			break;
		}
		if (recvlength == 1 && recvbuf[0] == OWNER_CHANGED) {
			//得到房主后证明没人了，退出
			flag = LEAVE_ROOM;
			mysend(ConnectSocket, &flag, 1, 0);
			beforeExit(con, login, &pokemon, &ConnectSocket, param, sendbuf);
			return 0;
		}
	}
	myrecv(ConnectSocket, recvbuf, 1, 0);
	char pokemonnum = recvbuf[0];
	myrecv(ConnectSocket, recvbuf, 1, 0);
	char rednum = recvbuf[0];
	fightingPokemon* p = new fightingPokemon[pokemonnum];
	for (int i = 0; i < pokemonnum; i++) {
		//跳过类型
		myrecv(ConnectSocket, recvbuf, 1, 0);
		myrecv(ConnectSocket, recvbuf, 1024, 0);
		istringstream is(recvbuf);
		boost::archive::text_iarchive ia(is);
		ia >> p[i];
	}
	myrecv(ConnectSocket, recvbuf, 1, 0);
	char me = recvbuf[0];
	//开始
	//判断己方对方
	fightingPokemon* opponent;
	int opponentlen;
	int opponentbegin;
	int backup;
	if (me < rednum) {
		opponentbegin = rednum;
		opponent = p + rednum;
		opponentlen = pokemonnum - rednum;
	}
	else {
		opponentbegin = 0;
		opponent = p;
		opponentlen = rednum;
	}
repeat:
	if (!p[me].isDead) {
		bool selected = false;
		if (true) {//rand() % 2 == 0) {
			//不用技能
			int mydamage = pokemon.begin()->second.attackDamage.returnValue();
			for (int i = 0; i < opponentlen; i++) {
				if (!((opponent + i)->isDead)) {
					if ((opponent + i)->hp < mydamage) {
						//选定
						sendbuf[0] = opponentbegin + i;
						sendbuf[1] = 0;
						selected = true;
						break;
					}
					else backup = i;
				}
			}
			if (!selected) {
				sendbuf[0] = opponentbegin + backup;
				sendbuf[1] = 0;
			}
		}
		else {
			//用技能
			;
		}
		//发送
		flag = ATTACK;
		mysend(ConnectSocket, &flag, 1, 0);
		mysend(ConnectSocket, sendbuf, 2, 0);
	}
	//接收attackdata
	while (!(myrecv(ConnectSocket, recvbuf, 1, 0) == 1 && recvbuf[0] == ATTACK_DATA));
	for (int i = 0; i < pokemonnum; i++) {
		myrecv(ConnectSocket, recvbuf, 1024, 0);
		istringstream is(recvbuf);
		boost::archive::text_iarchive ia(is);
		ia >> p[i];
	}
	//跳过接收attack数据
	myrecv(ConnectSocket, recvbuf, 1024, 0);
	//animationcomplete
	flag = ANIMATION_COMPLETE;
	mysend(ConnectSocket, &flag, 1, 0);
	myrecv(ConnectSocket, recvbuf, 1, 0);
	if (recvbuf[0] == ANIMATION_COMPLETE)
		goto repeat;
	else if (recvbuf[0] == BATTLE_FINISH) {
		myrecv(ConnectSocket, recvbuf, 1, 0);
		bool isBlueWin = (recvbuf[0] == 1 ? true : false);
		myrecv(ConnectSocket, recvbuf, 1, 0);
		if (recvbuf[0] == EXCHANGE_POKEMON) {
			if (((opponentbegin == 0) ^ isBlueWin)) {
				myrecv(ConnectSocket, recvbuf, 1, 0);
				char flag = EXCHANGE_POKEMON;
				mysend(ConnectSocket, &flag, 1, 0);
				*(int*)sendbuf = pokemon.begin()->first;
				mysend(ConnectSocket, sendbuf, sizeof(int), 0);
			}
		}
		else if (recvbuf[0] == BATTLE_FINISH)
			;
	}
	while (myrecv(ConnectSocket, recvbuf, 1, 0) != 1 || recvbuf[0] != SYNC_POKEMON);
	for (auto it = pokemon.begin(); it != pokemon.end(); it++)
		delete &(it->second);
	pokemon.clear();
	myrecv(ConnectSocket, recvbuf, 1024, 0);
	{
		num = recvbuf[0];
		for (int i = 0; i < num; i++) {
			myrecv(ConnectSocket, recvbuf, 1024, 0);
			char type = recvbuf[0];
			int id;
			myrecv(ConnectSocket, recvbuf, 1024, 0);
			istringstream is(recvbuf);
			boost::archive::text_iarchive ia(is);
			ia >> id;
			myrecv(ConnectSocket, recvbuf, 1024, 0);
			addPokemonMap(&pokemon, id, recvbuf, type);
		}
	}
	beforeExit(con, login, &pokemon, &ConnectSocket, param, sendbuf);
	return 0;

			
		

	

	while (true) {

		iResult = mysend(ConnectSocket, sendbuf, sendbuflen, 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		printf("Bytes Sent: %ld\n", iResult);
	}


	/*
	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
	printf("shutdown failed with error: %d\n", WSAGetLastError());
	closesocket(ConnectSocket);
	WSACleanup();
	return 1;
	}

	// Receive until the peer closes the connection
	do {

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0)
	printf("Bytes received: %d\n", iResult);
	else if (iResult == 0)
	printf("Connection closed\n");
	else
	printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	*/
}