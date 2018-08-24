//
// Created by idanyani on 8/24/18.
//

#ifndef PTRACE_ALLOC_MOSAICCONFIGURATION_H
#define PTRACE_ALLOC_MOSAICCONFIGURATION_H

#include <cstddef>
#include <iostream>

using namespace std;

//TODO: check if this class could be refactored to one common base class with
// multipe derived classes (in terms of working with dlmalloc allocator)
class MosaicConfiguration {
public:
    enum class ConfigType {
        MMAP_POOL,
        BRK_POOL,
        FILE_BACKED_POOL,
        GENERAL
    };

    struct MosaicConfigurationParams {
        size_t _1gb_start;
        size_t _1gb_end;
        size_t _2mb_start;
        size_t _2mb_end;
        size_t _pool_size;
        size_t _ffa_list_size;
    };

    struct GeneralParams {
        bool _analyze_hpbrs;
    };

    MosaicConfiguration();

    ~MosaicConfiguration() {}

    MosaicConfigurationParams &ReadFromEnvironmentVariables(ConfigType type);
    GeneralParams &GetGeneralParams();

private:
    void ReadBrkPoolEnvParams(MosaicConfigurationParams &params);

    void ReadMmapPoolEnvParams(MosaicConfigurationParams &params);

    void ReadFileBackedPoolEnvParams(MosaicConfigurationParams &params);

    void ReadGeneralEnvParams(GeneralParams &params);

    const unsigned long GetEnvironmentVariableValue(const char *key) const;

    MosaicConfigurationParams _mmap_pool_params;
    MosaicConfigurationParams _brk_pool_params;
    MosaicConfigurationParams _file_backed_pool_params;
    GeneralParams _general_params;

    const size_t KB = 1024;
    const size_t MB = 1024*KB;
    const size_t GB = 1024*MB;

    const char* MMAP_1GB_START_OFFSET_ENV_VAR = "HPC_MMAP_1GB_START_OFFSET";
    const char* MMAP_1GB_END_OFFSET_ENV_VAR = "HPC_MMAP_1GB_END_OFFSET";
    const char* MMAP_2MB_START_OFFSET_ENV_VAR = "HPC_MMAP_2MB_START_OFFSET";
    const char* MMAP_2MB_END_OFFSET_ENV_VAR = "HPC_MMAP_2MB_END_OFFSET";
    const char* MMAP_POOL_SIZE_ENV_VAR = "HPC_MMAP_POOL_SIZE";
    const char* MMAP_FFA_SIZE_ENV_VAR = "HPC_MMAP_FIRST_FIT_LIST_SIZE";

    const char* BRK_1GB_START_OFFSET_ENV_VAR = "HPC_BRK_1GB_START_OFFSET";
    const char* BRK_1GB_END_OFFSET_ENV_VAR = "HPC_BRK_1GB_END_OFFSET";
    const char* BRK_2MB_START_OFFSET_ENV_VAR = "HPC_BRK_2MB_START_OFFSET";
    const char* BRK_2MB_END_OFFSET_ENV_VAR = "HPC_BRK_2MB_END_OFFSET";
    const char* BRK_POOL_SIZE_ENV_VAR = "HPC_BRK_POOL_SIZE";

    const char* FILE_BACKED_POOL_SIZE_ENV_VAR = "HPC_FILE_BACKED_POOL_SIZE";
    const char* FILE_BACKED_FFA_SIZE_ENV_VAR =
            "HPC_FILE_BACKED_FIRST_FIT_LIST_SIZE";

    const char* LOG_VERBOSITY_ENV_VAR = "HPC_LOG_VERBOSITY";
    const char* DEBUG_BREAK_ENV_VAR = "HPC_DEBUG_BREAK";
    const char* ANALYZE_HPBRS_ENV_VAR = "HPC_ANALYZE_HPBRS";
};


#endif //PTRACE_ALLOC_MOSAICCONFIGURATION_H
