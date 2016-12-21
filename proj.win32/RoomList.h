
#ifndef __ROOMLIST_SCENE_H__
#define __ROOMLIST_SCENE_H__

#include "cocos2d.h"
#include "proj.win32\head.h"
#include <vector>
#include <string>

class RoomList : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene();

	virtual bool init();
	void update(float);
	void drawBadgeAndPokemon();
	void pokemonCallBackFunc(Ref* sender);
	void roomListCallBackFunc(Ref* sender);
	void playerListCallBackFunc(Ref* sender);
	void setRoomList(vector<listroom>* tempv);
	void setPlayerList(vector<string>* tempv);
	bool hasDrawed = false;
	// implement the "static create()" method manually
	CREATE_FUNC(RoomList);
};
#endif 

