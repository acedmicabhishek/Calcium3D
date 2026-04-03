#ifndef C3D_OPTIMIZATION_H
#define C3D_OPTIMIZATION_H

namespace C3D {
    namespace Optimization {
        void SetStaticBatching(bool enabled);
        void SetDynamicBatching(bool enabled);
        void SetAutoLOD(bool enabled);
        
        void SetVRS(bool enabled);
        void SetSSR(bool enabled);
        void SetHLOD(bool enabled);
        void SetOcclusionCulling(bool enabled);
    }
}

#endif
