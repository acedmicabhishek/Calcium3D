#pragma once

#include <string>
#include <vector>

class GPUManager {
public:
    enum class GPUVendor { NONE, NVIDIA, AMD, INTEL };
    
    struct GPUInfo {
        int index;
        GPUVendor vendor;
        std::string name;
        std::string vendorId;
    };

    static void EnsureProperGPU(int argc, char** argv);
    static std::vector<GPUInfo> GetAvailableGPUs();
    static void SetPreferredGPU(int index);
    static int GetPreferredGPU();

private:
    static GPUVendor GetDetectedVendor();
    static bool IsPrimeOffloadEnabled();
    static std::string GetVendorName(GPUVendor vendor);
};
