add_executable(
  pipe_valve_closed
  pipe_valve_closed.cpp
)

add_executable(
  pump_pipe
  pump_pipe.cpp
)

target_link_libraries(
  pipe_valve_closed
  PRIVATE
  lvtrans_core
)

target_link_libraries(
  pump_pipe
  PRIVATE
  lvtrans_core
)
