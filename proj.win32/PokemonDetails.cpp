

#include "PokemonDetails.h"
#include "SimpleAudioEngine.h"
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
extern vector<pair<string, string>> initPokemonMap;
extern int mysend(const char *buf, int len, int flags);
extern map<int, Pokemon&>* pokemon;//交换精灵专用
USING_NS_CC;
cocos2d::Size visibleSize;
cocos2d::Vec2 origin;

// 构造函数，如有对象成员，需要在此初始化
PokemonDetailsLayer::PokemonDetailsLayer() {}
// 析构
PokemonDetailsLayer::~PokemonDetailsLayer(){}

//虚函数，初始化，当执行本类的create()函数时会被自动调用
bool PokemonDetailsLayer::init(){
	bool bl = LayerColor::initWithColor(ccc4(255, 252, 255, 128));
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();
	//背景
	auto dialogBackground = Sprite::create("dialog.png");
	dialogBackground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	this->addChild(dialogBackground, 0);
	origin = Vec2(origin.x + visibleSize.width / 2 - dialogBackground->getContentSize().width / 2, origin.y + visibleSize.height / 2 - dialogBackground->getContentSize().height / 2);
	visibleSize = dialogBackground->getContentSize();
	
	//按钮
	okButton = ui::Button::create("ok.png");
	okButton->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y));
	okButton->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type){
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
	this->addChild(okButton, 50);
	//文字
	//经验，图标图形表示

	//加点没写


	nameLabel = Label::createWithTTF("empty", "fonts/simyou.ttf1", 20);
	nameLabel->setTextColor(Color4B::BLACK);
	this->addChild(nameLabel, 1);
	//经验条
	expbar = ui::LoadingBar::create("loadingbar.png");
	expbar->setDirection(ui::LoadingBar::Direction::LEFT);
	expbar->setPercent(100);
	expbar->setPosition(Vec2(origin.x + visibleSize.width / 4, visibleSize.height / 2 + origin.y - 150));
	this->addChild(expbar, 2);
	//经验条文字
	expbarLabel = Label::createWithTTF("empty", "fonts/simyou.ttf", 15);
	expbarLabel->setPosition(Vec2(origin.x + visibleSize.width / 4, visibleSize.height / 2 + origin.y - 130));
	expbarLabel->setTextColor(Color4B::BLACK);
	this->addChild(expbarLabel, 2);
	
	
	//masklayer
	auto maskListener = EventListenerTouchOneByOne::create();
	maskListener->onTouchBegan = [=](Touch *touch, Event *event) {
		maskListener->setSwallowTouches(true);
		return true;
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(maskListener, this);
	return bl;
}

string setString(Pokemon* p) {
	ostringstream os;
	os << "名字：\t\t\t\t" << p->name << "\n等级：\t\t\t\t" << p->level << "\n物理攻击力：\t" << p->attackDamage <<
		"\n魔法攻击力：\t" << p->power << "\n护甲：\t\t\t\t" << p->armor << "\n魔法抗性：\t\t" << p->mResist <<
		"\nHP上限：\t\t\t" << p->maxHP << "\nMP上限：\t\t\t" << p->maxMP << "\n暴击率：\t\t\t" << p->critical * 100 <<
		"%\n回避率：\t\t\t" << p->avoidance*100 << "%\n敏捷：\t\t\t\t" << p->agile << "\n剩余技能点：\t" << p->skillPoints;
	return os.str();
}

void PokemonDetailsLayer::setPokemon(Pokemon* p) {
	this->nameLabel->setString(setString(p));
	this->nameLabel->setPosition(Vec2(origin.x + visibleSize.width / 3 * 2, origin.y + visibleSize.height / 2));
	this->expbar->setPercent(100.00 * p->currentExp / p->targetExp);
	ostringstream os;
	os << "当前经验：" << 100.0 * p->currentExp / p->targetExp << "%";
	this->expbarLabel->setString(os.str());
	//this->pokemonSprite->setSpriteFrame(initPokemonMap[p->inittype].second.c_str());
	//SpriteFrameCache::getInstance()->addSpriteFrame("kbs", "kbs.png");
	//this->pokemonSprite->setSpriteFrame("kbs");
	//pokemon
	this->pokemonSprite = Sprite::create(initPokemonMap[p->inittype - 1].second.c_str());
	auto scaleAction = ScaleTo::create(1, 200.0 / pokemonSprite->getContentSize().width, 200.0 / pokemonSprite->getContentSize().width);
	this->pokemonSprite->setPosition(Vec2(origin.x + visibleSize.width / 4, visibleSize.height / 2 + origin.y));
	this->addChild(pokemonSprite, 1);
	this->pokemonSprite->runAction(scaleAction);
}

void PokemonDetailsLayer::setPokemon(vector<strengthPokemon>* tempv, vector<int>* tempi, string name, bool isExchangePokemon) {
	this->removeChild(nameLabel);
	this->removeChild(expbar);
	this->removeChild(expbarLabel);
	this->removeChild(pokemonSprite);
	if (isExchangePokemon)
		this->removeChild(okButton);
	auto scrollList = ui::ScrollView::create();
	scrollList->setDirection(ui::ScrollView::Direction::VERTICAL);
	scrollList->setContentSize(Size(visibleSize.width, visibleSize.height));
	scrollList->setInnerContainerSize(Size(visibleSize.width, ((isExchangePokemon ? (pokemon->size()) : (tempv->size())) + 1)* visibleSize.height));
	scrollList->setBounceEnabled(true);
	scrollList->setAnchorPoint(Vec2(0.5, 0.5));
	scrollList->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	this->addChild(scrollList, 4);

	if (!isExchangePokemon) {
		auto nameLabel = Label::createWithTTF("召唤师", "fonts/simyou.ttf", 50);
		nameLabel->setTextColor(Color4B::BLACK);
		nameLabel->setPosition(Vec2(visibleSize.width / 2, tempv->size()* visibleSize.height + visibleSize.height / 5 * 4 + 30));
		scrollList->addChild(nameLabel, 1);
		auto usernameLabel = Label::createWithTTF(name, "fonts/simyou.ttf1", 40);
		usernameLabel->setTextColor(Color4B::BLACK);
		usernameLabel->setPosition(Vec2(visibleSize.width / 2, tempv->size()* visibleSize.height + visibleSize.height / 5 * 4 - 30));
		scrollList->addChild(usernameLabel, 1);
		auto pokemonnum = tempv->size();
		int highnum = 0;
		for (auto it = tempv->begin(); it != tempv->end(); it++) {
			if (it->level == MAX_LEVEL)
				highnum++;
		}
		Sprite* lowBadge;
		Label* lowBageLabel;
		Sprite* highBadge;
		Label* highBageLabel;
		if (pokemonnum <= 4) {
			lowBadge = Sprite::create("lowBronze.png");
			lowBageLabel = Label::createWithTTF("黄铜", "fonts/simyou.ttf", 20);
		}
		else if (pokemonnum <= 7) {
			lowBadge = Sprite::create("lowSilver.png");
			lowBageLabel = Label::createWithTTF("白银", "fonts/simyou.ttf", 20);
		}
		else {
			lowBadge = Sprite::create("lowGold.png");
			lowBageLabel = Label::createWithTTF("黄金", "fonts/simyou.ttf", 20);
		}
		if (highnum <= 2) {
			highBadge = Sprite::create("highBronze.png");
			highBageLabel = Label::createWithTTF("高级黄铜", "fonts/simyou.ttf", 20);
		}
		else if (highnum <= 4) {
			highBadge = Sprite::create("highSilver.png");
			highBageLabel = Label::createWithTTF("高级白银", "fonts/simyou.ttf", 20);
		}
		else {
			highBadge = Sprite::create("highGold.png");
			highBageLabel = Label::createWithTTF("高级黄金", "fonts/simyou.ttf", 20);
		}
		auto lowScaleAction = ScaleTo::create(1, 100.0 / lowBadge->getContentSize().height, 100.0 / lowBadge->getContentSize().height);
		lowBadge->setPosition(Vec2(visibleSize.width / 2 - 90, tempv->size()* visibleSize.height + visibleSize.height / 2 + 25));
		lowBageLabel->setPosition(Vec2(visibleSize.width / 2 - 90, tempv->size()* visibleSize.height + visibleSize.height / 2 - 45));
		lowBageLabel->setTextColor(Color4B::BLACK);
		lowBadge->runAction(lowScaleAction);
		auto highScaleAction = ScaleTo::create(1, 100.0 / highBadge->getContentSize().height, 100.0 / highBadge->getContentSize().height);
		highBadge->setPosition(Vec2(visibleSize.width / 2 + 90, tempv->size()* visibleSize.height + visibleSize.height / 2 + 25));
		highBageLabel->setPosition(Vec2(visibleSize.width / 2 + 90, tempv->size()* visibleSize.height + visibleSize.height / 2 - 45));
		highBageLabel->setTextColor(Color4B::BLACK);
		highBadge->runAction(highScaleAction);
		scrollList->addChild(lowBadge, 1);
		scrollList->addChild(highBadge, 1);
		scrollList->addChild(lowBageLabel, 1);
		scrollList->addChild(highBageLabel, 1);

		//胜率条
		auto expbar = ui::LoadingBar::create("loadingbar.png");
		expbar->setScale(visibleSize.width / 2.0 / expbar->getContentSize().width);
		expbar->setDirection(ui::LoadingBar::Direction::LEFT);
		expbar->setPercent((*tempi)[1] == 0 ? 100 : (float)((*tempi)[0]) / (float)((*tempi)[1]) * 100.0);
		expbar->setPosition(Vec2(visibleSize.width / 2, tempv->size()* visibleSize.height + visibleSize.height / 4 - 50));
		scrollList->addChild(expbar, 20);
		//胜率文字
		ostringstream os;
		os << "胜:" << (*tempi)[0] << "   " << "负:" << (*tempi)[1] - (*tempi)[0] << "   " << "总场数:" << (*tempi)[1]
			<< "\n        胜率:" << ((*tempi)[1] == 0 ? 0 : (float)((*tempi)[0]) / (float)((*tempi)[1])) * 100 << "%";
		expbarLabel = Label::createWithTTF(os.str(), "fonts/simyou.ttf", 20);
		expbarLabel->setPosition(Vec2(visibleSize.width / 2, tempv->size()* visibleSize.height + visibleSize.height / 4));
		expbarLabel->setTextColor(Color4B::BLACK);
		scrollList->addChild(expbarLabel, 20);
	}
	else {
		auto nameLabel = Label::createWithTTF("请选择要送出的小精灵", "fonts/simyou.ttf", 50);
		nameLabel->setTextColor(Color4B::BLACK);
		nameLabel->setPosition(Vec2(visibleSize.width / 2, pokemon->size()* visibleSize.height + visibleSize.height / 2 + 30));
		scrollList->addChild(nameLabel, 1);
		auto usernameLabel = Label::createWithTTF("↓向下滑动↓", "fonts/simyou.ttf1", 40);
		usernameLabel->setTextColor(Color4B::BLACK);
		usernameLabel->setPosition(Vec2(visibleSize.width / 2, pokemon->size()* visibleSize.height + visibleSize.height / 2 - 30));
		scrollList->addChild(usernameLabel, 1);
	}

	int count = 2;
	if (!isExchangePokemon) {
		for (auto it = tempv->begin(); it != tempv->end(); it++) {
			origin = Vec2(0, scrollList->getInnerContainerSize().height - visibleSize.height * count);

			auto nameLabel = Label::createWithTTF(setString(&(*it)), "fonts/simyou.ttf1", 20);
			nameLabel->setPosition(Vec2(origin.x + visibleSize.width / 3 * 2, origin.y + visibleSize.height / 2));
			nameLabel->setTextColor(Color4B::BLACK);
			scrollList->addChild(nameLabel, 1);

			auto pokemonSprite = Sprite::create(initPokemonMap[it->inittype - 1].second.c_str());
			auto scaleAction = ScaleTo::create(1, 200.0 / pokemonSprite->getContentSize().width, 200.0 / pokemonSprite->getContentSize().width);
			pokemonSprite->setPosition(Vec2(origin.x + visibleSize.width / 4, visibleSize.height / 2 + origin.y));
			scrollList->addChild(pokemonSprite, 1);
			pokemonSprite->runAction(scaleAction);
			count++;
		}
	}
	else {
		auto buttonArray = new ui::Button*[pokemon->size()]();
		int buttonArrayLength = pokemon->size();
		for (auto it = pokemon->begin(); it != pokemon->end(); it++) {
			origin = Vec2(0, scrollList->getInnerContainerSize().height - visibleSize.height * count);

			auto nameLabel = Label::createWithTTF(setString(&(it->second)), "fonts/simyou.ttf1", 20);
			nameLabel->setPosition(Vec2(origin.x + visibleSize.width / 3 * 2, origin.y + visibleSize.height / 2));
			nameLabel->setTextColor(Color4B::BLACK);
			scrollList->addChild(nameLabel, 1);

			auto pokemonSprite = Sprite::create(initPokemonMap[it->second.inittype - 1].second.c_str());
			auto scaleAction = ScaleTo::create(1, 200.0 / pokemonSprite->getContentSize().width, 200.0 / pokemonSprite->getContentSize().width);
			pokemonSprite->setPosition(Vec2(origin.x + visibleSize.width / 4, visibleSize.height / 2 + origin.y));
			scrollList->addChild(pokemonSprite, 1);
			pokemonSprite->runAction(scaleAction);
			buttonArray[count - 2] = ui::Button::create("button.png");
			auto button = buttonArray[count - 2];
			button->setTitleText("选择");
			button->setTitleFontName("fonts/msyh.ttc");
			button->setTitleFontSize(25);
			button->setTitleColor(Color3B::BLACK);
			button->setPosition(Vec2(pokemonSprite->getPosition().x, pokemonSprite->getPosition().y - 120));
			button->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
				
				switch (type) {
				case ui::Widget::TouchEventType::BEGAN:
					if (!isChosen) {
						CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
					}
					break;
				case ui::Widget::TouchEventType::ENDED:
				{
														  if (!isChosen) {
															  char flag = EXCHANGE_POKEMON;
															  mysend(&flag, 1, 0);
															  auto pit = pokemon->begin();
															  for (int i = 0; i < count - 2; i++, pit++);
															  int num = pit->first;
															  mysend((char*)&num, sizeof(int), 0);
															  //隐藏所有按钮
															  for (int i = 0; i < buttonArrayLength; i++) {
																  if (buttonArray != nullptr)
																	  buttonArray[i]->setVisible(false);
															  }
															  //替换当前按钮
															  button->setTitleText("已选择");
															  button->setTitleColor(Color3B::GRAY);
															  button->setVisible(true);
															  isChosen = true;
															  delete buttonArray;
														  }

				}
					break;
				default:
					break;
				}
			});
			scrollList->addChild(button, 5);
			count++;
		}
	}
}

