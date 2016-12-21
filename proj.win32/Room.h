
#ifndef __ROOM_SCENE_H__
#define __ROOM_SCENE_H__

#include "cocos2d.h"
#include "proj.win32\head.h"
#include <vector>
#include <string>

class Room : public cocos2d::Layer {
public:
	static cocos2d::Scene* createScene();
	bool ACK;
	int nextId;
	virtual bool init();
	void update(float);
	// implement the "static create()" method manually
	CREATE_FUNC(Room);
	void pokemonCallBackFunc(Ref* sender);
	void addButton(bool isBlue, room* r, int i, bool isBlueTeam);
	void setRoom(room* r);
};
#endif 

