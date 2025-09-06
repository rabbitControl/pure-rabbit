set(RCP_FORMAT rabbit.format)

set(RCP_FORMAT_SOURCES
  rabbit.format.h rabbit.format.cpp
  RcpFormat.h RcpFormat.cpp
  PdMaxUtils.h
)

pd_add_external(${RCP_FORMAT} "${RCP_FORMAT_SOURCES}")

target_link_libraries(${RCP_FORMAT} PRIVATE rcpc)

if (WIN32)
  target_link_libraries(${RCP_FORMAT} PRIVATE wsock32 ws2_32)
endif()
