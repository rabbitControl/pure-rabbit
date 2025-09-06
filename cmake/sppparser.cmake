set(RCP_SPPP sppp)

pd_add_external(${RCP_SPPP} "sppp.cpp")

target_link_libraries(${RCP_SPPP} PRIVATE rcpc)

if (WIN32)
  target_link_libraries(${RCP_SPPP} PRIVATE wsock32 ws2_32)
endif()
