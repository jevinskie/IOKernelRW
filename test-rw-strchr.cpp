#include <cstdint>
#include <cstdio>
#include "lib/iokernelrw.h"

// values for 14.3 23D5033f release t6000

int main(void) {
    auto client = iokernelrw_open();
    fprintf(stderr, "client: 0x%08x\n", (uint32_t)client);
    fprintf(stderr, "\n\n");

    uint64_t strchr_signed = 0xDEADBEEFull;
    uint64_t strchr_unsigned = 0xFACEFEEDull;
    fprintf(stderr, "before: strchr_signed: 0x%016llx strchr_unsigned: 0x%016llx\n", strchr_signed, strchr_unsigned);
    const auto kret = iokernelrw_get_strchr(client, &strchr_signed, &strchr_unsigned);
    fprintf(stderr, "after: kret: 0x%08x strchr_signed: 0x%016llx strchr_unsigned: 0x%016llx\n", (uint32_t)kret, strchr_signed, strchr_unsigned);
    const uint64_t strchr_unslid = 0xffff'fe00'0741'4414ULL;
    fprintf(stderr, "strchr_slid:   0x%016llx\n", strchr_unsigned);
    fprintf(stderr, "strchr_unslid: 0x%016llx\n", strchr_unslid);
    uint8_t strchr_prologue[32] = {0};
    const auto kret2 = iokernelrw_read(client, strchr_unsigned, (void *)strchr_prologue, sizeof(strchr_prologue));
    fprintf(stderr, "after read: kret2: 0x%08x strchr_prologue: %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n", (uint32_t)kret2, strchr_prologue[0], strchr_prologue[1], strchr_prologue[2], strchr_prologue[3], strchr_prologue[4], strchr_prologue[5], strchr_prologue[6], strchr_prologue[7], strchr_prologue[8], strchr_prologue[9], strchr_prologue[10], strchr_prologue[11], strchr_prologue[12], strchr_prologue[13], strchr_prologue[14], strchr_prologue[15], strchr_prologue[16], strchr_prologue[17], strchr_prologue[18], strchr_prologue[19], strchr_prologue[20], strchr_prologue[21], strchr_prologue[22], strchr_prologue[23], strchr_prologue[24], strchr_prologue[25], strchr_prologue[26], strchr_prologue[27], strchr_prologue[28], strchr_prologue[29], strchr_prologue[30], strchr_prologue[31]);
    fprintf(stderr, "\n\n");

    const uint64_t slide = strchr_unsigned - strchr_unslid;
    fprintf(stderr, "kernel slide: 0x%016llx\n", slide);
    fprintf(stderr, "\n\n");

    const uint64_t pmap_ppl_disable_unslid = 0xffff'fe00'0715'561cULL;
    fprintf(stderr, "pmap_ppl_disable_unslid: 0x%016llx\n", pmap_ppl_disable_unslid);
    const uint64_t pmap_ppl_disable_slid = pmap_ppl_disable_unslid + slide;
    fprintf(stderr, "pmap_ppl_disable_slid:   0x%016llx\n", pmap_ppl_disable_slid);
    uint32_t pmap_ppl_disable_val = 0xDEADBEEFu;
    fprintf(stderr, "before read: pmap_ppl_disable_val: 0x%08x\n", pmap_ppl_disable_val);
    const auto kret3 = iokernelrw_read(client, pmap_ppl_disable_slid, (void *)&pmap_ppl_disable_val, sizeof(pmap_ppl_disable_val));
    fprintf(stderr, "after read: kret3: 0x%08x pmap_ppl_disable_val: 0x%08x\n", (uint32_t)kret3, pmap_ppl_disable_val);
    fprintf(stderr, "\n\n");


    const uint64_t pmap_ppl_locked_down_unslid = 0xffff'fe00'07b6'00e0ULL;
    fprintf(stderr, "pmap_ppl_locked_down_unslid: 0x%016llx\n", pmap_ppl_locked_down_unslid);
    const uint64_t pmap_ppl_locked_down_slid = pmap_ppl_locked_down_unslid + slide;
    fprintf(stderr, "pmap_ppl_locked_down_slid:   0x%016llx\n", pmap_ppl_locked_down_slid);
    uint32_t pmap_ppl_locked_down_val = 0xFACEFEEDu;
    fprintf(stderr, "before read: pmap_ppl_locked_down_val: 0x%08x\n", pmap_ppl_locked_down_val);
    const auto kret4 = iokernelrw_read(client, pmap_ppl_locked_down_slid, (void *)&pmap_ppl_locked_down_val, sizeof(pmap_ppl_locked_down_val));
    fprintf(stderr, "after read: kret4: 0x%08x pmap_ppl_locked_down_val: 0x%08x\n", (uint32_t)kret4, pmap_ppl_locked_down_val);
    fprintf(stderr, "\n\n");


    const uint64_t csr_unsafe_kernel_text_unslid = 0xffff'fe00'071f'7e8cULL;
    fprintf(stderr, "csr_unsafe_kernel_text_unslid: 0x%016llx\n", csr_unsafe_kernel_text_unslid);
    const uint64_t csr_unsafe_kernel_text_slid = csr_unsafe_kernel_text_unslid + slide;
    fprintf(stderr, "csr_unsafe_kernel_text_slid:   0x%016llx\n", csr_unsafe_kernel_text_slid);
    uint8_t csr_unsafe_kernel_text_val = 243;
    fprintf(stderr, "before read: csr_unsafe_kernel_text_val: 0x%02hhx\n", csr_unsafe_kernel_text_val);
    const auto kret5 = iokernelrw_read(client, csr_unsafe_kernel_text_slid, (void *)&csr_unsafe_kernel_text_val, sizeof(csr_unsafe_kernel_text_val));
    fprintf(stderr, "after read: kret5: 0x%08x csr_unsafe_kernel_text_val: 0x%02hhx\n", (uint32_t)kret5, csr_unsafe_kernel_text_val);
    fprintf(stderr, "\n\n");


    const uint64_t vm_kernel_slide_unslid = 0xffff'fe00'0726'8b08ULL;
    fprintf(stderr, "vm_kernel_slide_unslid: 0x%016llx\n", vm_kernel_slide_unslid);
    const uint64_t vm_kernel_slide_slid = vm_kernel_slide_unslid + slide;
    fprintf(stderr, "vm_kernel_slide_slid:   0x%016llx\n", vm_kernel_slide_slid);
    uint64_t vm_kernel_slide_val = 0xBAADC0DEull;
    fprintf(stderr, "before read: vm_kernel_slide_val: 0x%016llx\n", vm_kernel_slide_val);
    const auto kret6 = iokernelrw_read(client, vm_kernel_slide_slid, (void *)&vm_kernel_slide_val, sizeof(vm_kernel_slide_val));
    fprintf(stderr, "after read: kret6: 0x%08x vm_kernel_slide_val: 0x%016llx\n", (uint32_t)kret6, vm_kernel_slide_val);
    fprintf(stderr, "\n\n");

    return 0;
}
