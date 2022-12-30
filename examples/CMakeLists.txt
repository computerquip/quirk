add_executable(example-parse-msg parse-msg.c)
target_link_libraries(example-parse-msg PRIVATE quirk-message)
