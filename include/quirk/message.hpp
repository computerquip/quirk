#ifndef AB93BB04_3AB3_4F9D_ABB1_48FFA6498911
#define AB93BB04_3AB3_4F9D_ABB1_48FFA6498911

#include "message.h"

namespace irc
{

namespace msg
{

using msg_view = irc_msg_view;
using callbacks = irc_msg_callbacks;
using error_kind = irc_msg_error_kind;

};

};

irc::msg::msg_view operator "" _msg_view(const char *buf, size_t len)
{
    return {
        reinterpret_cast<const unsigned char *>(buf),
        reinterpret_cast<const unsigned char *>(buf + len)
    };
}


#endif /* AB93BB04_3AB3_4F9D_ABB1_48FFA6498911 */
