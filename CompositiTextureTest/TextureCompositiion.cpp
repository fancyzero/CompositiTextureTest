//
//  TextureCompositiion.cpp
//  CompositiTextureTest
//
//  Created by FancyZero on 13-2-18.
//
//

#include "TextureCompositiion.h"
#include "cocos2d.h"
#include <bitset>
#include <stdlib.h>
#include "string.h"
using namespace cocos2d;

namespace CCHelper
{
    /*
     TextureCompGroup
     */
    TextureCompGroup::TextureCompGroup()
    :m_cell_width(0), m_cell_height(0)
    {
        ::memset( m_cells, 0, sizeof(m_cells) );
        m_textures = CCArray::create();
        m_textures->retain();
    }
    
    TextureCompGroup::~TextureCompGroup()
    {
        if ( m_textures != NULL )
            m_textures->release();
        m_textures = NULL;
    }
    
    bool TextureCompGroup::cell_is_empty( int cell_index )
    {
        return !(m_cells[cell_index/8] & (1<< (cell_index%8)));
    }
    
    void TextureCompGroup::set_cell_empty( int cell_index, bool empty)
    {
        if ( empty )
            m_cells[cell_index/8] = (m_cells[cell_index/8] & ~(1<< (cell_index%8)));
        else
            m_cells[cell_index/8] =  (m_cells[cell_index/8] | (1<< (cell_index%8)));
        // 检查该cell对应的texture是否全部为空闲，是则删除该贴图
        if ( 0 & empty )
        {
            int cells_per_tex = ( m_texture_width / m_cell_width ) * ( m_texture_height / m_cell_height );
            int cell_index_start = cell_index / cells_per_tex * cells_per_tex;
            bool total_empty = true;
            for( int i = 0; i < cells_per_tex; i++ )
            {
                if ( !cell_is_empty(cell_index_start + i) )
                    total_empty = false;
            }
            if ( total_empty )
            {
                int texture_index = cell_index / cells_per_tex;
                CCTexture2D* tex = (CCTexture2D*) m_textures->objectAtIndex( texture_index );
                if ( tex != NULL )
                {
                    CCLog("Release texture: %x, retaincount before release: %d", tex, tex->retainCount());
                    // 由于可能有sprite对这张贴图的引用，所以这里的retaincount 不一定为 1
                    //assert( tex->retainCount() == 1 );
                    tex->release();
                    m_textures->removeObjectAtIndex(texture_index);
                    // 构造一个空的texture来占位
                    CCTexture2D* dummy_texture = new CCTexture2D();
                    set_texture(texture_index, dummy_texture);
                }
                
            }
        }
    }
    
    void TextureCompGroup::set_texture( int index, cocos2d::CCTexture2D *texture )
    {
        if ( index >= m_textures->count() )
            m_textures->addObject( texture );
        else
            m_textures->insertObject( texture, index );
    }
    
    CCTexture2D* TextureCompGroup::obtain_texture(int cell_index)
    {
    //    CCTexture2D * tttt = (CCTextureCache::sharedTextureCache()->addImage("1024test.png"));
  //      tttt->retain();
//        return tttt;
        int cells_per_tex = ( m_texture_width / m_cell_width ) * ( m_texture_height / m_cell_height );
        int texture_index = cell_index / cells_per_tex;
        assert( texture_index <= m_textures->count() );//
        if ( (texture_index >= m_textures->count()) || ((m_textures->objectAtIndex(texture_index) != NULL) &&
            (((CCTexture2D*)m_textures->objectAtIndex(texture_index))->getPixelsHigh() == 0)) )
        {
            //需要创建新贴图
            CCTexture2D* new_tex = new CCTexture2D();
            if ( texture_index >= m_textures->count() )
                m_textures->addObject(new_tex);
            else
                set_texture(texture_index, new_tex);
            
            unsigned int buffer_size = m_texture_width * m_texture_height*4;
            unsigned char* rgba8888_data = new unsigned char[buffer_size];
            memset( rgba8888_data, 0, buffer_size );
            /*
             测试代码
             for ( int j = 0; j < comp_group->m_texture_height; j ++ )
             for ( int i = 0; i < comp_group->m_texture_width; i += 4 )
             {
             rgba8888_data[j*comp_group->m_texture_width*4+i] = CCRANDOM_0_1()*255;
             rgba8888_data[j*comp_group->m_texture_width*4+i+1] = CCRANDOM_0_1()*255;
             rgba8888_data[j*comp_group->m_texture_width*4+i+2] = CCRANDOM_0_1()*255;
             rgba8888_data[j*comp_group->m_texture_width*4+i+3] = CCRANDOM_0_1()*255;
             }
             */
            new_tex->initWithData( rgba8888_data, kCCTexture2DPixelFormat_RGBA8888, m_texture_width, m_texture_height, CCSizeMake((float)m_texture_width, (float)m_texture_height) );
            CHECK_GL_ERROR_DEBUG();
            delete[] rgba8888_data;
            //new_tex->retain();
            CCLog("create new texture");
            CCLog("texture %x obtained retaincount: %d", new_tex, new_tex->retainCount() );
            return new_tex;
            //break;
        }
        else
        {
            CCTexture2D* tex = (CCTexture2D*)m_textures->objectAtIndex(texture_index);
            CCLog("texture %x obtained retaincount: %d", tex, tex->retainCount() );
            return tex;
        }
        
    }
    
    /*
     End of TextureCompGroup
     */
    
    
    /*
     TextureCompositionManager
     */
    TextureCompositionManager::TextureCompositionManager()
    {
        
    }
    
    TextureCompositionManager::~TextureCompositionManager()
    {
        TEXTURECOMPGROUPS::iterator it;
        for ( it = m_texture_groups.begin(); it != m_texture_groups.end(); ++it )
        {
            delete (*it);
        }
        m_texture_groups.clear();
    }
    
    CCSpriteFrame* TextureCompositionManager::add_image_from_file( const char* filename, CCRect rect, TextureCompGroup* comp_group )
    {
        int bpp = 4;
        CCImage img;
        img.initWithImageFile(filename);
        CCImage* image = &img;

        //        CCObject* obj;
        if ( image == NULL )
            return NULL;
        if ( comp_group == NULL )
            return NULL;
        int cells_per_tex = ( comp_group->m_texture_width / comp_group->m_cell_width ) * ( comp_group->m_texture_height / comp_group->m_cell_height );
        
        
        //int image_added = 0;
        //CCTexture2D* current_texture = NULL;// 当前将要被插入图片的 CompTexture
        // while( image_added < images->count() )
        // {
        //CCImage* image = (CCImage*)images->objectAtIndex(image_added);
        //if ( image == NULL )
        //    continue;
        unsigned char * data = image->getData();
        unsigned char * sub_data = NULL;
        
        
        // 检查图片尺寸是否合法
        //CCSize tex_size = tex->getContentSizeInPixels();
        CCSize img_size;
        img_size.width = image->getWidth();
        img_size.height = image->getHeight();
        
        // rect未设置时默认取原图完整大小
        if ( rect.size.width == 0 && rect.size.height == 0 )
        {
            rect.origin.x = rect.origin.y = 0;
            rect.size.width = image->getWidth();
            rect.size.height = image->getHeight();
        }
        
        // src rect 必须位于源图内
        if ( rect.origin.x < 0 ||
            rect.origin.y < 0 ||
            rect.origin.x + rect.size.width > img_size.width ||
            rect.origin.y + rect.size.height > img_size.height )
        {
            // TODO: warning
            CCLog("can't add image, invalid source rect.");
            return NULL;
        }
        // src rect 大小必须符合CompTexureGropup的大小设定
        if ( rect.size.width != comp_group->m_cell_width || rect.size.height != comp_group->m_cell_height )
        {
            CCLog("can't add image, source rect is different to the comptexturegroup settings.");
            return NULL;
        }
        
        if ( rect.size.width != img_size.width || rect.size.height != img_size.height )
        {
            // 如果只取部分图片，修改data为部分数据
            sub_data = new unsigned char[(int)rect.size.width*(int)rect.size.width * bpp];
            for ( int y = 0; y < (int)rect.size.height; y++ )//一行一行从data中复制像素到sub_data
            {
                unsigned char* write_start = sub_data + (int)rect.size.width * bpp * y;
                unsigned char* read_start = data + ((int)rect.origin.y + y) * (int)img_size.width * bpp + (int)rect.origin.x * bpp ;
                memcpy( write_start, read_start, (int)rect.size.width*bpp );
            }
            data = sub_data;
        }
        
        
        
        CCTexture2D* comp_texture_used = NULL;
        int cell_index = 0;
        CCRect frame_rect;
        
        // 找到第一个空闲的Cell，将图片插入此cell所对应的Texture中
        for ( cell_index = 0; cell_index < CELLS_PER_GROUP; cell_index++ )
        {
            // m_cells中每一个bit位代表一个cell是否被占用
            if ( comp_group->cell_is_empty(cell_index) )
            {
                // 插入图片
                int sub_cell_index = cell_index % cells_per_tex;//将cell_index 转换成相对于贴图的index
                
                int cells_per_line = comp_group->m_texture_width / comp_group->m_cell_width;//compotexture中每一行的cell数
                //计算这个frame所占的cell在compotexture上的rect
                frame_rect = CCRectMake( (sub_cell_index%cells_per_line)*comp_group->m_cell_width, sub_cell_index/cells_per_line*comp_group->m_cell_height, comp_group->m_cell_width, comp_group->m_cell_height );
                
                //往已有贴图的空闲cell中插入图片
                //使用TexSubImage2D修改贴图的部分区域
                comp_texture_used = (CCTexture2D*)comp_group->obtain_texture(cell_index);

                
                CCLog("update texture sub image: texture: %x name: %d", comp_texture_used, comp_texture_used->getName());
                glBindTexture( GL_TEXTURE_2D, comp_texture_used->getName());
                CHECK_GL_ERROR_DEBUG();
                glPixelStorei(GL_UNPACK_ALIGNMENT,4);
                CHECK_GL_ERROR_DEBUG();
                
                //glUseProgram(0);//disable shader
                //CHECK_GL_ERROR_DEBUG();
                
                glTexSubImage2D( GL_TEXTURE_2D, 0, frame_rect.origin.x, frame_rect.origin.y, frame_rect.size.width, frame_rect.size.height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                CHECK_GL_ERROR_DEBUG(); 
                
                if ( sub_data != NULL )
                {
                    delete[] sub_data;
                    sub_data = NULL;
                }

                break;
            }
            
        }
        
        if ( sub_data != NULL )
        {
            delete[] sub_data;
            sub_data = NULL;
        }
        if ( cell_index >= CELLS_PER_GROUP )
        {
            CCLog( "error comp_group %d,%d is full!!", comp_group->m_cell_width, comp_group->m_cell_height);
            return NULL;
        }
        //标记cell为被占用
        comp_group->set_cell_empty( cell_index, false );
        CCLog("add texture into cell %d", cell_index);
 
        CCSpriteFrameEx* frame = CCSpriteFrameEx::createWithTexture( comp_texture_used, frame_rect );
        CCLog("create spriteframe from texture %x(retaincount: %d), rect: (x:%.2f,y:%.2f,w:%.2f,h:%.2f)", comp_texture_used, comp_texture_used->retainCount(), frame_rect.origin.x, frame_rect.origin.y, frame_rect.size.width, frame_rect.size.height );
        
        frame_src_def fsd;
        fsd.filename = filename;
        fsd.src_rect = rect;
        // 设置frame信息到spriteframe中
        frame->setManagerAndCellIndex(this, fsd );
        // 将创建的frame加入缓存
        frame_data framedata;
        framedata.cell_index = cell_index;
        framedata.cell_width = comp_group->m_cell_width;
        framedata.cell_height = comp_group->m_cell_height;
        
        
        FRAMECACHE::mapped_type& fmdef = m_frame_cache[fsd];
        fmdef.cell_index = cell_index;
        fmdef.cell_width = comp_group->m_cell_width;
        fmdef.cell_height = comp_group->m_cell_height;
        fmdef.ref_count++;
        CCLog("update cache with cell: %d, group : %d-%d, refcount:%d", cell_index, comp_group->m_cell_width, comp_group->m_cell_height, fmdef.ref_count );
        return frame;
    }
    /*
     获取制定的CompGroup，如果不存在则创建一个
     */
    TextureCompGroup* TextureCompositionManager::get_comp_group( int cell_width, int cell_height )
    {
        TextureCompGroup* comp_group = NULL;
        // 先找一下所指定的goup是否已经存在
        TEXTURECOMPGROUPS::iterator it;
        for ( it = m_texture_groups.begin(); it != m_texture_groups.end(); ++it )
        {
            if ( (*it)->m_cell_width == cell_width && (*it)->m_cell_height == cell_height )
            {
                comp_group = (*it);
                break;
            }
        }
        // 不存在则添加一个新的group
        if ( comp_group == NULL )
        {
            comp_group = new TextureCompGroup();
            // 使用请求的资源大小为cell大小
            comp_group->m_cell_width = cell_width;
            comp_group->m_cell_height = cell_height;
            
            comp_group->m_texture_width = 1024;
            comp_group->m_texture_height = 1024;
            m_texture_groups.push_back( comp_group );
        }
        
        return comp_group;
    }
    
    CCSpriteFrame* TextureCompositionManager::load_spriteframe_from_file( const char* filename, CCRect rect )
    {
        if ( filename == NULL )
            return NULL;
        assert( rect.size.width > 0 && rect.size.height > 0 );
        
        // 如果已经加载过，则直接生成一个Frame返回给用户
        frame_src_def def;
        def.filename = filename;
        def.src_rect = rect;
        
        if ( m_frame_cache.find(def) != m_frame_cache.end() )
        {
            // 已经加载过
            // 直接生成一个spriteframe返回给用户，并且为该frame_src_def的引用计数加1
            TextureCompGroup* comp_group = get_comp_group( rect.size.width, rect.size.height );
            int cell_index = m_frame_cache[def].cell_index;
            int cells_per_tex = ( comp_group->m_texture_width / comp_group->m_cell_width ) * ( comp_group->m_texture_height / comp_group->m_cell_height );
            int sub_cell_index = cell_index % cells_per_tex;// 转换到对于单个贴图的cellindex
            CCTexture2D* comp_texture_used = (CCTexture2D*)comp_group->obtain_texture( cell_index );
            if ( comp_texture_used == NULL )
            {
                CCLOG("error: a cached resource was lost!!");
                assert(0);
                return NULL;
            }
            CCRect frame_rect = CCRectMake( (sub_cell_index%8)*comp_group->m_cell_width, sub_cell_index/8*comp_group->m_cell_height, comp_group->m_cell_width, comp_group->m_cell_height );
            
            CCLog("create spriteframe from cached texture %x, rect: (x:%.2f,y:%.2f,w:%.2f,h:%.2f)", comp_texture_used, frame_rect.origin.x, frame_rect.origin.y, frame_rect.size.width, frame_rect.size.height );
            CCSpriteFrameEx* frame = CCSpriteFrameEx::createWithTexture( comp_texture_used, frame_rect );
            frame_src_def frame_def;
            frame_def.filename = filename;
            frame_def.src_rect = rect;
            frame->setManagerAndCellIndex( this, frame_def );//记录来自于哪个frame
            // 增加值这个frame的引用计数
            m_frame_cache[def].ref_count++;
            CCLog("update cache with cell: %d, group : %d-%d, refcount:%d", m_frame_cache[def].cell_index, m_frame_cache[def].cell_width, m_frame_cache[def].cell_height, m_frame_cache[def].ref_count );
            return frame;
        }
        
        TextureCompGroup* comp_group = get_comp_group( rect.size.width, rect.size.height );
        if ( comp_group == NULL )
            return NULL;
        return add_image_from_file( filename, rect, comp_group );
    }
    
    void TextureCompositionManager::get_memory_usage( mem_usage_desc& desc )
    {
        FRAMECACHE::iterator it;
        desc.other_memory_used = 0;
        desc.texture_memory_used = 0;
        desc.frame_cached = m_frame_cache.size();
        for ( it = m_frame_cache.begin(); it != m_frame_cache.end(); ++it )
        {
            desc.other_memory_used += it->first.filename.size()+1;
            desc.other_memory_used += sizeof(CCRect);
            desc.other_memory_used += sizeof(frame_data);
        }
        desc.other_memory_used += m_texture_groups.size() * sizeof(TextureCompGroup);
        for ( TEXTURECOMPGROUPS::iterator it = m_texture_groups.begin(); it != m_texture_groups.end(); ++it )
        {
            desc.texture_memory_used += (*it)->m_textures->count() * (*it)->m_texture_width * (*it)->m_texture_height * 4;
        }
    }
    
    void TextureCompositionManager::release_frame( const frame_src_def& frame_def )
    {
        
        if ( m_frame_cache.find( frame_def ) == m_frame_cache.end() )
        {
            
            CCLog("error： 试图释放一个不存在的frame, filename: %s rect: %.2f,%.2f,%.2f,%.2f", frame_def.filename.c_str(), frame_def.src_rect.origin.x, frame_def.src_rect.origin.y, frame_def.src_rect.size.width, frame_def.src_rect.size.height );
            assert(0);
            return;
        }
        
        if ( m_frame_cache[frame_def].ref_count == 0 )
        {
            CCLog("error: bad ref_count of frame");
            assert(0);
            return;
        }
        
        m_frame_cache[frame_def].ref_count --;
        
        CCLog("release frame, filename: %s rect: %.2f,%.2f,%.2f,%.2f refcount: %d", frame_def.filename.c_str(), frame_def.src_rect.origin.x, frame_def.src_rect.origin.y, frame_def.src_rect.size.width, frame_def.src_rect.size.height, m_frame_cache[frame_def].ref_count );
        if ( m_frame_cache[frame_def].ref_count == 0 )
        {
            // 标记对应的cell为empty
            TextureCompGroup* g = get_comp_group(m_frame_cache[frame_def].cell_width, m_frame_cache[frame_def].cell_height);
            g->set_cell_empty( m_frame_cache[frame_def].cell_index, true );
            CCLog("set cell %d as empty cell", m_frame_cache[frame_def].cell_index );
            // 释放 frame chache 中的对应项
            m_frame_cache.erase(frame_def);
        }
        
    }
    
    /*
     CCSpriteFrameEx
     */
    CCSpriteFrameEx* CCSpriteFrameEx::createWithTexture(CCTexture2D *pobTexture, const CCRect& rect)
    {
        CCSpriteFrameEx *pSpriteFrame = new CCSpriteFrameEx();;
        pSpriteFrame->initWithTexture(pobTexture, rect);
        pSpriteFrame->autorelease();
        
        return pSpriteFrame;
    }
    
    CCSpriteFrameEx::~CCSpriteFrameEx()
    {
        if ( m_tcm_manager != NULL )
        {
            m_tcm_manager->release_frame( m_frame_def );
        }
    }
    
    void CCSpriteFrameEx::setManagerAndCellIndex( TextureCompositionManager* tcm, const frame_src_def frame_def )
    {
        m_tcm_manager = tcm;
        m_frame_def.filename = frame_def.filename;
        m_frame_def.src_rect = frame_def.src_rect;
        CCLog("set frame def: %d, %d, %d, %d",(int) m_frame_def.src_rect.origin.x,(int) m_frame_def.src_rect.origin.y,(int) m_frame_def.src_rect.size.width,(int) m_frame_def.src_rect.size.height );
    }
}