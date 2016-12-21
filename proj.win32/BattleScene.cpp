#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include "BattleScene.h"
#include "SimpleAudioEngine.h"
#include "FightingPokemonLayer.h"
#include "FloatLabel.h"
#include "PokemonDetails.h"
#include "ui/CocosGUI.h"
#include "proj.win32\head.h"
#include "proj.win32\popLayer.h"
#include "RoomList.h"
#include <cmath>

USING_NS_CC;

extern std::queue<void*> pipe;
extern int mysend(const char *buf, int len, int flags);
extern bool pipeCanReadBattleScene;
extern bool pipeCanReadRoomList;
extern char userName[20];
extern vector<pair<string, string>> initPokemonMap;
extern Size visibleSize;
ui::LoadingBar* hpbar;
ui::LoadingBar* mpbar;
Label* hpbarLabel;
Label* mpbarLabel;
Label* timerLabel;
int rednum;
int bluenum;
vector<fightingPokemon*> fp;
FightingPokemonLayer* fplayer[10];
int mynum;
bool isTimerStarted = true;
bool isBlueWin;
ui::LoadingBar* waitingbar;

struct fireParam {
	int type;
	Vec2 firePosition;
	int damage;
	bool isCritical;
	bool isAvoided;
	FightingPokemonLayer* fplayer;
	int to;
	int from;
	bool isPhysical;
};


void BattleScene::onMouseMove(Event *event) {
	static int currentTarget = -1;
	EventMouse* e = (EventMouse*)event;
	bool found = false;
	for (int i = 0; i < rednum + bluenum; i++) {
		if (fplayer[i]->getBoundingBox().containsPoint(Director::getInstance()->convertToUI(e->getLocation()))) {
			found = true;
			if (i != currentTarget) {
				currentTarget == -1 ? 0 : fplayer[currentTarget]->playMoveOnAnimation(false);
				fplayer[i]->playMoveOnAnimation(true);
				currentTarget = i;
			}
			break;
		}
	}
	if (!found) {
		currentTarget == -1 ? 0 : fplayer[currentTarget]->playMoveOnAnimation(false);
		currentTarget = -1;
	}
}

void BattleScene::onMouseUp(Event *event) {
	if (isTimerStarted) {
		EventMouse* e = (EventMouse*)event;
		//判定目标
		int target = -1;
		for (int i = 0; i < rednum + bluenum; i++) {
			if (fplayer[i]->getBoundingBox().containsPoint(Director::getInstance()->convertToUI(e->getLocation())))
				target = i;
		}
		//目标不是空
		if (target != -1) {
			if (e->getMouseButton() == 0) {
				//左键普通攻击
				if (mynum < rednum ? target >= rednum : target < rednum) {
					char flag = ATTACK;
					char to = target;
					mysend(&flag, 1, 0);
					mysend(&to, 1, 0);
					isTimerStarted = false;
				}
				else {
					auto floatlabel = FloatLabel::create();
					floatlabel->setParameters("目标选择的不对哦~", 1, Color4B::WHITE, 36);
					floatlabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 200));
					this->addChild(floatlabel, 4);
				}
			}
			else {
				//技能攻击
				CCLOG("right");
			}
		}

	}
}

void BattleScene::die(Node*, void*) {
	bool hasDead = false;
	for (int i = 0; i < fp.size(); i++) {
		if (fp[i]->hp == 0 && fplayer[i]->isVisible()) {
			hasDead = true;
			fplayer[i]->exit();
		}
	}
	char flag = ANIMATION_COMPLETE;
	mysend(&flag, 1, 0);
}

void BattleScene::syncPokemon(cocos2d::Node* sender, void* param) {
	for (int i = 0; i < fp.size(); i++) {
		fplayer[i]->setHPbar(100.0 * fp[i]->hp / fp[i]->maxHP, false);
		if (i == mynum) {
			hpbar->setPercent(100.0 * fp[i]->hp / fp[i]->maxHP);
			ostringstream os;
			os << fp[mynum]->hp << "/" << fp[mynum]->maxHP;
			hpbarLabel->setString(os.str().c_str());
		}
	}
}

void BattleScene::setFire(cocos2d::Node* sender, void* param) {
	auto fireparam = (fireParam*)param;
//	Sleep(200);
	if (!fireparam->isAvoided) {
		ParticleSystem* ps;
		switch (fireparam->type) {
		case 0:
			ps = ParticleFire::create();
			break;
		case 1:
			ps = ParticleSmoke::create();
			break;
		case 2:
			ps = ParticleExplosion::create();
			break;
		case 3:
			ps = ParticleFlower::create();
			break;
		default:
			break;
		}
		ps->setDuration(0.5);
		ps->setPosition(fireparam->firePosition);
		ps->setAutoRemoveOnFinish(true);
		this->addChild(ps, 3);
	}
	auto floatNum = FloatLabel::create();
	ostringstream os;
	os << fireparam->damage;
	if (!fireparam->isAvoided) {
		floatNum->setParameters(os.str().c_str(), 3, fireparam->isCritical ? Color4B::RED : (fireparam->isPhysical ? Color4B::ORANGE : Color4B::BLUE), fireparam->isCritical ? 36 : 20);
		floatNum->setPosition(fireparam->firePosition.x, fireparam->firePosition.y - 120);
		this->addChild(floatNum, 5);
		fireparam->fplayer->setHPbar(-100.0 * fireparam->damage / fp[fireparam->to]->maxHP, true);
	}
	else fireparam->fplayer->startFlash();
	free(fireparam);
//	Sleep(100);
	if (fireparam->to == mynum) {
		hpbar->setPercent(hpbar->getPercent() - 100.0 * fireparam->damage / fp[mynum]->maxHP);
		ostringstream os;
		os << fp[mynum]->maxHP * hpbar->getPercent() / 100.0 << "/" << fp[mynum]->maxHP;
		hpbarLabel->setString(os.str().c_str());
	}
	if (fireparam->from == mynum) {
		mpbar->setPercent(100.0 * fp[mynum]->mp / fp[mynum]->maxMP);
		ostringstream os;
		os << fp[mynum]->mp << "/" << fp[mynum]->maxMP;
		mpbarLabel->setString(os.str().c_str());
	}
}

void BattleScene::applyAttackData(vector<Attack> att) {
	cocos2d::Vector<FiniteTimeAction*> finalaction;
	for (auto it = att.begin(); it != att.end(); it++) {
		Vec2 fromOriginalPos = fplayer[it->fromseq]->getPosition();
		Vec2 toOriginalPos = fplayer[it->toseq]->getPosition();
		float yvsx = (fplayer[it->toseq]->getPosition().y - fplayer[it->fromseq]->getPosition().y) /
			(fplayer[it->toseq]->getPosition().x - fplayer[it->fromseq]->getPosition().x);
		auto moveaction = TargetedAction::create(fplayer[it->fromseq], MoveTo::create(0.5, fplayer[it->toseq]->getPosition()));
		//呼叫函数
		int hundred = fplayer[it->fromseq]->getPosition().x < fplayer[it->toseq]->getPosition().x ? 100 : -100;
		auto killback = TargetedAction::create(fplayer[it->toseq], MoveBy::create(0.5, Vec2(hundred, hundred * yvsx)));
		auto killbackreturn = TargetedAction::create(fplayer[it->toseq], MoveTo::create(0.5, toOriginalPos));

		auto moveactionreturn = TargetedAction::create(fplayer[it->fromseq], MoveTo::create(0.5, fromOriginalPos));
		fireParam* fparam = new fireParam();
		fparam->damage = it->damage;
		fparam->firePosition = fplayer[it->toseq]->getPosition();
		fparam->fplayer = fplayer[it->toseq];
		fparam->isAvoided = it->isAvoided;
		fparam->isCritical = it->isCritical;
		fparam->type = ((Pokemon*)fp[it->fromseq])->clientAttack();
		fparam->to = it->toseq;
		fparam->from = it->fromseq;
		fparam->isPhysical = it->isPhysical;
		Sequence* seq;
		if (!it->isAvoided)
			seq = Sequence::create(moveaction, CCCallFuncND::create(this, callfuncND_selector(BattleScene::setFire), (void*)fparam), killback, killbackreturn, moveactionreturn, nullptr);
		else seq = Sequence::create(moveaction, CCCallFuncND::create(this, callfuncND_selector(BattleScene::setFire), (void*)fparam), killback, killbackreturn, moveactionreturn, nullptr);
		finalaction.pushBack(seq);
	}
	finalaction.pushBack(CCCallFuncND::create(this, callfuncND_selector(BattleScene::syncPokemon), nullptr));
	finalaction.pushBack(CCCallFuncND::create(this, callfuncND_selector(BattleScene::die), nullptr));
	this->runAction(Sequence::create(finalaction));

	
}

Scene* BattleScene::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = BattleScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void BattleScene::timer(float) {
	if (!isTimerStarted)
		timerLabel->setVisible(false);
	if (isTimerStarted) {
		timerLabel->setVisible(true);
		ostringstream os;
		os << time;
		timerLabel->setString(os.str().c_str());
		time--;
		if (time == -1) {
			isTimerStarted = false;
			time = 30;
			timerLabel->setVisible(false);
		}
	}
}

bool BattleScene::init() {
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	isTimerStarted = true;
	this->scheduleUpdate();
	this->schedule(schedule_selector(BattleScene::timer), 1.0f, kRepeatForever, 0);
	while (mynum == -1);//等待基础信息
	visibleSize = Director::getInstance()->getVisibleSize();
	auto bg = Sprite::create("battleBackground.jpg");
	//背景
	{
		bg->setScale(1.3);
		bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
		auto bgmask = Sprite::create("transparentBlack.png");
		bgmask->setPosition(visibleSize.width / 2, visibleSize.height / 2);
		this->addChild(bg, 0);
		this->addChild(bgmask, 1);
	}
	//监听器
	auto _mouseListener = EventListenerMouse::create();
	_mouseListener->onMouseMove = CC_CALLBACK_1(BattleScene::onMouseMove, this);
	_mouseListener->onMouseUp = CC_CALLBACK_1(BattleScene::onMouseUp, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);
	//攻击，技能按钮
	{
		
	}
	//头像，血条
	{
		ostringstream os;
		os << fp[mynum]->maxHP << "/" << fp[mynum]->maxHP;
		hpbarLabel = Label::createWithTTF(os.str().c_str(), "fonts/simyou.ttf", 20);
		ostringstream os1;
		os1 << fp[mynum]->maxMP << "/" << fp[mynum]->maxMP;
		mpbarLabel = Label::createWithTTF(os1.str().c_str(), "fonts/simyou.ttf", 20);
		auto nameLabel = Label::createWithTTF(userName, "fonts/simyou.ttf", 20);
		auto frame = Sprite::create("frame.png");
		auto head = Sprite::create(initPokemonMap[fp[mynum]->inittype - 1].second);
		hpbar = ui::LoadingBar::create("HP.png");
		hpbar->setDirection(ui::LoadingBar::Direction::LEFT);
		hpbar->setPercent(100);
		mpbar = ui::LoadingBar::create("MP.png");
		mpbar->setDirection(ui::LoadingBar::Direction::LEFT);
		mpbar->setPercent(100);
		head->setScale(100.0 / head->getContentSize().width);
		frame->setScale(115.0 / frame->getContentSize().width);
		hpbar->setScale(200.0 / hpbar->getContentSize().width, 40.0 / hpbar->getContentSize().height);
		mpbar->setScale(200.0 / mpbar->getContentSize().width, 40.0 / mpbar->getContentSize().height);
		head->setPosition(Vec2(visibleSize.width - 300, 100));
		frame->setPosition(Vec2(visibleSize.width - 300, 100));
		hpbar->setPosition(Vec2(head->getPosition().x + 100 + 115.0 / 2, 120));
		hpbarLabel->setPosition(hpbar->getPosition());
		mpbar->setPosition(Vec2(head->getPosition().x + 100 + 115.0 / 2, 80));
		mpbarLabel->setPosition(mpbar->getPosition());
		nameLabel->setTextColor(Color4B::BLACK);
		nameLabel->setPosition(hpbar->getPosition().x, hpbar->getPosition().y + 30);
		this->addChild(frame, 1);
		this->addChild(hpbar, 1);
		this->addChild(hpbarLabel, 3);
		this->addChild(mpbar, 1);
		this->addChild(mpbarLabel, 3);
		this->addChild(head, 2);
		this->addChild(nameLabel, 3);
	}
	//timer
	{
		timerLabel = Label::createWithTTF("", "fonts/simyou.ttf", 70);
		timerLabel->enableBold();
		timerLabel->enableOutline(Color4B::BLUE, 1);
		timerLabel->setTextColor(Color4B::WHITE);
		timerLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 70));
		this->addChild(timerLabel, 2);
	}
	//小精灵
	{
		int firstnum;
		int secondnum;
		if (mynum >= rednum) {
			//自己在蓝方
			firstnum = rednum;
			secondnum = 0;
		}
		else {
			//自己在红方
			firstnum = 0;
			secondnum = rednum;
		}
		//下方
		for (int i = firstnum; (i < secondnum && firstnum == 0) || (i < rednum + bluenum && firstnum != 0); i++) {
			auto fpl = FightingPokemonLayer::create();
			fpl->setInitParameters(fp[i]->name, fp[i]->inittype, false);
			fpl->setPosition(Vec2((visibleSize.width - 300) + pow(-1.0, i - firstnum) * (i + 1 - firstnum) / 2 * 100, 300 + pow(-1.0, i - firstnum) * (i + 1 - firstnum) / 2 * 50));
			fpl->setTag(i);
			this->addChild(fpl, 1);
			fplayer[i] = fpl;
		}
		//上方
		for (int i = secondnum; (i < firstnum && firstnum != 0)|| (i < rednum + bluenum && firstnum == 0); i++) {
			auto fpl = FightingPokemonLayer::create();
			fpl->setInitParameters(fp[i]->name, fp[i]->inittype, true);
			fpl->setPosition(Vec2(300 + pow(-1.0, i - secondnum) * (i + 1 - secondnum) / 2 * 100, visibleSize.height - 300 + pow(-1.0, i - secondnum) * (i + 1 - secondnum) / 2 * 50));
			fpl->setTag(i);
			this->addChild(fpl, 1);
			fplayer[i] = fpl;
		}
	}

	auto ps = ParticleGalaxy::create();
	ps->setColor(Color3B::BLUE);
	ps->setAutoRemoveOnFinish(true);
	ps->setDuration(3);
	ps->setPosition(500, 500);
	this->addChild(ps, 100);
	
	
	return true;
}

void BattleScene::playWaingBar(float) {
	if (waitingbar->getPercent() >= 1)
		waitingbar->setPercent(waitingbar->getPercent() - 1);
}

void BattleScene::update(float) {
	if (!isTimerStarted)
		time = 30;
	static char state = -1;
	//切换场景记得改回-1
	if (pipe.size() != 0 && pipeCanReadBattleScene) {
		if (state == -1) {
			unsigned int c = (unsigned)(pipe.front());
			pipe.pop();
			if (c == NETWORK_ERROR) {
				auto poplayerNetWorkError = PopLayer::create();
				poplayerNetWorkError->setParameters(true, "网络连接被中断", true);
				this->addChild(poplayerNetWorkError, 99);
			}
			else if (c == ANIMATION_COMPLETE)
				isTimerStarted = true;
			else if (c == SYNC_POKEMON) {
				state = -1;
				pipeCanReadBattleScene = false;
				pipeCanReadRoomList = true;
				Director::getInstance()->replaceScene(TransitionFade::create(1.5, RoomList::createScene()));
			}
			else state = c;
		}
		else if (state == ATTACK_DATA) {
			if (pipe.size() >= 1) {
				isTimerStarted = false;
				auto att1 = (vector<Attack>*)(pipe.front());
				this->applyAttackData(*att1);
				pipe.pop();
				state = -1;
			}
		}
		else if (state == BATTLE_FINISH) {
			if (pipe.size() >= 2) {
				isBlueWin = ((char)(pipe.front()) == 1 ? true : false);
				pipe.pop();
				state = -1;
				if ((char)(pipe.front()) == BATTLE_FINISH) {
					pipe.pop();
					auto pop = PopLayer::create();
					pipeCanReadBattleScene = false;
					pipeCanReadRoomList = true;
					if (!(isBlueWin ^ (mynum >= rednum))) {
						pop->setParameters(true, "恭喜，你赢啦！", false, true);
						this->addChild(pop, 30);
						for (int i = 0; i < rand() % 50; i++) {
							auto ps = ParticleFireworks::createWithTotalParticles(rand() % 500);
							ps->setDuration(rand() % 5);
							ps->setPosition(Vec2(rand() % (int)visibleSize.width, rand() % (int)visibleSize.height));
							this->addChild(ps, 31);
						}
					}
					else {
						pop->setParameters(true, "很遗憾，你输了……", false, true);
						this->addChild(pop, 30);
						for (int i = 0; i < rand() % 50; i++) {
							auto ps = ParticleSnow::createWithTotalParticles(rand() % 500);
							ps->setDuration(rand() % 5);
							ps->setPosition(Vec2(rand() % (int)visibleSize.width, rand() % (int)visibleSize.height));
							this->addChild(ps, 31);
						}
					}
				}
				else if ((char)(pipe.front()) == EXCHANGE_POKEMON){
					pipe.pop();
					waitingbar = ui::LoadingBar::create("loadingbar.png");
					if (!(isBlueWin ^ (mynum >= rednum))) {
						//赢了
						waitingbar->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 150));
						auto pop = PopLayer::create();
						pop->setParameters(false, "     恭喜，你赢啦！     \n请等待其他玩家送出小精灵", false);
						this->addChild(pop, 30);
						for (int i = 0; i < rand() % 50; i++) {
							auto ps = ParticleFireworks::createWithTotalParticles(rand() % 500);
							ps->setDuration(rand() % 5);
							ps->setPosition(Vec2(rand() % (int)visibleSize.width, rand() % (int)visibleSize.height));
							this->addChild(ps, 31);
						}
					}
					else {
						//输了
						waitingbar->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 427 / 2));
						auto detailLayer = PokemonDetailsLayer::create();
						detailLayer->setPokemon(nullptr, nullptr, "", true);
						this->addChild(detailLayer, 10);
						for (int i = 0; i < rand() % 50; i++) {
							auto ps = ParticleSnow::createWithTotalParticles(rand() % 500);
							ps->setDuration(rand() % 5);
							ps->setPosition(Vec2(rand() % (int)visibleSize.width, rand() % (int)visibleSize.height));
							this->addChild(ps, 71);
						}
					}
					//等待条
					waitingbar->setScale(350.0 / waitingbar->getContentSize().width);
					waitingbar->setPercent(100);
					this->addChild(waitingbar, 1000);
					this->schedule(schedule_selector(BattleScene::playWaingBar), 30 / 100.0, 100, 0);
				}
			}
		}
	}
}