
#ifndef _FLOATLABEL_H
#define _FLOATLABEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "head.h"

USING_NS_CC;

class FloatLabel : public cocos2d::Layer {
public:
	FloatLabel();
	~FloatLabel();
public:
	virtual bool init();
	CREATE_FUNC(FloatLabel);
	cocos2d::Label* text;
	int velocity;
	GLubyte opactity = 255;
	void setParameters(std::string message, int velocity, Color4B color, int size);
	void update(float);
};

#endif 