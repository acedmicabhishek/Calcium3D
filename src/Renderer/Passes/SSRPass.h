#ifndef SSR_PASS_H
#define SSR_PASS_H

#include "../RenderPipeline.h"
#include "../Shader.h"

class SSRPass : public RenderPass {
public:
    enum class PassType {
        Geometry,        
        Transparency,    
        All,             
        CubemapOnly      
    };

    SSRPass(PassType type);
    ~SSRPass() override;

    void Init() override;
    void Execute(const RenderContext& context) override;

private:
    Shader* m_SSRShader = nullptr;
    unsigned int m_QuadVAO = 0;
    unsigned int m_QuadVBO = 0;
    
    unsigned int m_ResolveFBO = 0;
    unsigned int m_ResolveColorObj = 0;
    unsigned int m_ResolveNormalObj = 0;
    unsigned int m_ResolveDepthObj = 0;
    
    int m_LastWidth = 0;
    int m_LastHeight = 0;
    
    PassType m_PassType;

    void SetupQuad();
    void ResizeResolveFBO(int width, int height);

    
    unsigned int m_GeoNormalArchive = 0;

    static unsigned int s_GeometryNormalArchive; 
    static unsigned int s_GeometryDepthArchive;  
    static unsigned int s_ArchiveFBO;             
    static int          s_ArchiveWidth, s_ArchiveHeight;
    static void EnsureGeoNormalArchive(int w, int h);

public:
    static unsigned int GetGeometryDepth() { return s_GeometryDepthArchive; }
};

#endif
