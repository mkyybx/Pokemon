
#ifndef _FIGHTINGPOKEMONLAYER_H
#define _FIGHTINGPOKEMONLAYER_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

class FightingPokemonLayer : public cocos2d::Layer {
public:
	FightingPokemonLayer();
	~FightingPokemonLayer();
	void setInitParameters(std::string name, char inittype, bool upDownFlip);
	void playMoveOnAnimation(bool play);
	Rect getBoundingBox() const;
	void startFlash();
	//差分是正数,percent+diff
	void setHPbar(int percent, bool isDifference);
	void exit();
	CREATE_FUNC(FightingPokemonLayer);
private:
	Sprite* sprite;
	Label* name;
	ui::LoadingBar* hpbar;
	bool upDownFlip;
	char inittype;
	bool isAnimationPlaying;
	GLubyte opacity = 255;
	GLubyte direction = 1;
	//buf??
	virtual bool init();
	void update(float);
	void flash(float);
	int targetHP;
	void changeHP(float);
	void changeOpacity(float);
};

#endif 