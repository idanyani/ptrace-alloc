#include "huge_pages_configuration.h"

int main() {
    MosaicConfiguration hppc;
    auto mmap_conf = hppc.ReadFromEnvironmentVariables
            (MosaicConfiguration::ConfigType::MMAP_POOL);
    std::cout << "MMAP Config:" << std::endl
              << "\tPool Size: " << mmap_conf._pool_size << std::endl
              << "\t1GB Start: " << mmap_conf._1gb_start << std::endl
              << "\t1GB End: " << mmap_conf._1gb_end << std::endl
              << "\t2MB Start: " << mmap_conf._2mb_start << std::endl
              << "\t2MB End: " << mmap_conf._2mb_end << std::endl
              << std::endl;

    auto brk_conf = hppc.ReadFromEnvironmentVariables
            (MosaicConfiguration::ConfigType::BRK_POOL);
    std::cout << "BRK Config:" << std::endl
              << "\tPool Size: " << brk_conf._pool_size << std::endl
              << "\t1GB Start: " << brk_conf._1gb_start << std::endl
              << "\t1GB End: " << brk_conf._1gb_end << std::endl
              << "\t2MB Start: " << brk_conf._2mb_start << std::endl
              << "\t2MB End: " << brk_conf._2mb_end << std::endl
              << std::endl;
}
