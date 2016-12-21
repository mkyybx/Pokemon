
#include "SelectAILayer.h"
#include "SimpleAudioEngine.h"
#include "popLayer.h"
#include "head.h"
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
extern vector<pair<string, string>> initPokemonMap;
extern int mysend(const char *buf, int len, int flags);
USING_NS_CC;
ui::TextField* levelBox;
// 构造函数，如有对象成员，需要在此初始化
SelectAILayer::SelectAILayer() {}
// 析构
SelectAILayer::~SelectAILayer(){}

void SelectAILayer::setIsBlue(bool isBlue) {
	this->isBlue = isBlue;
}

//虚函数，初始化，当执行本类的create()函数时会被自动调用
bool SelectAILayer::init(){
	bool bl = LayerColor::initWithColor(ccc4(255, 252, 255, 128));
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();
	//背景
	auto dialogBackground = Sprite::create("dialog.png");
	dialogBackground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	this->addChild(dialogBackground, 0);
	origin = Vec2(origin.x + visibleSize.width / 2 - dialogBackground->getContentSize().width / 2, origin.y + visibleSize.height / 2 - dialogBackground->getContentSize().height / 2);
	visibleSize = dialogBackground->getContentSize();

	auto scrollList = ui::ScrollView::create();
	scrollList->setDirection(ui::ScrollView::Direction::VERTICAL);
	scrollList->setContentSize(Size(visibleSize.width - 200, visibleSize.height - 30));
	scrollList->setInnerContainerSize(Size(visibleSize.width - 100, (initPokemonMap.size() / 9 + 1) * visibleSize.height));
	scrollList->setBounceEnabled(true);
	scrollList->setAnchorPoint(Vec2(0.5, 0.5));
	scrollList->setPosition(Vec2(visibleSize.width / 2  - 100 + origin.x, visibleSize.height / 2 + origin.y));
	this->addChild(scrollList, 4);

	//小精灵
	int row = 1;
	int col = 1;
	for (auto it = initPokemonMap.begin(); it != initPokemonMap.end(); it++) {
		auto frame = Sprite::create("frame.png");
		frame->setScale(120.0 / frame->getContentSize().width, 110.0 / frame->getContentSize().height);
		auto button = ui::Button::create(it->second);
		button->setScale(100.0 / button->getContentSize().height);
		button->setTag((row - 1) * 3 + col - 1);
		button->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  string s = levelBox->getString();
													  istringstream is(s);
													  int level;
													  if (is >> level && level <= MAX_LEVEL && level > 0) {
														  virtualParameter vp;
														  vp.initpokemonid = (row - 1) * 3 + col;
														  vp.isBlue = this->isBlue;
														  vp.level = level;
														  char flag = ADD_AI;
														  mysend(&flag, 1, 0);
														  ostringstream os;
														  boost::archive::text_oarchive oa(os);
														  oa << vp;
														  string sss = os.str();
														  mysend(os.str().c_str(), os.str().size(), 0);
														  this->removeFromParentAndCleanup(true);
													  }
													  else {
														  auto popLayer = PopLayer::create();
														  popLayer->setParameters(true, "等级输入的不对哦~", false);
														  this->addChild(popLayer, 70);
													  }
			}
				break;
			default:
				break;
			}
		});
		button->setPosition(Vec2(scrollList->getContentSize().width / 3 * col - 85, scrollList->getInnerContainerSize().height - (row - 1) * 150 - 60));
		frame->setPosition(Vec2(scrollList->getContentSize().width / 3 * col - 85, scrollList->getInnerContainerSize().height - (row - 1) * 150 - 60));
		scrollList->addChild(frame, 1);
		scrollList->addChild(button, 1);
		if (++col > 3) {
			col = 1;
			row++;
		}
	}

	//取消按钮
	auto cancelButton = ui::Button::create("cancel.png");
	cancelButton->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y));
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
	this->addChild(cancelButton, 5);
	//等级输入框
	auto levelLabel = Label::createWithTTF("等级:", "fonts/simyou.ttf", 20);
	levelLabel->setTextColor(Color4B::BLACK);
	levelLabel->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + visibleSize.height / 2 + 15));
	this->addChild(levelLabel, 1);
	levelBox = ui::TextField::create("输入等级……", "fonts/simyou.ttf1", 15);
	levelBox->setColor(Color3B::BLACK);
	levelBox->setMaxLength(2);
	levelBox->setMaxLengthEnabled(true);
	levelBox->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + visibleSize.height / 2 - 15));
	this->addChild(levelBox, 5);
	auto levelBoxbg = Sprite::create("transparent.png");
	levelBoxbg->setScale(levelBox->getContentSize().width / levelBoxbg->getContentSize().width, levelBox->getContentSize().height / levelBoxbg->getContentSize().height);
	levelBoxbg->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + visibleSize.height / 2 - 15));
	this->addChild(levelBoxbg, 4);

	auto maskListener = EventListenerTouchOneByOne::create();
	maskListener->onTouchBegan = [=](Touch *touch, Event *event) {
		maskListener->setSwallowTouches(true);
		return true;
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(maskListener, this);
	return bl;
}