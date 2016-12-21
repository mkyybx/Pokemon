#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include "proj.win32\head.h"
#include "proj.win32\popLayer.h"
#include "proj.win32\registerLayer.h"
#include "proj.win32\RoomList.h"
#include "proj.win32\sha1.c"

USING_NS_CC;

extern std::queue<void*> pipe;
extern int myrecv(char *buf, int flags);
extern int mysend(const char *buf, int len, int flags);
extern bool pipeCanReadHelloWorld;
extern bool pipeCanReadRoomList;
char userName[20];
PopLayer* logining;
PopLayer* registering;

void HelloWorld::update(float) {
	static char state = -1;
	
	if (pipe.size() != 0 && pipeCanReadHelloWorld) {//m.lParam == networkThreadId){
		CCLOG("msg get");
		if (state == -1) {
			unsigned int c = (unsigned)(pipe.front());
			pipe.pop();
			if (c == NETWORK_ERROR) {
				auto poplayerNetWorkError = PopLayer::create();
				poplayerNetWorkError->setParameters(true, "网络连接被中断", true);
				this->addChild(poplayerNetWorkError, 99);
			}
			else if (c == INVALID_USER_OR_PASS)
				logining->setParameters(true, "用户名或密码错误\n检查是否开启了大写锁定？", false);
			else if (c == REGISTER_FAIL)
				registering->setParameters(true, "该用户名已被注册\n换个名试试？", false);
			else if (c == REGISTER_SUCCESS)
				registering->setParameters(true, "注册成功", false);
			else if (c == LOGIN_SUCCESS) {
				state = -1;
				pipeCanReadHelloWorld = false;
				pipeCanReadRoomList = true;
				Director::getInstance()->replaceScene(TransitionFade::create(1.5, RoomList::createScene()));
				//记得改NULL
			}
			else state = c;
		}
	}
}

class LoginInfo {
public:
	char user[20];
	char pass[41];
	//会复制，保证数组长度20
	void setUserPass(const char user[20], const unsigned char* pass, int length) {
		memcpy(this->user, user, 20);
		SHA1Context context;
		uint8_t Message_Digest[20];
		SHA1Reset(&context);
		SHA1Input(&context, pass, length);
		unsigned char temp[20];
		SHA1Result(&context, temp);
		for (int i = 0; i < 20; i++) {
			this->pass[2 * i] = temp[i] / 16 + 'a';
			this->pass[2 * i + 1] = temp[i] % 16 + 'a';
		}
		this->pass[40] = 0;
	}
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & user;
		ar & pass;
	}
};
//?
void regist (const char user[20], const unsigned char* pass, int length) {
	LoginInfo regist;
	regist.setUserPass(user, pass, length);
	ostringstream os;
	boost::archive::text_oarchive oa(os);
	oa << regist;
	char flag = REGISTER;
	mysend(&flag, 1, 0);
	mysend(os.str().c_str(), os.str().length(), 0);
}

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
	Vector<MenuItem*> mi;
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));
	mi.pushBack(closeItem);
    // create menu, it's an autorelease object
    auto menu = Menu::createWithArray(mi);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    auto label = Label::createWithTTF("Pokemon", "fonts/Marker Felt.ttf", 48);
	auto user = Label::createWithTTF("用户名", "fonts/simyou.ttf1", 36);
	auto pass = Label::createWithTTF("密  码", "fonts/simyou.ttf1", 36);
	user->setTextColor(Color4B::BLACK);
	pass->setTextColor(Color4B::BLACK);
    // position the label on the center of the screen
	Size windowSize = Director::getInstance()->getWinSize();
	user->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200, origin.y + visibleSize.height / 2 + 20));
	pass->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200, origin.y + visibleSize.height / 2 - 20));
    label->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height - label->getContentSize().height));

	user->setTextColor(Color4B::WHITE);
	user->enableOutline(Color4B::BLUE,1);
	pass->setTextColor(Color4B::WHITE);
	pass->enableOutline(Color4B::BLUE, 1);
	
	//输入框
	auto userTextField = ui::TextField::create("_______________", "fonts/simyou.ttf1", 48);
	auto passTextField = ui::TextField::create("_______________", "fonts/simyou.ttf1", 48);
	userTextField->setMaxLength(20);
	userTextField->setMaxLengthEnabled(true);
	passTextField->setMaxLength(20);
	passTextField->setPasswordStyleText("*");
	passTextField->setPasswordEnabled(true);
	passTextField->setMaxLengthEnabled(true);
	userTextField->setColor(Color3B::BLACK);
	passTextField->setColor(Color3B::BLACK);
	userTextField->setPosition(Vec2(origin.x + visibleSize.width / 2 + 100, origin.y + visibleSize.height / 2 + 20));
	passTextField->setPosition(Vec2(origin.x + visibleSize.width / 2 + 100, origin.y + visibleSize.height / 2 - 20));
	

	//按钮
	auto loginButton = ui::Button::create("login.png");
	auto registerButton = ui::Button::create("register.png");
	loginButton->setPosition(Vec2(origin.x + visibleSize.width / 2 - 100, origin.y + visibleSize.height / 2 - 100));
	registerButton->setPosition(Vec2(origin.x + visibleSize.width / 2 + 100, origin.y + visibleSize.height / 2 - 100));
	loginButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){ 
		switch (type) { 
		case ui::Widget::TouchEventType::BEGAN: 
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
			break; 
		case ui::Widget::TouchEventType::ENDED:
		{
												  LoginInfo login;
												  login.setUserPass(userTextField->getString().c_str(), (const unsigned char*)(passTextField->getString().c_str()), passTextField->getStringLength());
												  ostringstream os;
												  boost::archive::text_oarchive oa(os);
												  oa << login;
												  char flag = LOGIN;
												  mysend(&flag, 1, 0);
												  mysend(os.str().c_str(), os.str().length(), 0);
												  logining = PopLayer::create();
												  logining->setParameters(false, "登录中……", false);
												  this->addChild(logining, 10);
												  memcpy(userName, login.user, 20);
		}
			break; 
		default: 
			break; 
		} 
	});
	registerButton->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type){
		switch (type) {
		case ui::Widget::TouchEventType::BEGAN:
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("button.mp3");
			break;
		case ui::Widget::TouchEventType::ENDED:
		{
												  auto registerlayer = RegisterLayer::create();
												  this->addChild(registerlayer, 5);
		}
			break;
		default:
			break;
		}
	});

    // add the label as a child to this layer
	
	this->addChild(user,1);
	this->addChild(label, 1);
	this->addChild(pass,1);
	this->addChild(loginButton, 1);
	this->addChild(registerButton, 1);

	this->addChild(userTextField,1);
	this->addChild(passTextField,1);
    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("my1.png");
	sprite->setAnchorPoint(Vec2(visibleSize.width / sprite->getContentSize().width / 2.0, 0.5));
	auto movebyleft = MoveBy::create(35, Vec2(-1342, 0));
	auto movebyright = MoveBy::create(0, Vec2(1342, 0));
	auto seq = Sequence::create(movebyleft, movebyright, nullptr);
	sprite->runAction(RepeatForever::create(seq));

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
	//开启监听器
	this->scheduleUpdate();
	
	//记得改NULL
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
	
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
	#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}
