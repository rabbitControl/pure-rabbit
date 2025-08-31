set(RCP_PARSE rabbit.parse)

set(RCP_PARSE_SOURCES
  rabbit.parse.h rabbit.parse.cpp
  RcpParse.h RcpParse.cpp
  PdMaxUtils.h
)

pd_add_external(${RCP_PARSE} "${RCP_PARSE_SOURCES}")

target_link_libraries(${RCP_PARSE} rcpc)
