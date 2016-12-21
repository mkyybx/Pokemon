#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include "head.h"
#include "cocos2d.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "9638"
using namespace std;
queue<void*> pipe;
SOCKET s;
vector<pair<string, string>> initPokemonMap;
vector<fightingPokemon*> fightingpokemon;
vector<Attack> attackData;
bool pipeCanReadHelloWorld = false;
bool pipeCanReadRoomList = false;
bool pipeCanReadRoom = false;
bool pipeCanReadBattleScene = false;

void sendEvent(UINT msg, void* wpdata) {
	//外面free
	pipe.push((void*)msg);
	if (wpdata != (void*)(0xFFFFFFFF))
		pipe.push(wpdata);
	//PostThreadMessage(tid, msg, (WPARAM)wpdata, GetCurrentThreadId());
}

int myrecv(char *buf, int flags) {
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

int mysend(const char *buf, int len, int flags) {
	char* length = (char*)(&len);
	send(s, length, sizeof(int), flags);
	int state = send(s, buf, len, flags);
	if (state == -1) {
		char c = NETWORK_ERROR;
		sendEvent(c, (void*)(0xFFFFFFFF));
	}
	return state;
}

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

DWORD WINAPI networkThread(LPVOID IpParam) {
	srand(time(0));
	//客户端登录成功之后自动请求小精灵，并保存编号
	pipeCanReadHelloWorld = true;
	initPokemonMap.push_back({ "蚊香蛙", "wxw.png" });
	initPokemonMap.push_back({ "喷火龙", "phl.png" });
	initPokemonMap.push_back({ "卡比兽", "kbs.png" });
	initPokemonMap.push_back({ "妙蛙花", "mwh.png" });
	initPokemonMap.push_back({ "铁甲贝", "tjb.png" });
	initPokemonMap.push_back({ "穿山王", "csw.png" });
	initPokemonMap.push_back({ "闪电鸟", "sdn.png" });
	initPokemonMap.push_back({ "雷丘", "lq.png" });
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char recvbuf[1024];
	int recvbuflen;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		char c = NETWORK_ERROR;
		sendEvent(c, (void*)(0xFFFFFFFF));
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		char c = NETWORK_ERROR;
		sendEvent(c, (void*)(0xFFFFFFFF));
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
			char c = NETWORK_ERROR;
			sendEvent(c, (void*)(0xFFFFFFFF));
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
		char c = NETWORK_ERROR;
		sendEvent(c, (void*)(0xFFFFFFFF));
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	printf("connected");
	s = ConnectSocket;
	map<int, Pokemon&> pokemon;
	
	while (true) {
		char num;
		iResult = myrecv(recvbuf, 0);
		if (iResult <= 0) {
			printf("send failed with error: %d\n", WSAGetLastError());
			char flag = NETWORK_ERROR;
			sendEvent(flag, (void*)(0xFFFFFFFF));
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		if (iResult == 1 && recvbuf[0] == INVALID_USER_OR_PASS)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == REGISTER_FAIL)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == REGISTER_SUCCESS)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == LEAVE_ROOM)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == ENTER_ROOM)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == LEAVE_ROOM_ACK)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == CHANGE_POKMON_ACK)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == OWNER_CHANGED)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == TEAM_NOT_EQUAL)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == ANIMATION_COMPLETE)
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == LOGIN_SUCCESS) {
			sendEvent(recvbuf[0], (void*)(0xFFFFFFFF));
			myrecv(recvbuf, 0);
			{
				char num = recvbuf[0];
				for (int i = 0; i < num; i++) {
					myrecv(recvbuf, 0);
					char type = recvbuf[0];
					int id;
					myrecv(recvbuf, 0);
					istringstream is(recvbuf);
					boost::archive::text_iarchive ia(is);
					ia >> id;
					myrecv(recvbuf, 0);
					addPokemonMap(&pokemon, id, recvbuf, type);
				}
			}
			sendEvent(SYNC_POKEMON, &pokemon);
		}
		else if (iResult == 1 && recvbuf[0] == QUERY_ONLINE) {
			char* temprecvbuf = (char*)malloc(sizeof(char)* 100000);
			myrecv(temprecvbuf, 0);
			istringstream is(temprecvbuf);
			boost::archive::text_iarchive ia(is);
			vector<string>* tempv = new vector<string>;
			ia >> *tempv;
			free(temprecvbuf);
			//GUI去free tempv
			sendEvent(QUERY_ONLINE, tempv);
		}
		else if (iResult == 1 && recvbuf[0] == NO_SUCH_USER)
			sendEvent(NO_SUCH_USER, (void*)(0xFFFFFFFF));
		else if (iResult == 1 && recvbuf[0] == QUERY_POKEMON) {
			auto tempv = new vector<strengthPokemon>;
			while (myrecv(recvbuf, 0) == 1 && recvbuf[0] != RETURN_USER_INFO) {
				strengthPokemon sp;
				//跳过类型信息
				myrecv(recvbuf, 0);
				istringstream is(recvbuf);
				boost::archive::text_iarchive ia(is);
				ia >> sp;
				tempv->push_back(sp);
			}
			sendEvent(QUERY_POKEMON, tempv);
			//GUI去free tempv
			myrecv(recvbuf, 0);
			auto name = new string(recvbuf);
			auto tempi = new vector<int>;
			myrecv(recvbuf, 0);
			istringstream is(recvbuf);
			int i;
			is >> i;
			tempi->push_back(i);
			myrecv(recvbuf, 0);
			istringstream is1(recvbuf);
			is1 >> i;
			tempi->push_back(i);
			sendEvent((UINT)name, tempi);
			//GUI去free tempi
			//GUI去free name
		}
		else if (iResult == 1 && recvbuf[0] == LIST_ROOM) {
			try {
				myrecv(recvbuf, 0);
				int num = *(int*)recvbuf;
				auto tempv = new vector<listroom>;
				for (int i = 0; i < num; i++) {
					myrecv(recvbuf, 0);
					listroom templ;
					istringstream is(recvbuf);
					boost::archive::text_iarchive ia(is);
					ia >> templ;
					tempv->push_back(templ);
				}
				sort(tempv->begin(), tempv->end());
				sendEvent(LIST_ROOM, tempv);
				//GUI去free tempv
			}
			catch (exception ex) {
				CCLOG(ex.what());
			}
		}
		else if (iResult == 1 && recvbuf[0] == BATTLE_START) {
			//头|个数|红队人数|(类型)数据|自身编号
			myrecv(recvbuf, 0);
			num = recvbuf[0];
			myrecv(recvbuf, 0);
			char rednum = recvbuf[0];
			for (int i = 0; i < num; i++) {
				myrecv(recvbuf, 0);
				char type = recvbuf[0];
				auto apo = new fightingPokemon();
				switch (type) {
				case AGILE:
					apo = new agilePokemon();
					break;
				case MEET:
					apo = new meetPokemon();
					break;
				case DEFENSE:
					apo = new defensePokemon();
					break;
				case STRENGTH:
					apo = new strengthPokemon();
					break;
				default:
					break;
				}
				myrecv(recvbuf, 0);
				istringstream is(recvbuf);
				boost::archive::text_iarchive ia(is);
				ia >> *apo;
				fightingpokemon.push_back(apo);
				//网络线程在结束战斗后free fightingpokemon里面的内容
			}
			myrecv(recvbuf, 0);
			char mynum = recvbuf[0];
			sendEvent(BATTLE_START, (void*)num);
			sendEvent(rednum, &fightingpokemon);
			sendEvent(mynum, (void*)(0xFFFFFFFF));
		}
		else if (iResult == 1 && recvbuf[0] == ROOM_CHANGED) {
			auto myroom = new room();
			myrecv(recvbuf, 0);
			istringstream is(recvbuf);
			boost::archive::text_iarchive ia(is);
			ia >> *myroom;
			sendEvent(ROOM_CHANGED, myroom);
			//GUI去free myroom
		}
		else if (iResult == 1 && recvbuf[0] == ATTACK_DATA) {
			for (int i = 0; i < num; i++) {
				myrecv(recvbuf, 0);
				istringstream is(recvbuf);
				boost::archive::text_iarchive ia(is);
				ia >> *fightingpokemon[i];
			}
			myrecv(recvbuf, 0);
			istringstream is(recvbuf);
			boost::archive::text_iarchive ia(is);
			ia >> attackData;
			sendEvent(ATTACK_DATA, &attackData);
			//GUI应该复制最后free
		}
		else if (iResult == 1 && recvbuf[0] == BATTLE_FINISH) {
			char isBlueWin;
			myrecv(recvbuf, 0);
			isBlueWin = recvbuf[0];
			
			myrecv(recvbuf, 0);
			char temp1 = recvbuf[0];
			myrecv(recvbuf, 0);
	//		for (int i = 0; i < fightingpokemon.size(); i++)
	//			delete fightingpokemon[i];
			fightingpokemon.clear();
			sendEvent(BATTLE_FINISH, (void*)isBlueWin);
			sendEvent(temp1, (void*)0xFFFFFFFF);
			if (recvbuf[0] == SYNC_POKEMON)
				goto syc;
		}
		else if (iResult == 1 && recvbuf[0] == SYNC_POKEMON) {
		syc:
			myrecv(recvbuf, 0);
			{
				char num = recvbuf[0];
				for (auto it = pokemon.begin(); it != pokemon.end(); it++)
					delete(&(it->second));
				pokemon.clear();
				for (int i = 0; i < num; i++) {
					myrecv(recvbuf, 0);
					char type = recvbuf[0];
					int id;
					myrecv(recvbuf, 0);
					istringstream is(recvbuf);
					boost::archive::text_iarchive ia(is);
					ia >> id;
					myrecv(recvbuf, 0);
					addPokemonMap(&pokemon, id, recvbuf, type);
				}
			}
			sendEvent(SYNC_POKEMON, (void*)0xFFFFFFFF);
			sendEvent(SYNC_POKEMON, &pokemon);
		}
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
	return 0;
}
