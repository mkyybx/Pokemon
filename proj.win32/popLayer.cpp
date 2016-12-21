// PopLayer.cpp

#include "popLayer.h"
#include "SimpleAudioEngine.h"
#include "RoomList.h"
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
USING_NS_CC;

// ���캯�������ж����Ա����Ҫ�ڴ˳�ʼ��
PopLayer::PopLayer() {}
// ����
PopLayer::~PopLayer(){}

//�麯������ʼ������ִ�б����create()����ʱ�ᱻ�Զ�����
bool PopLayer::init(){
	bool bl = LayerColor::initWithColor(ccc4(255, 252, 255, 128));
	// ��������
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto dialogBackground = Sprite::create("dialog.png");
	dialogBackground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	button = ui::Button::create("ok.png");
	button->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2 - dialogBackground->getContentSize().height / 2));
	text = Label::createWithTTF("", "fonts/simyou.ttf", 36);
	text->setTextColor(Color4B::BLACK);
	text->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	this->addChild(button, 2);
	this->addChild(dialogBackground, 0);
	this->addChild(text, 1);
	auto maskListener = EventListenerTouchOneByOne::create();
	button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type){
		switch (type) {
		case ui::Widget::TouchEventType::BEGAN:
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
			break;
		case ui::Widget::TouchEventType::ENDED:
			if (this->innershutdown)
				exit(0);
			if (backToRoomListScene)
				Director::getInstance()->replaceScene(TransitionFade::create(1.5, RoomList::createScene()));
			this->removeFromParentAndCleanup(true);
			break;
		default:
			break;
		}
	});
	
	maskListener->onTouchBegan = [=](Touch *touch, Event *event) {
		maskListener->setSwallowTouches(true);
		return true;
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(maskListener, this);
	
	return bl;
	
}

void PopLayer::setParameters(bool isButtonShowed, cocos2d::String message, bool isShutdown) {
	text->setString(message.getCString());
	button->setVisible(isButtonShowed);
	innershutdown = isShutdown;
	backToRoomListScene = false;
}

void PopLayer::setParameters(bool isButtonShowed, cocos2d::String message, bool isShutdown, bool backToRoomListScene) {
	text->setString(message.getCString());
	button->setVisible(isButtonShowed);
	innershutdown = isShutdown;
	this->backToRoomListScene = backToRoomListScene;
}


