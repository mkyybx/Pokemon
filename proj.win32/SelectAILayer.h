
#ifndef _SELECTAILAYER_H
#define _SELECTAILAYER_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class SelectAILayer : public cocos2d::LayerColor {
public:
	SelectAILayer();
	~SelectAILayer();
	bool isBlue;
public:
	virtual bool init();
	CREATE_FUNC(SelectAILayer);
	void setIsBlue(bool isBlue);
};

#endif 