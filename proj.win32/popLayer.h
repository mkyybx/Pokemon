
#ifndef _POPLAYER_H
#define _POPLAYER_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class PopLayer : public cocos2d::LayerColor {
public:
	PopLayer();
	~PopLayer();
public:
	bool innershutdown;
	bool backToRoomListScene;
	virtual bool init();
	CREATE_FUNC(PopLayer);
	cocos2d::Label* text;
	cocos2d::ui::Button* button;
	void setParameters(bool isButtonShowed, cocos2d::String message, bool isShutdown);
	void PopLayer::setParameters(bool isButtonShowed, cocos2d::String message, bool isShutdown, bool backToRoomListScene);
};

#endif 