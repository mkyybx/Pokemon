

#include "FloatLabel.h"
#include "SimpleAudioEngine.h"
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
USING_NS_CC;

// ���캯�������ж����Ա����Ҫ�ڴ˳�ʼ��
FloatLabel::FloatLabel() {}
// ����
FloatLabel::~FloatLabel(){}

//�麯������ʼ������ִ�б����create()����ʱ�ᱻ�Զ�����
bool FloatLabel::init(){
	bool bl = Layer::init();
	this->setAnchorPoint(Vec2(0.5, 0.5));
	text = Label::createWithTTF("", "fonts/simyou.ttf", 36);
	text->setPosition(Vec2(0, 0));
	this->addChild(text, 1);
	this->scheduleUpdate();
	return bl;
}

void FloatLabel::update(float) {
	text->setOpacity(opactity--);
	text->setPosition(Vec2(text->getPosition().x, text->getPosition().y + velocity));
	if (opactity == 0)
		this->removeFromParent();
}

void FloatLabel::setParameters(std::string message, int velocity, Color4B color, int size) {
	this->velocity = velocity;
	text->setString(message);
	text->setTextColor(color);
	text->setSystemFontSize(size);
}


