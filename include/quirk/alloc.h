#ifndef D660F735_424B_44C2_A69C_F3319E2C2282
#define D660F735_424B_44C2_A69C_F3319E2C2282

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef void *quirk_aligned_alloc(size_t alignment, size_t size);
typedef void quirk_aligned_free(void* ptr, size_t alignment, size_t size);

#if defined(_MSC_VER )
    #define QUIRK_ALIGNOF(TYPE) __alignof(TYPE)
#elif defined(__GNUC__)
    #define QUIRK_ALIGNOF(TYPE) __alignof__(TYPE)
#else
    #define QUIRK_ALIGNOF(TYPE) offsetof(struct { char c; TYPE x; }, x)
#endif

#if defined(__cplusplus)
}
#endif

#endif /* D660F735_424B_44C2_A69C_F3319E2C2282 */
