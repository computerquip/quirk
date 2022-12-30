#include <quirk/message.h>
#include <stddef.h>

/*
 * - Doesn't support IPv6 yet.
 * - Requires a contiguous buffer in continuations. Otherwise,
 *   the parser would need to hold an 8KiB buffer to handle tags.
 * - This could prevent bounds check if the caller assures that the
 *   message has an ending "\r" or "\n". It currently doesn't.
 */

/*!conditions:re2c*/

struct quirk_msg
{
    quirk_strview buf;
    char condition, state;
    unsigned char yych, yyaccept;
    unsigned char *marker;

/*!stags:re2c format = '    unsigned char *@@;\n'; */

    quirk_msg_callbacks *callbacks;
    void *user_data;
    int untrusted_tag;

    union {
        struct {
            quirk_strview vendor;
            quirk_strview key;
        } tag;
    } views;
};

typedef void tag_fn(void *data, quirk_strview vendor, quirk_strview key, quirk_strview value);

quirk_strview view_from_stags(unsigned char *tag1, unsigned char *tag2)
{
    quirk_strview result = { tag1, tag2 };
    return result;
}

static void do_parameter_callback(
  quirk_msg *msg,
  unsigned char *t1,
  unsigned char *t2
) {
    if (msg->callbacks->parameter) {
        msg->callbacks->parameter(msg->user_data, view_from_stags(t1, t2));
    }
}

static void do_tag_callback(
  quirk_msg *msg,
  unsigned char *t1,
  unsigned char *t2
) {
    tag_fn *callback = msg->callbacks->tag;
    quirk_strview value = view_from_stags(t1, t2);

    if (msg->untrusted_tag)
        callback = msg->callbacks->untrusted_tag;

    if (callback) {
        callback(msg->user_data,
            msg->views.tag.vendor,
            msg->views.tag.key,
            value);
    }
}

static void secure_zeroize(void *data, size_t data_size)
{
    volatile unsigned char *volatile_data = data;

    while (data_size--){
        *volatile_data++ = 0;
    }
}

quirk_msg *quirk_msg_create(quirk_aligned_alloc *alloc_fn)
{
    quirk_msg *msg = alloc_fn(QUIRK_ALIGNOF(quirk_msg), sizeof(quirk_msg));

    if (msg == NULL) {
        return msg;
    }

    secure_zeroize(msg, sizeof *msg);
    msg->state = -1;

    return msg;
}

void quirk_msg_destroy(quirk_msg *msg, quirk_aligned_free *free_fn)
{
    secure_zeroize(msg, sizeof *msg);
    free_fn(msg, QUIRK_ALIGNOF(quirk_msg), sizeof(quirk_msg));
}

void quirk_msg_set_callbacks(
  quirk_msg *msg,
  quirk_msg_callbacks *callbacks,
  void *data
) {
    msg->callbacks = callbacks;
    msg->user_data = data;
}

void quirk_msg_reset(quirk_msg *msg)
{
    quirk_msg_callbacks *callbacks = msg->callbacks;
    void *user_data = msg->user_data;

    secure_zeroize(msg, sizeof *msg);

    msg->callbacks = callbacks;
    msg->user_data = user_data;
}

unsigned char *quirk_msg_get_position(quirk_msg *msg)
{
    return msg->buf.begin;
}

unsigned char *quirk_msg_get_error_message(quirk_msg_error_kind error)
{
    static unsigned char success[] = "success";
    static unsigned char need_more_data[] = "need more data";
    static unsigned char invalid_tag[] = "invalid tag";
    static unsigned char invalid_source[] = "invalid source";
    static unsigned char invalid_command[] = "invalid command";
    static unsigned char invalid_parameter[] = "invalid parameter";
    static unsigned char invalid_delimiter[] = "invalid delimiter";
    static unsigned char unexpected_end[] = "unexpected end";
    static unsigned char bad_state[] = "bad state";
    static unsigned char unknown_error[] = "unknown";

    switch (error) {
        case quirk_msg_error_success: return success;
        case quirk_msg_error_need_more_data: return need_more_data;
        case quirk_msg_error_invalid_tag: return invalid_tag;
        case quirk_msg_error_invalid_source: return invalid_source;
        case quirk_msg_error_invalid_command: return invalid_command;
        case quirk_msg_error_invalid_parameter: return invalid_parameter;
        case quirk_msg_error_invalid_delimiter: return invalid_delimiter;
        case quirk_msg_error_unexpected_end: return unexpected_end;
        case quirk_msg_error_bad_state: return bad_state;
    }

    return unknown_error;
}

/* There are various ambiguous*/
quirk_msg_error_kind quirk_msg_parse(
  quirk_msg *msg,
  unsigned char *begin,
  unsigned char *end
) {
    unsigned char *t1, *t2, *t3, *t4, *t5, *t6;

    msg->buf.begin = begin;
    msg->buf.end = end;
    /*!getstate:re2c */

/*!re2c
    re2c:api:style             = free-form;
    re2c:api                   = custom;
    re2c:tags                  = 1;
    re2c:indent:string         = "    ";
    re2c:tags:expression       = "msg->@@";
    re2c:define:YYFILL         = "return quirk_msg_error_need_more_data;";
    re2c:define:YYCTYPE        = "unsigned char";
    re2c:define:YYLIMIT        = "msg->buf.end";
    re2c:define:YYLESSTHAN     = "msg->buf.begin >= msg->buf.end";
    re2c:define:YYPEEK         = "*msg->buf.begin";
    re2c:define:YYSKIP         = "++msg->buf.begin;";
    re2c:define:YYSTAGP        = "@@{tag} = msg->buf.begin;";
    re2c:define:YYSTAGN        = "@@{tag} = NULL;";
    re2c:define:YYSHIFT        = "msg->buf.begin += @@{shift};";
    re2c:define:YYSHIFTSTAG    = "@@{tag} += @@{shift};";
    re2c:define:YYBACKUP       = "msg->marker = msg->buf.begin;";
    re2c:define:YYRESTORE      = "msg->buf.begin = msg->marker;";
    re2c:define:YYGETSTATE     = "msg->state";
    re2c:define:YYSETSTATE     = "msg->state = @@;";
    re2c:define:YYGETCONDITION = "msg->condition";
    re2c:define:YYSETCONDITION = "msg->condition = @@;";
    re2c:variable:yych           = "msg->yych";
    re2c:variable:yyaccept       = "msg->yyaccept";

    digit      = [0-9];
    digits     = digit+;
    alpha      = [a-zA-Z];
    alnum      = digit | alpha;
    letter     = alpha;
    space      = " ";
    crlf       = "\r\n" | "\r" | "\n";
    nospcrlfcl = [^\x00\r\n: ];
    trailing   = ( ":" | space | nospcrlfcl )*;
    middle     = nospcrlfcl ( ":" | nospcrlfcl )*;
    nickname   = [A-}] [-A-}0-9];
    user       = [\x21-\x39\x41-\x7E];
    key        = ( alpha | digit | "-" )+;
    value      = [^\x00\r\n; ]+;

    shortname  = alnum ( alnum | "-" )* ( alnum );
    hostaddr   = digits{1,3} "." digits{1,3} "." digits{1,3} "." digits{1,3};
    hostname   = shortname ( "." shortname )*;
    host       = hostname | hostaddr;
    servername = hostname;
    vendor     = host;

    command_numeric = digit{3};
    command_name    = letter+;
    command         = command_numeric | command_name;

    <init> "@" :=> tag_prefix
    <init> "" :=> source

    <tags> space+ :=> source_prefix
    <tags> ";"    :=> tag_prefix
    <tags> ""     { return quirk_msg_error_invalid_tag; }

    <tag_prefix> "+" => tag {
        msg->untrusted_tag = 1;
        goto yyc_tag;
    }
    <tag_prefix> "" :=> tag

    <tag> ( @t1 vendor @t2 "/" )? @t3 key @t4 => tag_value {
        msg->views.tag.vendor = view_from_stags(t1, t2);
        msg->views.tag.key = view_from_stags(t3, t4);
        t1 = t4;
        t2 = t4;
        goto yyc_tag_value;
    }
    <tag> "" { return quirk_msg_error_invalid_tag; }

    <tag_value> "=" @t1 value @t2 => tags {
        do_tag_callback(msg, t1, t2);
        goto yyc_tags;
    }
    <tag_value> "=" @t1 @t2       => tags {
        do_tag_callback(msg, t1, t2);
        goto yyc_tags;
    }
    <tag_value> ""               :=> tags

    <source_prefix> ":" :=> source
    <source_prefix> ""  :=> command

    <source> @t1 servername @t2 space+ => command {
        if (msg->callbacks->server_source) {
            msg->callbacks->server_source(
                msg->user_data,
                view_from_stags(t1, t2));
        }

        goto yyc_command;
    }
    <source> @t1 nickname @t2 "!"
             @t3 user @t4 "@"
             @t5 [^\x00\r\n ]+ @t6 space+ => command
    {
        if (msg->callbacks->user_source) {
            msg->callbacks->user_source(
                msg->user_data,
                view_from_stags(t1, t2),
                view_from_stags(t3, t4),
                view_from_stags(t5, t6));
        }

        goto yyc_command;
    }
    <source> "" { return quirk_msg_error_invalid_source; }

    <command> @t1 command @t2 => parameters {
        if (msg->callbacks->command) {
            msg->callbacks->command(msg->user_data, view_from_stags(t1, t2));
        }

        goto yyc_parameters;
    }
    <command> "" { return quirk_msg_error_invalid_command; }

    <parameters> space+ :=> parameter
    <parameters> "" :=> delimiter

    <parameter> ":" @t1 trailing @t2 => delimiter {
        do_parameter_callback(msg, t1, t2);
        goto yyc_delimiter;
    }
    <parameter> @t1 middle @t2 => parameters {
        do_parameter_callback(msg, t1, t2);
        goto yyc_parameters;
    }
    <parameter> "" { return quirk_msg_error_invalid_parameter; }

    <delimiter> crlf { return quirk_msg_error_success; }
    <delimiter> "" { return quirk_msg_error_invalid_delimiter; }
*/

    return quirk_msg_error_bad_state;
}
