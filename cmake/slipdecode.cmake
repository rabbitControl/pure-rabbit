set(SLIP_DECODER slipdecoder)

pd_add_external(${SLIP_DECODER} "slipdecoder.cpp")

target_link_libraries(${SLIP_DECODER} rcpc)

if (WIN32)
  target_link_libraries(${SLIP_DECODER} PRIVATE wsock32 ws2_32)
endif()
