#include <string.h>
#include <kern/task.h>
#include <libkern/copyio.h>
#include <mach/vm_param.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IOMemoryDescriptor.h>
#include "IOKernelRWUserClient.h"
#if __has_include(<ptrauth.h>)
#include <ptrauth.h>
#endif

#define super IOUserClient
OSDefineMetaClassAndFinalStructors(IOKernelRWUserClient, IOUserClient)

bool IOKernelRWUserClient::initWithTask(task_t owningTask, void *securityID, uint32_t type)
{
    if(!super::initWithTask(owningTask, securityID, type))
    {
        return false;
    }

    bool allow = false;
    OSObject *entitlement = copyClientEntitlement(owningTask, "com.apple.security.siguza.kernel-rw");
    if(entitlement)
    {
        allow = (entitlement == kOSBooleanTrue);
        entitlement->release();
    }
    return allow;
}

IOReturn IOKernelRWUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *args, IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
    static const IOExternalMethodDispatch methods[] =
    {
        /* 0 */ { (IOExternalMethodAction)&IOKernelRWUserClient::readVirt,  3, 0, 0, 0 },
        /* 1 */ { (IOExternalMethodAction)&IOKernelRWUserClient::writeVirt, 3, 0, 0, 0 },
        /* 2 */ { (IOExternalMethodAction)&IOKernelRWUserClient::readPhys,  4, 0, 0, 0 },
        /* 3 */ { (IOExternalMethodAction)&IOKernelRWUserClient::writePhys, 4, 0, 0, 0 },
        /* 4 */ { (IOExternalMethodAction)&IOKernelRWUserClient::getStrchr, 2, 0, 0, 0 },
        /* 5 */ { (IOExternalMethodAction)&IOKernelRWUserClient::getkOSBooleanTrue, 2, 0, 0, 0 },
    };

    if(selector < sizeof(methods)/sizeof(methods[0]))
    {
        dispatch = const_cast<IOExternalMethodDispatch*>(&methods[selector]);
        target = this;
    }

    return super::externalMethod(selector, args, dispatch, target, reference);
}

IOReturn IOKernelRWUserClient::readVirt(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args)
{
    int r = copyout((const void*)args->scalarInput[0], (user_addr_t)args->scalarInput[1], args->scalarInput[2]);
    return r == 0 ? kIOReturnSuccess : kIOReturnVMError;
}

IOReturn IOKernelRWUserClient::writeVirt(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args)
{
    int r = copyin((user_addr_t)args->scalarInput[0], (void*)args->scalarInput[1], args->scalarInput[2]);
    return r == 0 ? kIOReturnSuccess : kIOReturnVMError;
}

IOReturn IOKernelRWUserClient::physcopy(uint64_t src, uint64_t dst, uint64_t len, uint64_t alignment, IODirection direction)
{
    IOReturn retval = kIOReturnError;
    IOReturn ret;
    IOOptionBits mapOptions = 0;

    uint64_t va = direction == kIODirectionIn ? dst : src;
    uint64_t pa = direction == kIODirectionIn ? src : dst;

    switch(alignment)
    {
        case 0:
            break;

        case 4:
        case 8:
            if((pa % alignment) != 0 || (len % alignment) != 0)
            {
                return kIOReturnNotAligned;
            }
            mapOptions |= kIOMapInhibitCache;
            break;

        default:
            return kIOReturnBadArgument;
    }

    uint64_t voff = va & PAGE_MASK;
    uint64_t poff = pa & PAGE_MASK;
    va &= ~(uint64_t)PAGE_MASK;
    pa &= ~(uint64_t)PAGE_MASK;

    IOMemoryDescriptor *vDesc = IOMemoryDescriptor::withAddressRange((mach_vm_address_t)va, (len + voff + PAGE_MASK) & ~(uint64_t)PAGE_MASK, direction == kIODirectionIn ? kIODirectionOut : kIODirectionIn, current_task());
    if(!vDesc)
    {
        retval = iokit_vendor_specific_err(1);
    }
    else
    {
        ret = vDesc->prepare();
        if(ret != kIOReturnSuccess)
        {
            retval = ret;
        }
        else
        {
            IOMemoryMap *vMap = vDesc->map();
            if(!vMap)
            {
                retval = iokit_vendor_specific_err(2);
            }
            else
            {
                IOMemoryDescriptor *pDesc = IOMemoryDescriptor::withPhysicalAddress((IOPhysicalAddress)pa, (len + poff + PAGE_MASK) & ~(uint64_t)PAGE_MASK, direction);
                if(!pDesc)
                {
                    retval = iokit_vendor_specific_err(3);
                }
                else
                {
                    IOMemoryMap *pMap = pDesc->map(mapOptions);
                    if(!pMap)
                    {
                        retval = iokit_vendor_specific_err(4);
                    }
                    else
                    {
                        IOVirtualAddress v = vMap->getVirtualAddress();
                        IOVirtualAddress p = pMap->getVirtualAddress();
                        const void *from = (const void*)(direction == kIODirectionIn ? p + poff : v + voff);
                              void *to   = (      void*)(direction == kIODirectionIn ? v + voff : p + poff);

                        switch(alignment)
                        {
                            case 0:
                                bcopy(from, to, len);
                                break;

                            case 4:
                                for(size_t i = 0; i < len/4; ++i)
                                {
                                    ((volatile uint32_t*)to)[i] = ((const volatile uint32_t*)from)[i];
                                }
                                break;

                            case 8:
                                for(size_t i = 0; i < len/8; ++i)
                                {
                                    ((volatile uint64_t*)to)[i] = ((const volatile uint64_t*)from)[i];
                                }
                                break;
                        }

                        retval = kIOReturnSuccess;

                        pMap->release();
                    }
                    pDesc->release();
                }
                vMap->release();
            }
            vDesc->complete();
        }
        vDesc->release();
    }

    return retval;
}

IOReturn IOKernelRWUserClient::readPhys(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args)
{
    return physcopy(args->scalarInput[0], args->scalarInput[1], args->scalarInput[2], args->scalarInput[3], kIODirectionIn);
}

IOReturn IOKernelRWUserClient::writePhys(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args)
{
    return physcopy(args->scalarInput[0], args->scalarInput[1], args->scalarInput[2], args->scalarInput[3], kIODirectionOut);
}

IOReturn IOKernelRWUserClient::getStrchr(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args)
{
    typedef __typeof__ (&strchr) strchr_fptr_t;
    strchr_fptr_t strchr_fptr_signed = &strchr;
    const void *strchr_fptr_signed_void = (const void *)strchr_fptr_signed;
#if !__has_feature(ptrauth_calls)
    const void *strchr_fptr_unsigned_void = (const void *)strchr_fptr_signed_void;
#else
    const void *strchr_fptr_unsigned_void = (const void *)ptrauth_strip(strchr_fptr_signed_void, ptrauth_key_function_pointer);
#endif
    int r1 = copyout((const void*)&strchr_fptr_signed_void, (user_addr_t)args->scalarInput[0], sizeof(strchr_fptr_signed_void));
    int r2 = copyout((const void*)&strchr_fptr_unsigned_void, (user_addr_t)args->scalarInput[1], sizeof(strchr_fptr_unsigned_void));
    return ((r1 == 0) && (r2 == 0)) ? kIOReturnSuccess : kIOReturnVMError;
}

IOReturn IOKernelRWUserClient::getkOSBooleanTrue(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args)
{
    typedef __typeof__ (&kOSBooleanTrue) kOSBooleanTrue_ptr_t;
    kOSBooleanTrue_ptr_t kOSBooleanTrue_ptr_signed = &kOSBooleanTrue;
    const void *kOSBooleanTrue_ptr_signed_void = (const void *)kOSBooleanTrue_ptr_signed;
#if !__has_feature(ptrauth_calls)
    const void *kOSBooleanTrue_ptr_unsigned_void = (const void *)kOSBooleanTrue_ptr_signed_void;
#else
    const void *kOSBooleanTrue_ptr_unsigned_void = (const void *)ptrauth_strip(kOSBooleanTrue_ptr_signed_void,   ptrauth_key_process_independent_data);
#endif
    int r1 = copyout((const void*)&kOSBooleanTrue_ptr_signed_void, (user_addr_t)args->scalarInput[0], sizeof(kOSBooleanTrue_ptr_signed_void));
    int r2 = copyout((const void*)&kOSBooleanTrue_ptr_unsigned_void, (user_addr_t)args->scalarInput[1], sizeof(kOSBooleanTrue_ptr_unsigned_void));
    return ((r1 == 0) && (r2 == 0)) ? kIOReturnSuccess : kIOReturnVMError;
}
