#include "Application.h"
#include "ResourceManager.h"
#include "GPUManager.h"

#ifdef C3D_RUNTIME
#include "RuntimeApplication.h"
#else
#include "EditorApplication.h"
#endif

int main(int argc, char** argv) {
    GPUManager::EnsureProperGPU(argc, argv);
    
    printf("[Calcium3D] Engine Starting...\n");
    
    ApplicationSpecification spec;
    spec.Name = "Calcium3D";
    spec.Width = 1200;
    spec.Height = 800;
    
#ifdef C3D_RUNTIME
    RuntimeApplication app(spec);
#else
    EditorApplication app(spec);
#endif

    if (!app.Init()) {
        return -1;
    }
    
    app.Run();
    return 0;
}