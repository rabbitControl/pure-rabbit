set(SIZE_PREFIX sizeprefix)

pd_add_external(${SIZE_PREFIX} "sizeprefix.cpp")

if (WIN32)
  target_link_libraries(${RCP_SPPP} PRIVATE wsock32 ws2_32)
endif()
