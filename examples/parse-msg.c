#include <quirk/message.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define VIEW_PF(VIEW) \
    ((int)((VIEW).end - (VIEW).begin)), (VIEW).begin

static void print_tag(
  void *data,
  struct irc_msg_view vendor,
  struct irc_msg_view key,
  struct irc_msg_view value
) {
    printf("\ttag: %.*s/%.*s=%.*s\n",
        VIEW_PF(vendor),
        VIEW_PF(key),
        VIEW_PF(value));
}

static void print_untrusted_tag(
  void *data,
  struct irc_msg_view vendor,
  struct irc_msg_view key,
  struct irc_msg_view value
) {
    printf("\tuntrusted tag: %.*s/%.*s=%.*s\n",
        VIEW_PF(vendor),
        VIEW_PF(key),
        VIEW_PF(value));
}

static void print_server_source(void *data, struct irc_msg_view source)
{
    printf("\tserver source: %.*s\n", VIEW_PF(source));
}

static void print_user_source(
  void *data,
  struct irc_msg_view nickname,
  struct irc_msg_view user,
  struct irc_msg_view host
) {
    printf("\tuser source: %.*s!%.*s@%.*s\n",
        VIEW_PF(nickname),
        VIEW_PF(user),
        VIEW_PF(host));
}

static void print_command(void *data, struct irc_msg_view command)
{
    printf("\tcommand: %.*s\n", VIEW_PF(command));
}

static void print_parameter(void *data, struct irc_msg_view parameter)
{
    printf("\tparameter: %.*s\n", VIEW_PF(parameter));
}

static void print_whitespace(size_t n)
{
    for (; n > 0; --n) {
        printf(" ");
    }
}

static struct irc_msg_callbacks print_callbacks = {
    print_untrusted_tag,
    print_tag,
    print_server_source,
    print_user_source,
    print_command,
    print_parameter
};

void *custom_aligned_alloc(size_t alignment, size_t size)
{
    return malloc(size);
}

void custom_aligned_free(void *data, size_t alignment, size_t size)
{
    free(data);
}

int main(void)
{
    size_t old_index = 0;
    size_t index = 0;
    enum irc_msg_error_kind error = irc_msg_error_success;
    const unsigned char *error_idx = NULL;

    /* All structures are the same size when given the same size arguments.
     * Normally, you would split up memory into pools of irc_msg and buffers.
     * Here, we just increment the index of the memory range. */

    /* This would generally be in a buffer allocated somewhere in memory_range
     * but we're cheating here for testing purposes. */
    static unsigned char basic_test[] = ""
        "@time=2022-12-27T22:55:28.784Z;+bobby=sally "
        ":calcium.libera.chat "
        "PONG calcium.libera.chat :LAG13894\r\n";

    static size_t basic_test_size = sizeof basic_test / sizeof basic_test[0] - 1;
    static unsigned char *basic_test_end = &basic_test[sizeof basic_test / sizeof basic_test[0] - 1];

    static unsigned char err_test[] = ""
        "@time=2022-12-27T22:55:28.784Z;+bo$bby=sally "
        ":calcium.libera.chat "
        "PONG calcium.libera.chat :LAG13894\r\n";

    static size_t err_test_size = sizeof err_test / sizeof err_test[0] - 1;
    static unsigned char *err_test_end =
        &err_test[sizeof err_test / sizeof err_test[0] - 1];

    struct irc_msg *msg = irc_msg_create(custom_aligned_alloc);
    assert(msg != NULL);

    irc_msg_set_callbacks(msg, &print_callbacks, NULL);

    printf("test 1:\n");

    error = irc_msg_parse(
        msg,
        basic_test,
        basic_test_end - 6);

    assert(error == irc_msg_error_need_more_data);

    printf("\tjust taking a break!\n");

    error = irc_msg_parse(msg, basic_test_end - 5, basic_test_end);
    assert(error == irc_msg_error_success);

    /* We can just reuse the message. reset will keep context data and
     * callbacks intact. */
    irc_msg_reset(msg);

    /* I CUT THAT TAG IN HALF
     * The parser will see that we're partially out of data midway
     * through a tag which is dandy but this shows that we can process
     * The previous tag before reaching the tags delimiter " ". */
    printf("\ntest 2:\n");

    error = irc_msg_parse(
        msg,
        basic_test,
        basic_test_end - basic_test_size + 40);

    assert(error == irc_msg_error_need_more_data);

    printf("\tjust taking a break!\n");

    error = irc_msg_parse(msg, basic_test + 41, basic_test_end);
    assert(error == irc_msg_error_success);

    irc_msg_reset(msg);

    /* USEFUL ERROR MESSAGES!
     * Error messages aren't usually that useful in IRC. It's not
     * like the user is going to care generally. But this is a
     * full-fledged parser and it's useful to developers (maybe).
     *
     * There's an error below in the tags. See if you can spot it. */
    printf("\ntest 3:\n");

    error = irc_msg_parse(msg, err_test, err_test_end);
    assert(error == irc_msg_error_invalid_tag);

    /* This may not work on some consoles due to wrapping.
     * Expand your console for this or disable wrapping. */
    error_idx = irc_msg_get_position(msg);
    printf("Error at char %d\n", (int)(error_idx - err_test));
    printf("%.*s\n", (int)(err_test_end - err_test), err_test);
    print_whitespace(error_idx - err_test);
    printf("^\n");
    print_whitespace(error_idx - err_test);

    /* Note that we could return UTF-8 here but don't just because.
     * It's more "correct" to feed this to a UTF-8 capable API but
     * realistically, we're probably never going to put non-ASCII
     * in the error message strings. */
    printf("%s\n", irc_msg_get_error_message(error));

    irc_msg_destroy(msg, custom_aligned_free);
}