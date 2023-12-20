#include <cstdint>
#include <cstdio>
#include "lib/iokernelrw.h"

int main(void) {
    uint64_t strchr_signed = 0xDEADBEEFull;
    uint64_t strchr_unsigned = 0xFACEFEEDull;
    fprintf(stderr, "before: strchr_signed: 0x%016llx strchr_unsigned: 0x%016llx\n", strchr_signed, strchr_unsigned);
    auto client = iokernelrw_open();
    fprintf(stderr, "client: 0x%08x\n", (uint32_t)client);
    const auto kret = iokernelrw_get_strchr(client, &strchr_signed, &strchr_unsigned);
    fprintf(stderr, "after: kret: 0x%08x strchr_signed: 0x%016llx strchr_unsigned: 0x%016llx\n", (uint32_t)kret, strchr_signed, strchr_unsigned);
    return 0;
}
