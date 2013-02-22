//
//  TextureCompositiion.h
//

#ifndef __CompositiTextureTest__TextureCompositiion__
#define __CompositiTextureTest__TextureCompositiion__

#include <vector>
#include "cocos2d.h"

namespace cocos2d
{
    class CCString;
    class CCSpriteFrame;
    class CCArray;
    class CCImage;
    
};

#define CELLS_PER_GROUP 800

namespace CCHelper
{
    class TextureCompGroup
    {
    public:
        TextureCompGroup();
        ~TextureCompGroup();
        bool cell_is_empty( int cell_index );
        void set_cell_empty( int cell_index, bool empty );
        /*
         获得与cell index 对应的texture
         如果该贴图不存在则会创建
         */
        cocos2d::CCTexture2D* obtain_texture( int cell_index );

        int                     m_cell_width;
        int                     m_cell_height;
        int                     m_texture_width;
        int                     m_texture_height;
        unsigned char           m_cells[CELLS_PER_GROUP];// cell使用情况的bitset，最多支持800张贴图
        cocos2d::CCArray*       m_textures;// 强引用
    protected:
        void  set_texture( int index, cocos2d::CCTexture2D* texture );
        
    };
    
    /*
     使用frame_src_def为key，frame_data为value来对已经加载过的贴图做缓存
     */
    struct frame_src_def
    {
        
        bool operator < ( const frame_src_def& src ) const
        {
            if ( filename < src.filename )
                return true;
            else if ( filename == src.filename )
            {
                if ( (int)src_rect.origin.x < (int)src.src_rect.origin.x )
                    return true;
                else if ( (int)src_rect.origin.x == (int)src.src_rect.origin.x )
                {
                    if ( (int)src_rect.origin.y < (int)src.src_rect.origin.y )
                        return true;
                    else if ( (int)src_rect.origin.y == (int)src.src_rect.origin.y )
                    {
                    if ( (int)src_rect.size.width < (int)src.src_rect.size.width )
                        return true;
                        else if ( (int)src_rect.size.width == (int)src.src_rect.size.width )
                        {
                            if ( (int)src_rect.size.height < (int)src.src_rect.size.height )
                                return true;
                            else
                                return false;
                        }
                    }
                }
            }
            return false;
            
        }
        
        std::string filename;// TODO: 改成name可以节省内存消耗
        cocos2d::CCRect      src_rect;
    };
    struct frame_data
    {
        frame_data()
        :ref_count(0)
        {
            
        }
        int cell_width;
        int cell_height;
        int cell_index;
        int ref_count;
    };
    
    
    class TextureCompositionManager
    {
    public:
        TextureCompositionManager();
        ~TextureCompositionManager();
        /*
         设置最终合并出来的贴图的最大尺寸
         当超过该尺寸时会创建新的贴图
         group: 用来指定不同的图片组
         */
        void set_group_texture_size( int group, int width, int height );
        /*
         将指定的贴图加载并合并到一张大贴图中
         返回对应的CCSpriteFrame
         */
        cocos2d::CCSpriteFrame* load_spriteframe_from_file( const char* filename, cocos2d::CCRect rect );
        /*
         获得内存占用
         */
        struct mem_usage_desc
        {
            unsigned int frame_cached;
            unsigned int texture_memory_used;//not the actually video memory used
            unsigned int other_memory_used;
        };
        void get_memory_usage( mem_usage_desc& desc );
        /*
         释放一个frame: 减少一个frame的引用计数，为0时使对应的cell变为空闲，如果整张贴图的所有cell都空闲则删除这个贴图
         */
        void release_frame( const frame_src_def& frame_def );
        /*
         设置用来拼合贴图的动态贴图的大小
         */
        void set_comp_texture_size( int width, int height );
    protected:
        
        cocos2d::CCSpriteFrame* add_image_from_file( const char* filename, cocos2d::CCRect rect, TextureCompGroup* comp_group );
        TextureCompGroup* get_comp_group( int cell_width, int cell_height );
        
        int                                     m_comp_texture_width;
        int                                     m_comp_texture_height;
        typedef std::vector<TextureCompGroup*> TEXTURECOMPGROUPS;
        typedef std::map<frame_src_def, frame_data> FRAMECACHE;
        TEXTURECOMPGROUPS                       m_texture_groups;
        FRAMECACHE                              m_frame_cache;//已经创建过的frames，使用“贴图名＋rect”作为key
    };
    
    
    class CCSpriteFrameEx : public cocos2d::CCSpriteFrame
    {
    public:
        
        ~CCSpriteFrameEx();
        void setManagerAndCellIndex( TextureCompositionManager* tcm, const frame_src_def frame_def );
        
        static CCSpriteFrameEx* createWithTexture(cocos2d::CCTexture2D *pobTexture, const cocos2d::CCRect& rect);
        
    protected:
        TextureCompositionManager*  m_tcm_manager;
        frame_src_def m_frame_def;//保存位于合成贴图的描述，用于释放时通知TextureCompositionManager，减少对应frame的引用计数
        
    };
    
}

#endif /* defined(__CompositiTextureTest__TextureCompositiion__) */
