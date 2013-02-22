#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;
#include "TextureCompositiion.h"
int iframes = 0;
CCHelper::TextureCompositionManager tcm;
CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
CCSprite* retaincount_test_spr = NULL;
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.
    
    // add a "close" icon to exit the progress. it's an autorelease object
    //CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
      //                                                    "CloseNormal.png",
        //                                                  "CloseSelected.png",
          //                                                this,
            //                                              menu_selector(HelloWorld::menuCloseCallback) );
    //pCloseItem->setPosition( ccp(CCDirector::sharedDirector()->getWinSize().width - 20, 20) );
    
    // create menu, it's an autorelease object
    //CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    //pMenu->setPosition( CCPointZero );
    //this->addChild(pMenu, 1);
    
    /////////////////////////////
    // 3. add your codes below...
    
    // add a label shows "Hello World"
    // create and initialize a label
    //CCLabelTTF* pLabel = CCLabelTTF::create("Hello World", "Thonburi", 34);
    
    // ask director the window size
    //CCSize size = CCDirector::sharedDirector()->getWinSize();
    
    // position the label on the center of the screen
    //pLabel->setPosition( ccp(size.width / 2, size.height - 20) );
    
    // add the label as a child to this layer
    //this->addChild(pLabel, 1);
    
    //CCTexture2D* tex = CCTextureCache::sharedTextureCache()->addImage("1024test.png");
    
    //CCHelper::CCSpriteFrameEx* frm = CCHelper::CCSpriteFrameEx::createWithTexture(tex, CCRectMake(0, 0, 40, 40));
    
    // add "HelloWorld" splash screen"
    //CCSprite* pSprite = CCSprite::createWithSpriteFrame(frm);//("1024test.png");
    
    // position the sprite on the center of the screen
    //pSprite->setPosition( ccp(size.width/2, size.height/2) );
    
    // add the sprite as a child to this layer
    //this->addChild(pSprite, 0);
    
    this->setTouchEnabled(true);

    return true;
}


void HelloWorld::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    int perline = 1024/40;
    // 下面这一行可以设置动态贴图的大小，默认为1024x1024
    //tcm.set_comp_texture_size( 128,128 );
    for ( int i = 0; i < 1; i++ )
    {
        // 通过文件名和一个rect作为参数，load一个frame

        CCSpriteFrame* spfrm = tcm.load_spriteframe_from_file( "1024test.png", CCRectMake((iframes%perline)*40, (iframes/perline)*40, 40, 40) );
        assert( spfrm );       
        // 注意，必须保持返回的引用，否则一旦被autorelease后，会被认为该frame不再有人使用，从而会被挪作他用
        spfrm->retain();


        CCSprite* testspr;
        retaincount_test_spr = testspr = CCSprite::createWithSpriteFrame(spfrm );
        //testspr->setPosition( ccp(0,0));
        testspr->setPosition( ccp((iframes%perline)*40+20, 748-(iframes/perline)*40) );
        this->addChild(testspr, 0);
        CCHelper::TextureCompositionManager::mem_usage_desc desc;
        tcm.get_memory_usage(desc);
        CCLog("memory usage: frame: %d, texture: %.2fMB, other: %.2fMB", desc.frame_cached, desc.texture_memory_used/1024.0f/1024.0f, desc.other_memory_used/1024.0f/1024.0f);
        iframes++;
    }

}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    CCDirector::sharedDirector()->end();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
