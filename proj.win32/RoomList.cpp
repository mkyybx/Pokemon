#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include "RoomList.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "PokemonDetails.h"
#include "popLayer.h"
#include "Room.h"

USING_NS_CC;
extern bool pipeCanReadRoomList;
extern bool pipeCanReadRoom;
extern std::queue<void*> pipe;
extern char userName[20];
extern int mysend(const char *buf, int len, int flags);
map<int, Pokemon&>* pokemon = NULL;
cocos2d::ui::ScrollView* roomList;
cocos2d::ui::ScrollView* playerList;
RoomList* scene;
PopLayer* entering;

void RoomList::pokemonCallBackFunc(Ref* sender) {
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
	auto detailLayer = PokemonDetailsLayer::create();
	detailLayer->setPokemon(&(pokemon->find(((MenuItem*)(sender))->getTag())->second));
	scene->addChild(detailLayer, 15);
}

void RoomList::roomListCallBackFunc(Ref* sender) {
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
	entering = PopLayer::create();
	entering->setParameters(false, "正在进入房间……", false);
	this->addChild(entering, 15);
	char flag = ENTER_ROOM;
	mysend(&flag, 1, 0);
	int id = ((MenuItem*)(sender))->getTag();
	mysend((const char*)&id, sizeof(int), 0);
}

void RoomList::playerListCallBackFunc(Ref* sender) {
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
	char flag = QUERY_POKEMON;
	mysend(&flag, 1, 0);
	string s = ((MenuItem*)(sender))->getName();
	mysend(s.c_str(), s.length(), 0);
}

void RoomList::setRoomList(vector<listroom>* tempv) {
	int roomcount = 0;
	roomList->removeAllChildren();
	roomList->setInnerContainerSize(Size(roomList->getInnerContainerSize().width, roomList->getContentSize().height < tempv->size() * 30 + 30 ? tempv->size() * 30 + 30 : roomList->getContentSize().height));
	for (auto it = tempv->begin(); it != tempv->end(); it++) {
		ostringstream os;
		string s(it->owner);
		os << it->id << '\t' << s << "的房间";
		if (it->isDuel)
			os << "(决斗模式)";
		if (it->isStarted)
			os << "\t游戏中";
		auto tempLabel = MenuItemLabel::create(Label::createWithTTF(os.str(), "fonts/SIMYOU.ttf1", 20), CC_CALLBACK_1(RoomList::roomListCallBackFunc, this));
		(!it->isStarted) ? (it->isDuel ? tempLabel->setColor(Color3B::RED) : tempLabel->setColor(Color3B::ORANGE)) : tempLabel->setColor(Color3B::GRAY);
		tempLabel->setTag(it->id);
		auto tempMenu = Menu::create(tempLabel, NULL);
		tempMenu->setPosition(Vec2(roomList->getInnerContainerSize().width / 2, roomList->getInnerContainerSize().height - roomcount * 30 - 15));
		roomcount++;
		roomList->addChild(tempMenu);
	}
}

void RoomList::setPlayerList(vector<string>* tempv) {
	int playercount = 0;
	playerList->removeAllChildren();
	playerList->setInnerContainerSize(Size(playerList->getInnerContainerSize().width, playerList->getContentSize().height < tempv->size() * 30 + 30 ? tempv->size() * 30 + 30 : playerList->getContentSize().height));
	for (auto it = tempv->begin(); it != tempv->end(); it++) {
		auto tempLabel = MenuItemLabel::create(Label::createWithTTF(*it, "fonts/msyhbd.ttc", 20), CC_CALLBACK_1(RoomList::playerListCallBackFunc, this));
		tempLabel->setColor(Color3B::ORANGE);
		tempLabel->setName(*it);
		auto tempMenu = Menu::create(tempLabel, NULL);
		tempMenu->setPosition(Vec2(playerList->getInnerContainerSize().width / 2, playerList->getInnerContainerSize().height - playercount * 30 - 15));
		playercount++;
		playerList->addChild(tempMenu);
	}
}

Scene* RoomList::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = RoomList::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void RoomList::drawBadgeAndPokemon() {
	if (!hasDrawed) {
		auto visibleSize = Director::getInstance()->getVisibleSize();
		Vec2 origin = Director::getInstance()->getVisibleOrigin();
		auto myHornor = Label::createWithTTF("我的荣誉", "fonts/simyou.ttf", 20);
		myHornor->setTextColor(Color4B::WHITE);
		myHornor->setPosition(Vec2(visibleSize.width / 5 * 4 + origin.x, visibleSize.height + origin.y - 100));
		this->addChild(myHornor, 3);
		int pokemonnum = pokemon->size();
		int highnum = 0;

		//小精灵列表
		auto pokemonListLabel = Label::createWithTTF("我的小精灵", "fonts/simyou.ttf", 20);
		pokemonListLabel->setPosition(Vec2(visibleSize.width / 5 * 4 + origin.x, visibleSize.height + origin.y - 300));
		this->addChild(pokemonListLabel, 3);
		auto pokemonList = ui::ScrollView::create();
		pokemonList->setBackGroundImage("transparent.png");
		pokemonList->setDirection(ui::ScrollView::Direction::VERTICAL);
		pokemonList->setContentSize(Size(visibleSize.width / 5, visibleSize.height + origin.y - 400));
		pokemonList->setInnerContainerSize(Size(visibleSize.width / 5, pokemonList->getContentSize().height < pokemon->size() * 30 + 30 ? pokemon->size() * 30 + 30 : pokemonList->getContentSize().height));
		pokemonList->setBounceEnabled(true);
		pokemonList->setAnchorPoint(Vec2(0.5, 1));
		pokemonList->setPosition(Vec2(visibleSize.width / 5 * 4 + origin.x, visibleSize.height + origin.y - 350));
		int pokemoncount = 0;
		for (map<int, Pokemon&>::iterator it = pokemon->begin(); it != pokemon->end(); it++) {
			if (it->second.level == MAX_LEVEL)
				highnum++;
			auto tempLabel = MenuItemLabel::create(Label::createWithTTF(it->second.name, "fonts/msyhbd.ttc", 20), CC_CALLBACK_1(RoomList::pokemonCallBackFunc, this));
			tempLabel->setColor(Color3B::ORANGE);
			tempLabel->setTag(it->first);
			auto tempMenu = Menu::create(tempLabel, NULL);
			tempMenu->setPosition(Vec2(pokemonList->getContentSize().width / 2, pokemonList->getInnerContainerSize().height - pokemoncount * 30 - 15));
			pokemoncount++;
			pokemonList->addChild(tempMenu);
		}
		this->addChild(pokemonList, 4);

		Sprite* lowBadge;
		Label* lowBageLabel;
		Sprite* highBadge;
		Label* highBageLabel;
		if (pokemonnum <= 4) {
			lowBadge = Sprite::create("lowBronze.png");
			lowBageLabel = Label::createWithTTF("黄铜", "fonts/simyou.ttf", 15);
		}
		else if (pokemonnum <= 7) {
			lowBadge = Sprite::create("lowSilver.png");
			lowBageLabel = Label::createWithTTF("白银", "fonts/simyou.ttf", 15);
		}
		else {
			lowBadge = Sprite::create("lowGold.png");
			lowBageLabel = Label::createWithTTF("黄金", "fonts/simyou.ttf", 15);
		}
		if (highnum <= 2) {
			highBadge = Sprite::create("highBronze.png");
			highBageLabel = Label::createWithTTF("高级黄铜", "fonts/simyou.ttf", 15);
		}
		else if (highnum <= 4) {
			highBadge = Sprite::create("highSilver.png");
			highBageLabel = Label::createWithTTF("高级白银", "fonts/simyou.ttf", 15);
		}
		else {
			highBadge = Sprite::create("highGold.png");
			highBageLabel = Label::createWithTTF("高级黄金", "fonts/simyou.ttf", 15);
		}
		lowBadge->setScale(0.64);
		highBadge->setScale(0.64);
		lowBadge->setPosition(Vec2(visibleSize.width / 5 * 4 - 65 + origin.x, visibleSize.height + origin.y - 160));
		lowBageLabel->setPosition(Vec2(visibleSize.width / 5 * 4 - 65 + origin.x, visibleSize.height + origin.y - 160 - lowBadge->getContentSize().height * 0.64 / 2 - 10));
		highBadge->setPosition(Vec2(visibleSize.width / 5 * 4 + 65 + origin.x, visibleSize.height + origin.y - 160));
		highBageLabel->setPosition(Vec2(visibleSize.width / 5 * 4 + 65 + origin.x, visibleSize.height + origin.y - 160 - highBadge->getContentSize().height * 0.64 / 2 - 10));
		this->addChild(lowBadge, 1);
		this->addChild(highBadge, 1);
		this->addChild(lowBageLabel, 1);
		this->addChild(highBageLabel, 1);
		hasDrawed = true;
	}
}

void RoomList::update(float) {
	static char state = -1;
	if (pipe.size() != 0 && pipeCanReadRoomList) {
		if (state == -1) {
			unsigned int c = (unsigned)(pipe.front());
			pipe.pop();
			if (c == NETWORK_ERROR) {
				auto poplayerNetWorkError = PopLayer::create();
				poplayerNetWorkError->setParameters(true, "网络连接被中断", true);
				this->addChild(poplayerNetWorkError, 99);
			}
			else if (c == NO_SUCH_USER) {
				auto popLayer = PopLayer::create();
				popLayer->setParameters(true, "没有这个用户，\n是不是打错了？", false);
				this->addChild(popLayer, 30);
			}
			else if (c == ENTER_ROOM) {
				state = -1;
				pipeCanReadRoomList = false;
				pipeCanReadRoom = true;
				Director::getInstance()->replaceScene(TransitionFade::create(1.5, Room::createScene()));
				
			}
			else if (c == LEAVE_ROOM)
				entering->setParameters(true, "好像没有这个房间……\n或者是房间没地方惹", false);
			else state = c;
		}
		else if (state == SYNC_POKEMON) {
			if (pipe.size() >= 1) {
				pokemon = (map<int, Pokemon&>*)(pipe.front());
				pipe.pop();
				if ((char)pokemon != SYNC_POKEMON) {
					state = -1;
					drawBadgeAndPokemon();
				}
			}
		}
		else if (state == LIST_ROOM) {
			if (pipe.size() >= 1) {
				auto tempv = (vector<listroom>*)(pipe.front());
				pipe.pop();
				state = -1;
				setRoomList(tempv);
				delete(tempv);
			}
		}
		else if (state == QUERY_ONLINE) {
			if (pipe.size() >= 1) {
				auto tempv = (vector<string>*)(pipe.front());
				pipe.pop();
				state = -1;
				setPlayerList(tempv);
				delete(tempv);
			}
		}
		else if (state == QUERY_POKEMON) {
			if (pipe.size() >= 3) {
				auto tempv = (vector<strengthPokemon>*)(pipe.front());
				pipe.pop();
				auto name = (string*)(pipe.front());
				pipe.pop();
				auto tempi = (vector<int>*)(pipe.front());
				pipe.pop();
				state = -1;
				auto detailLayer = PokemonDetailsLayer::create();
				detailLayer->setPokemon(tempv, tempi, *name, false);
				scene->addChild(detailLayer, 15);
				delete(tempv);
				delete(name);
				delete(tempi);
			}
		}
	}
}

bool RoomList::init() {
	hasDrawed = false;
	// 1. super init first
	if (!Layer::init()) {
		return false;
	}
	//开启监听器
	scene = this;
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	//添加背景
	{
		auto bg = Sprite::create("roomlist.jpg");
		bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
		this->addChild(bg, 0);
	}
	//显示欢迎信息
	{
		string contactedUser;
		contactedUser.append("欢迎召唤师\n");
		contactedUser.append(userName);
		auto user = Label::createWithTTF(contactedUser, "fonts/simyou.ttf", 20);
		user->setPosition(Vec2(visibleSize.width / 5 * 4 + origin.x, visibleSize.height + origin.y - 50));
		auto userFrame = DrawNode::create();
		this->addChild(user, 1);
		if (pokemon != NULL)
			drawBadgeAndPokemon();
	}
	//房间列表
	{
		auto roomListLabel = Label::createWithTTF("房间列表", "fonts/simyou.ttf", 30);
		roomListLabel->setPosition(Vec2(visibleSize.width / 5 + origin.x, visibleSize.height + origin.y - 100));
		this->addChild(roomListLabel, 3);
		roomList = ui::ScrollView::create();
		roomList->setBackGroundImage("transparent.png");
		roomList->setDirection(ui::ScrollView::Direction::VERTICAL);
		roomList->setContentSize(Size(visibleSize.width / 5 * 1.5, visibleSize.height + origin.y - 300));
		roomList->setInnerContainerSize(Size(visibleSize.width / 5, 2000));
		roomList->setBounceEnabled(true);
		roomList->setAnchorPoint(Vec2(0.5, 1));
		roomList->setPosition(Vec2(visibleSize.width / 5 + origin.x, visibleSize.height + origin.y - 200));
		this->addChild(roomList, 4);
		char flag = LIST_ROOM;
		mysend(&flag, 1, 0);

		auto refreshButton = ui::Button::create("refresh.png");
		refreshButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  char flag = LIST_ROOM;
													  mysend(&flag, 1, 0);
			}
				break;
			default:
				break;
			}
		});
		refreshButton->setPosition(Vec2(roomList->getPosition().x + roomList->getContentSize().width / 2 - refreshButton->getContentSize().height - 5, roomList->getPosition().y - roomList->getContentSize().height + refreshButton->getContentSize().height + 5));
		this->addChild(refreshButton, 5);

		auto searchBox = ui::TextField::create("输入房间ID……", "fonts/simyou.ttf1", 20);
		searchBox->setColor(Color3B::BLACK);
		searchBox->setMaxLength(5);
		searchBox->setMaxLengthEnabled(true);
		searchBox->setPosition(Vec2(roomList->getPosition().x, roomList->getPosition().y + 25));
		auto searchBoxbg = Sprite::create("transparent.png");
		searchBoxbg->setScale(roomList->getContentSize().width / searchBoxbg->getContentSize().width, searchBox->getContentSize().height / searchBoxbg->getContentSize().height);
		searchBoxbg->setPosition(Vec2(roomList->getPosition().x, roomList->getPosition().y + 25));
		this->addChild(searchBoxbg, 2);
		this->addChild(searchBox, 3);

		auto goButton = ui::Button::create("go.png");
		goButton->setScale(searchBox->getContentSize().height / goButton->getContentSize().height);
		goButton->setPosition(Vec2(searchBox->getPosition().x + searchBox->getContentSize().width + goButton->getContentSize().width / 2 + 15, roomList->getPosition().y + 25));
		goButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  string s = searchBox->getString();
													  istringstream is(s);
													  int i;
													  if (!(is >> i)) {
														  auto errorLayer = PopLayer::create();
														  errorLayer->setParameters(true, "房间ID为纯数字\n    =￣ω￣=", false);
														  this->addChild(errorLayer, 15);
													  }
													  else {
														  entering = PopLayer::create();
														  entering->setParameters(false, "正在进入房间……", false);
														  this->addChild(entering, 15);
														  char flag = ENTER_ROOM;
														  mysend(&flag, 1, 0);
														  mysend((char*)&i, sizeof(int), 0);
													  }
													  //测试字母的情况
			}
				break;
			default:
				break;
			}
		});
		this->addChild(goButton, 4);
	}
	//在线玩家
	{
		auto playerListLabel = Label::createWithTTF("玩家列表", "fonts/simyou.ttf", 30);
		playerListLabel->setPosition(Vec2(visibleSize.width / 5 * 2.5 + origin.x, visibleSize.height + origin.y - 100));
		this->addChild(playerListLabel, 3);
		playerList = ui::ScrollView::create();
		playerList->setBackGroundImage("transparent.png");
		playerList->setDirection(ui::ScrollView::Direction::VERTICAL);
		playerList->setContentSize(Size(visibleSize.width / 5, visibleSize.height + origin.y - 300));
		playerList->setInnerContainerSize(Size(visibleSize.width / 5, 2000));
		playerList->setBounceEnabled(true);
		playerList->setAnchorPoint(Vec2(0.5, 1));
		playerList->setPosition(Vec2(visibleSize.width / 5 * 2.5 + origin.x, visibleSize.height + origin.y - 200));
		this->addChild(playerList, 4);
		char flag = QUERY_ONLINE;
		mysend(&flag, 1, 0);
		auto refreshButton = ui::Button::create("refresh.png");
		refreshButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  char flag = QUERY_ONLINE;
													  mysend(&flag, 1, 0);
			}
				break;
			default:
				break;
			}
		});
		refreshButton->setPosition(Vec2(playerList->getPosition().x + playerList->getContentSize().width / 2 - refreshButton->getContentSize().height - 5, playerList->getPosition().y - playerList->getContentSize().height + refreshButton->getContentSize().height + 5));
		this->addChild(refreshButton, 5);

		auto searchBox = ui::TextField::create("输入玩家ID……", "fonts/simyou.ttf1", 20);
		searchBox->setColor(Color3B::BLACK);
		searchBox->setMaxLength(20);
		searchBox->setMaxLengthEnabled(true);
		searchBox->setPosition(Vec2(playerList->getPosition().x, playerList->getPosition().y + 25));
		auto searchBoxbg = Sprite::create("transparent.png");
		searchBoxbg->setScale(playerList->getContentSize().width / searchBoxbg->getContentSize().width, searchBox->getContentSize().height / searchBoxbg->getContentSize().height);
		searchBoxbg->setPosition(Vec2(playerList->getPosition().x, playerList->getPosition().y + 25));
		this->addChild(searchBoxbg, 2);
		this->addChild(searchBox, 3);

		auto goButton = ui::Button::create("go.png");
		goButton->setScale(searchBox->getContentSize().height / goButton->getContentSize().height);
		goButton->setPosition(Vec2(searchBox->getPosition().x + searchBox->getContentSize().width - goButton->getContentSize().width / 2, playerList->getPosition().y + 25));
		goButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  string s = searchBox->getString();
													  if (s.length() > 0) {
														  char flag = QUERY_POKEMON;
														  mysend(&flag, 1, 0);
														  mysend(s.c_str(), s.size(), 0);
													  }
			}
				break;
			default:
				break;
			}
		});
		this->addChild(goButton, 4);
	}
	//按钮
	{
		auto duelButton = ui::Button::create("button.png");
		duelButton->setTitleText("创建决斗房间");
		duelButton->setTitleFontName("fonts/msyh.ttc");
		duelButton->setTitleFontSize(25);
		duelButton->setTitleColor(Color3B::BLACK);
		duelButton->setPosition(Vec2(visibleSize.width / 5, 50));
		duelButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  pipeCanReadRoomList = false;
													  char flag = CREATE_ROOM;
													  mysend(&flag, 1, 0);
													  bool isDuel = true;
													  mysend((char*)&isDuel, 1, 0);
													  pipeCanReadRoom = true;
													  Director::getInstance()->replaceScene(TransitionFade::create(1.5, Room::createScene()));
													  
			}
				break;
			default:
				break;
			}
		});
		this->addChild(duelButton, 3);
	}
	{
		auto practiceButton = ui::Button::create("button.png");
		practiceButton->setTitleText("创建练习房间");
		practiceButton->setTitleFontName("fonts/msyh.ttc");
		practiceButton->setTitleFontSize(25);
		practiceButton->setTitleColor(Color3B::BLACK);
		practiceButton->setPosition(Vec2(visibleSize.width / 5 * 2.5, 50));
		practiceButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
			switch (type) {
			case ui::Widget::TouchEventType::BEGAN:
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
				break;
			case ui::Widget::TouchEventType::ENDED:
			{
													  pipeCanReadRoomList = false;
													  char flag = CREATE_ROOM;
													  mysend(&flag, 1, 0);
													  bool ispractice = false;
													  mysend((char*)&ispractice, 1, 0);
													  pipeCanReadRoom = true;
													  Director::getInstance()->replaceScene(TransitionFade::create(1.5, Room::createScene()));
													  
			}
				break;
			default:
				break;
			}
		});
		this->addChild(practiceButton, 3);
	}
	this->scheduleUpdate();
} 