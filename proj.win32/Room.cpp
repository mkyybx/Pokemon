#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include "Room.h"
#include "RoomList.h"
#include "BattleScene.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "popLayer.h"
#include "SelectAILayer.h"

USING_NS_CC;
extern bool pipeCanReadRoom;
extern bool pipeCanReadRoomList;
extern bool pipeCanReadBattleScene;
extern std::queue<void*> pipe;
extern int mysend(const char *buf, int len, int flags);
extern map<int, Pokemon&>* pokemon;
extern char userName[20];
extern vector<pair<string, string>> initPokemonMap;
extern int rednum;
extern int bluenum;
extern vector<fightingPokemon*> fp;
extern int mynum;
extern bool isTimerStarted;
listroom thisroom;
ui::Button* aiButtonRed;
ui::Button* teamButtonRed;
ui::Button* aiButtonBlue;
ui::Button* teamButtonBlue;
ui::Button* startButton;
ui::ScrollView* blueList;
ui::ScrollView* redList;
Sprite* myPokemon;
extern Size visibleSize;
extern Vec2 origin;

Scene* Room::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = Room::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void Room::pokemonCallBackFunc(Ref* sender) {
	ACK = true;
	if (sender != nullptr) {
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
		if (ACK) {
			nextId = ((MenuItem*)(sender))->getTag();
			char flag = CHANGE_POKEMON;
			mysend(&flag, 1, 0);
			mysend((char*)(&nextId), sizeof(int), 0);
			ACK = false;
		}
	}
	else {
		auto scaleActionOut = ScaleTo::create(1, 0.001 / myPokemon->getContentSize().height);
		myPokemon->runAction(scaleActionOut);
		this->removeChild(myPokemon);
		myPokemon = Sprite::create(initPokemonMap[pokemon->find(nextId)->second.inittype - 1].second);
		myPokemon->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 4 * 3 + 100));
		auto scaleAction = ScaleTo::create(1, 150.0 / myPokemon->getContentSize().height);
		this->addChild(myPokemon, 4);
		myPokemon->runAction(scaleAction);
		ACK = true;
	}
}

void Room::addButton(bool isBlue, room* r,int i, bool isBlueTeam) {
	if (!isBlue) {
		if (isBlueTeam) {
			teamButtonRed = ui::Button::create("button.png");
			teamButtonRed->setScale(0.8);
			teamButtonRed->setTitleText("加入");
			teamButtonRed->setTitleFontName("fonts/msyh.ttc");
			teamButtonRed->setTitleFontSize(25);
			teamButtonRed->setTitleColor(Color3B::BLACK);
			teamButtonRed->setPosition(Vec2(redList->getContentSize().width / 2 - 100, (redList->getContentSize().height * (6 - i) / 6.0 - 50)));
			teamButtonRed->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
				switch (type) {
				case ui::Widget::TouchEventType::BEGAN:
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
					break;
				case ui::Widget::TouchEventType::ENDED:
				{
														  char flag = CHANGE_TEAM;
														  mysend(&flag, 1, 0);
				}
					break;
				default:
					break;
				}
			});
			if (i < 5)
				redList->addChild(teamButtonRed);
		}
		//电脑按钮
		aiButtonRed = ui::Button::create("button.png");
		aiButtonRed->setScale(0.8);
		aiButtonRed->setTitleText("添加电脑");
		aiButtonRed->setTitleFontName("fonts/msyh.ttc");
		aiButtonRed->setTitleFontSize(25);
		aiButtonRed->setTitleColor(Color3B::BLACK);
		aiButtonRed->setPosition(Vec2(redList->getContentSize().width / 2 + 100, (redList->getContentSize().height * (6 - i) / 6.0 - 50)));
		aiButtonRed->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  auto popup = SelectAILayer::create();
													  popup->setIsBlue(false);
													  this->addChild(popup, 20);
			}
				break;
			default:
				break;
			}
		});
		if (r != nullptr && string(userName) != r->owner)
			aiButtonRed->setVisible(false);
		if (i < 5)
			redList->addChild(aiButtonRed);
	}
	else {
		if (!isBlueTeam) {
			teamButtonBlue = ui::Button::create("button.png");
			teamButtonBlue->setScale(0.8);
			teamButtonBlue->setTitleText("加入");
			teamButtonBlue->setTitleFontName("fonts/msyh.ttc");
			teamButtonBlue->setTitleFontSize(25);
			teamButtonBlue->setTitleColor(Color3B::BLACK);
			teamButtonBlue->setPosition(Vec2(blueList->getContentSize().width / 2 - 100, (blueList->getContentSize().height * (6 - i) / 6.0 - 50)));
			teamButtonBlue->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
				switch (type) {
				case ui::Widget::TouchEventType::BEGAN:
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
					break;
				case ui::Widget::TouchEventType::ENDED:
				{
														  char flag = CHANGE_TEAM;
														  mysend(&flag, 1, 0);
				}
					break;
				default:
					break;
				}
			});
			if (i < 5)
				blueList->addChild(teamButtonBlue);
		}
		//电脑按钮
		aiButtonBlue = ui::Button::create("button.png");
		aiButtonBlue->setScale(0.8);
		aiButtonBlue->setTitleText("添加电脑");
		aiButtonBlue->setTitleFontName("fonts/msyh.ttc");
		aiButtonBlue->setTitleFontSize(25);
		aiButtonBlue->setTitleColor(Color3B::BLACK);
		aiButtonBlue->setPosition(Vec2(blueList->getContentSize().width / 2 + 100, (blueList->getContentSize().height * (6 - i) / 6.0 - 50)));
		aiButtonBlue->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  auto popup = SelectAILayer::create();
													  popup->setIsBlue(true);
													  this->addChild(popup, 20);
			}
				break;
			default:
				break;
			}
		});
		if (r != nullptr && string(userName) != r->owner)
			aiButtonBlue->setVisible(false);
		if (i < 5)
			blueList->addChild(aiButtonBlue);
		if (r != nullptr && string(userName) == r->owner)
			startButton->setVisible(true);
	}
}

void Room::setRoom(room* r) {
	bool isBlue = true;
//红队
	redList->removeAllChildren();
	int i;
	for (i = 0; i < r->reduser.size(); i++) {
		auto label = Label::createWithTTF(r->reduser[i], "fonts/simyou.ttf", 20);
		if (string(userName) == r->reduser[i]) {
			isBlue = false;
			label->setTextColor(Color4B::ORANGE);
		}
		else label->setTextColor(Color4B::WHITE);
		label->setPosition(Vec2(redList->getContentSize().width / 2, redList->getContentSize().height * (6 - i) / 6.0 - 50));
		redList->addChild(label, 10);
	}
	this->addButton(false, r, i, isBlue);
//蓝队
	blueList->removeAllChildren();
	for (i = 0; i < r->blueuser.size(); i++) {
		auto label = Label::createWithTTF(r->blueuser[i], "fonts/simyou.ttf", 20);
		if (string(userName) == r->blueuser[i]) {
			isBlue = true;
			label->setTextColor(Color4B::ORANGE);
		}
		else label->setTextColor(Color4B::WHITE);
		label->setPosition(Vec2(blueList->getContentSize().width / 2, blueList->getContentSize().height * (6 - i) / 6.0 - 50));
		blueList->addChild(label, 10);
	}
	this->addButton(true, r, i, isBlue);
	delete(r);
}
bool Room::init() {
	// 1. super init first
	if (!Layer::init()) {
		return false;
	}
	//背景
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();
	auto background = Sprite::create("pokeball.png");
	background->setScale(0.5);
	background->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	auto rotate = RotateBy::create(20, 360);
	background->runAction(RepeatForever::create(rotate));
	this->addChild(background);
	//红队
	{
		redList = ui::ScrollView::create();
		redList->setContentSize(Size(visibleSize.width / 2 - 150, visibleSize.height - 300));
		redList->setPosition(Vec2(redList->getContentSize().width / 2, visibleSize.height / 2));
		redList->setBackGroundImage("transparent.png");
		redList->setDirection(ui::ScrollView::Direction::VERTICAL);
		redList->setInnerContainerSize(redList->getContentSize());
		redList->setBounceEnabled(true);
		redList->setAnchorPoint(Vec2(0.5, 0.5));
		this->addChild(redList, 4);
		auto redListLabel = Label::createWithTTF("Red Team", "fonts/simyou.ttf", 30);
		redListLabel->setPosition(Vec2(redList->getPosition().x, redList->getPosition().y + redList->getContentSize().height / 2 + 50));
		redListLabel->setTextColor(Color4B::RED);
		this->addChild(redListLabel, 3);
		this->addButton(false, nullptr, 0, false);
	}
	//蓝队
	{
		blueList = ui::ScrollView::create();
		blueList->setContentSize(Size(visibleSize.width / 2 - 150, visibleSize.height - 300));
		blueList->setPosition(Vec2(visibleSize.width - blueList->getContentSize().width / 2, visibleSize.height / 2));
		blueList->setBackGroundImage("transparent.png");
		blueList->setDirection(ui::ScrollView::Direction::VERTICAL);
		blueList->setInnerContainerSize(blueList->getContentSize());
		blueList->setBounceEnabled(true);
		blueList->setAnchorPoint(Vec2(0.5, 0.5));
		this->addChild(blueList, 4);
		auto blueListLabel = Label::createWithTTF("Blue Team", "fonts/simyou.ttf", 30);
		blueListLabel->setPosition(Vec2(blueList->getPosition().x, blueList->getPosition().y + blueList->getContentSize().height / 2 + 50));
		blueListLabel->setTextColor(Color4B::BLUE);
		this->addChild(blueListLabel, 3);
		this->addButton(true, nullptr, 0, false);
	}
	//按钮
	{
		startButton = ui::Button::create("button.png");
		startButton->setTitleText("开始游戏");
		startButton->setTitleFontName("fonts/msyh.ttc");
		startButton->setTitleFontSize(25);
		startButton->setTitleColor(Color3B::BLACK);
		startButton->setPosition(Vec2(visibleSize.width / 2 - 100, 75));
		startButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  char flag = BATTLE_START;
													  mysend(&flag, 1, 0);
			}
				break;
			default:
				break;
			}
		});
		startButton->setVisible(false);
		this->addChild(startButton, 3);
	}
	{
		auto leaveButton = ui::Button::create("button.png");
		leaveButton->setTitleText("退出房间");
		leaveButton->setTitleFontName("fonts/msyh.ttc");
		leaveButton->setTitleFontSize(25);
		leaveButton->setTitleColor(Color3B::BLACK);
		leaveButton->setPosition(Vec2(visibleSize.width / 2 + 100, 75));
		leaveButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  char flag = LEAVE_ROOM;
													  mysend(&flag, 1, 0);
			}
				break;
			default:
				break;
			}
		});
		this->addChild(leaveButton, 3);
	}
	//自己的小精灵
	{
		auto pokemonList = ui::ScrollView::create();
		pokemonList->setBackGroundImage("transparent.png");
		pokemonList->setDirection(ui::ScrollView::Direction::VERTICAL);
		pokemonList->setContentSize(Size(visibleSize.width / 5, visibleSize.height + origin.y - 400));
		pokemonList->setInnerContainerSize(Size(visibleSize.width / 5, pokemonList->getContentSize().height < pokemon->size() * 30 + 30 ? pokemon->size() * 30 + 30 : pokemonList->getContentSize().height));
		pokemonList->setBounceEnabled(true);
		pokemonList->setAnchorPoint(Vec2(0.5, 0.5));
		pokemonList->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
		int pokemoncount = 0;
		for (map<int, Pokemon&>::iterator it = pokemon->begin(); it != pokemon->end(); it++) {
			auto tempLabel = MenuItemLabel::create(Label::createWithTTF(it->second.name, "fonts/msyhbd.ttc", 20), CC_CALLBACK_1(Room::pokemonCallBackFunc, this));
			tempLabel->setColor(Color3B::GREEN);
			tempLabel->setTag(it->first);
			auto tempMenu = Menu::create(tempLabel, NULL);
			tempMenu->setPosition(Vec2(pokemonList->getContentSize().width / 2, pokemonList->getInnerContainerSize().height - pokemoncount * 30 - 15));
			pokemoncount++;
			pokemonList->addChild(tempMenu);
		}
		this->addChild(pokemonList, 4);
		myPokemon = Sprite::create(initPokemonMap[pokemon->begin()->second.inittype - 1].second);
		myPokemon->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 4 * 3 + 100));
		auto scaleAction = ScaleTo::create(1, 150.0 / myPokemon->getContentSize().height);
		this->addChild(myPokemon, 4);
		myPokemon->runAction(scaleAction);
	}
	this->scheduleUpdate();
}
void Room::update(float) {
	static char state = -1;
	if (pipe.size() != 0 && pipeCanReadRoom) {
		if (state == -1) {
			unsigned int c = (unsigned)(pipe.front());
			pipe.pop();
			if (c == NETWORK_ERROR) {
				auto poplayerNetWorkError = PopLayer::create();
				poplayerNetWorkError->setParameters(true, "网络连接被中断", true);
				this->addChild(poplayerNetWorkError, 99);
			}
			else if (c == LEAVE_ROOM_ACK) {
				pipeCanReadRoomList = true;
				pipeCanReadRoom = false;
				Director::getInstance()->replaceScene(TransitionFade::create(1.5, RoomList::createScene()));
				state = -1;
			}
			else if (c == CHANGE_POKMON_ACK) {
				this->pokemonCallBackFunc(nullptr);
			}
			else if (c == BATTLE_START) {
				//下一个场景
				if (pipe.size() >= 4) {
					isTimerStarted = true;
					char num = (char)(pipe.front());
					pipe.pop();
					rednum = (char)(pipe.front());
					pipe.pop();
					bluenum = num - rednum;
					auto fp1 = (vector<fightingPokemon*>*)(pipe.front());
					fp = *fp1;
					pipe.pop();
					mynum = (char)(pipe.front());
					pipe.pop();
					state = -1;
				}
				pipeCanReadRoom = false;
				pipeCanReadBattleScene = true;
				Director::getInstance()->replaceScene(TransitionFade::create(1.5, BattleScene::createScene()));
				
			}
			else if (c == OWNER_CHANGED) {
				aiButtonBlue->setVisible(true);
				aiButtonRed->setVisible(true);
				startButton->setVisible(true);
			}
			else if (c == TEAM_NOT_EQUAL) {
				auto poplayer = PopLayer::create();
				poplayer->setParameters(true, "决斗模式双方队伍\n人数必须相等哦……", false);
				this->addChild(poplayer, 20);
			}
			else state = c;
		}
		else if (state == ROOM_CHANGED) {
			if (pipe.size() >= 1) {
				auto r = (room*)(pipe.front());
				pipe.pop();
				state = -1;
				setRoom(r);
			}
		}
	}
}