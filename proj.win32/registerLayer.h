
#ifndef _REGISTERLAYER_H
#define _REGISTERLAYER_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class RegisterLayer : public cocos2d::LayerColor {
public:
	RegisterLayer();
	~RegisterLayer();
public:
	virtual bool init();
	CREATE_FUNC(RegisterLayer);
};

#endif 