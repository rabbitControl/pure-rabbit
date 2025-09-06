set(SLIP_ENCODER slipencoder)

pd_add_external(${SLIP_ENCODER} "slipencoder.cpp")

target_link_libraries(${SLIP_ENCODER} rcpc)

if (WIN32)
  target_link_libraries(${SLIP_ENCODER} PRIVATE wsock32 ws2_32)
endif()
