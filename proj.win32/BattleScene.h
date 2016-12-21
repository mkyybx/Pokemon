
#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "head.h"

USING_NS_CC;

class BattleScene : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene();

	virtual bool init();

	// a selector callback
	void BattleScene::update(float);
	void onMouseMove(Event *event);
	void onMouseUp(Event *event);
	void timer(float);
	void applyAttackData(vector<Attack> att);
	//两个参数，type和fireposition
	void setFire(Node*, void*);
	void die(Node*, void*);
	void syncPokemon(Node*, void*);
	void playWaingBar(float);
	int time = 30;
	// implement the "static create()" method manually
	CREATE_FUNC(BattleScene);
};
#endif // __HELLOWORLD_SCENE_H__