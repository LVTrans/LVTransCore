set(
  LVTRANS_EXAMPLES
  pipe_valve_closed
  pump_pipe
  pump_startup
  reservoir_pipe_valve
  reservoir_pipe_valve_old
)

foreach(example IN LISTS LVTRANS_EXAMPLES)
  add_executable(
    ${example}
    ${example}.cpp
  )

  target_link_libraries(
    ${example}
    PRIVATE
    lvtrans_core
  )

  target_compile_options(${example} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endforeach()
