
#include "registerLayer.h"
#include "SimpleAudioEngine.h"
#include "popLayer.h"
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
USING_NS_CC;
extern void regist(const char user[20], const unsigned char* pass, int length);
extern PopLayer* registering;
// ���캯�������ж����Ա����Ҫ�ڴ˳�ʼ��
RegisterLayer::RegisterLayer() {}
// ����
RegisterLayer::~RegisterLayer(){}

//�麯������ʼ������ִ�б����create()����ʱ�ᱻ�Զ�����
bool RegisterLayer::init(){
	bool bl = LayerColor::initWithColor(ccc4(255, 252, 255, 128));
	// ��������
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	//����
	auto dialogBackground = Sprite::create("dialog.png");
	dialogBackground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	this->addChild(dialogBackground,50);
	//��ť
	auto okButton = ui::Button::create("ok.png");
	okButton->setPosition(Vec2(origin.x + visibleSize.width / 2 - 150, origin.y + visibleSize.height / 2 - dialogBackground->getContentSize().height / 2));
	auto cancelButton = ui::Button::create("cancel.png");
	cancelButton->setPosition(Vec2(origin.x + visibleSize.width / 2 + 150, origin.y + visibleSize.height / 2 - dialogBackground->getContentSize().height / 2));
	this->addChild(okButton,51);
	this->addChild(cancelButton,51);
	//�ı�
	auto userText = Label::createWithTTF("�� �� ��", "fonts/simyou.ttf1", 36);
	userText->setTextColor(Color4B::BLACK);
	userText->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200, origin.y + visibleSize.height / 2 + 40));
	auto passText = Label::createWithTTF("��    ��", "fonts/simyou.ttf1", 36);
	passText->setTextColor(Color4B::BLACK);
	passText->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200, origin.y + visibleSize.height / 2));
	auto confirmPassText = Label::createWithTTF("ȷ������", "fonts/simyou.ttf1", 36);
	confirmPassText->setTextColor(Color4B::BLACK);
	confirmPassText->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200, origin.y + visibleSize.height / 2 - 40));
	this->addChild(userText,51);
	this->addChild(passText,51);
	this->addChild(confirmPassText, 51);
	//�ı���
	auto userTextField = ui::TextField::create("_______________", "fonts/simyou.ttf1", 48);
	auto passTextField = ui::TextField::create("_______________", "fonts/simyou.ttf1", 48);
	auto confirmPassTextField = ui::TextField::create("_______________", "fonts/simyou.ttf1", 48);
	userTextField->setMaxLength(20);
	userTextField->setMaxLengthEnabled(true);
	passTextField->setMaxLength(20);
	passTextField->setPasswordStyleText("*");
	passTextField->setPasswordEnabled(true);
	passTextField->setMaxLengthEnabled(true);
	confirmPassTextField->setMaxLength(20);
	confirmPassTextField->setPasswordStyleText("*");
	confirmPassTextField->setPasswordEnabled(true);
	confirmPassTextField->setMaxLengthEnabled(true);
	userTextField->setColor(Color3B::BLACK);
	passTextField->setColor(Color3B::BLACK);
	confirmPassTextField->setColor(Color3B::BLACK);
	userTextField->setPosition(Vec2(origin.x + visibleSize.width / 2 + 100, origin.y + visibleSize.height / 2 + 40));
	passTextField->setPosition(Vec2(origin.x + visibleSize.width / 2 + 100, origin.y + visibleSize.height / 2));
	confirmPassTextField->setPosition(Vec2(origin.x + visibleSize.width / 2 + 100, origin.y + visibleSize.height / 2 - 40));
	this->addChild(userTextField, 51);
	this->addChild(passTextField, 51);
	this->addChild(confirmPassTextField, 51);
	//������
	cancelButton->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type){
		switch (type) {
		case ui::Widget::TouchEventType::BEGAN:
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
			break;
		case ui::Widget::TouchEventType::ENDED:
			this->removeFromParentAndCleanup(true);
			break;
		default:
			break;
		}
	});
	okButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
		switch (type) {
		case ui::Widget::TouchEventType::BEGAN:
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
			break;
		case ui::Widget::TouchEventType::ENDED:
			CCLOG("%s\n",passTextField->getString());
			CCLOG("%s\n", confirmPassTextField->getString());
			CCLOG("%d\n", passTextField->getString() == confirmPassText->getString());
			if (passTextField->getString() == confirmPassTextField->getString()) {
				//?
				regist(userTextField->getString().c_str(), (const unsigned char*)(passTextField->getString().c_str()), passTextField->getStringLength());
				registering = PopLayer::create();
				registering->setParameters(false, "ע���С���", false);
				
				this->getParent()->addChild(registering, 10);
				this->removeFromParentAndCleanup(true);
			}
			else {
				auto poplayer = PopLayer::create();
				poplayer->setParameters(true, "�����������벻һ��\n����������", false);
				this->addChild(poplayer, 80);
			}
			break;
		default:
			break;
		}
	});
	auto maskListener = EventListenerTouchOneByOne::create();
	maskListener->onTouchBegan = [=](Touch *touch, Event *event) {
		maskListener->setSwallowTouches(true);
		return true;
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(maskListener, this);
	return bl;
}