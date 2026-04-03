#ifndef C3D_SYSTEM_H
#define C3D_SYSTEM_H

#include <string>

namespace C3D {
    namespace Time {
        float DeltaTime();
        float TotalTime();
        void SetTimeScale(float scale);
        float GetTimeScale();
    }

    namespace Log {
        void Trace(const std::string& msg);
        void Info(const std::string& msg);
        void Warning(const std::string& msg);
        void Error(const std::string& msg);
    }
}

#endif
