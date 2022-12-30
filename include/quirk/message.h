#ifndef D4613980_2B93_4898_A95B_88A38D7E046A
#define D4613980_2B93_4898_A95B_88A38D7E046A

#include <quirk/alloc.h>
#include <quirk/strview.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct quirk_msg;
typedef struct quirk_msg quirk_msg;

enum quirk_msg_error_kind
{
    quirk_msg_error_success,
    quirk_msg_error_need_more_data,
    quirk_msg_error_invalid_tag,
    quirk_msg_error_invalid_source,
    quirk_msg_error_invalid_command,
    quirk_msg_error_invalid_parameter,
    quirk_msg_error_invalid_delimiter,
    quirk_msg_error_unexpected_end,
    quirk_msg_error_bad_state
};

typedef enum quirk_msg_error_kind quirk_msg_error_kind;

struct quirk_msg_callbacks
{
    void (*tag)(void *data, quirk_strview vendor, quirk_strview key, quirk_strview value);
    void (*untrusted_tag)(void *data, quirk_strview vendor, quirk_strview key, quirk_strview value);
    void (*server_source)(void *data, quirk_strview source);
    void (*user_source)(void *data, quirk_strview nickname, quirk_strview user, quirk_strview host);
    void (*command)(void *data, quirk_strview command);
    void (*parameter)(void *data, quirk_strview parameter);
};

typedef struct quirk_msg_callbacks quirk_msg_callbacks;

enum quirk_msg_error_kind quirk_msg_parse(quirk_msg *msg, unsigned char *begin, unsigned char *end);
quirk_msg *quirk_msg_create(quirk_aligned_alloc *alloc_fn);
void quirk_msg_destroy(quirk_msg *msg, quirk_aligned_free *free_fn);
unsigned char *quirk_msg_get_error_message(quirk_msg_error_kind error);
unsigned char *quirk_msg_get_position(quirk_msg *msg);
void quirk_msg_reset(quirk_msg *msg);
void quirk_msg_set_callbacks(quirk_msg *msg, quirk_msg_callbacks *callabcks, void *data);

#if defined(__cplusplus)
} // extern "C"
#endif


#endif /* D4613980_2B93_4898_A95B_88A38D7E046A */
