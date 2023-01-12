# What is this?

It's (eventually) going to be yet another IRC library. I've made an IRC library in the past but it wasn't quite what I wanted. I also never used it. It doesn't handle message tags or large message at all.

# Interop with C and C++
Currently, I plan to make everything interface with C and then wrap it in C++ in a header so either can be used fairly seamlessly.

# Current state
Currently, only the parser is implemented. It uses re2c to generate the actual parser. The parser supports the following:

- Continuations. You can stop parsing and resume at a later time (see limitations).
- Doesn't require buffering.
- Supports UTF-8 depending on how it's generated with re2c.

Limitations:

- While continuation of lexing is supported, it has the limitation that new data must be contiguously appended to the old data. This is to prevent the requirement of buffering lexemes in-between.
- It likely will struggle to parse hostnames used by various networks as they don't use standard hostnames.
- It doesn't support IPv6 addresses yet.
- Requires bounds checks. This could be removed in the future depending on how it's used.