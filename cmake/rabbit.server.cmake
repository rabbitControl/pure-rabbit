set(RCP_SERVER rabbit.server)

set(RCP_SERVER_SOURCES
  rabbit.server.h rabbit.server.cpp
  PdRcp.h PdRcp.cpp
  RabbitHoleServerTransporter.h RabbitHoleServerTransporter.cpp
  WebsocketServerTransporter.h WebsocketServerTransporter.cpp
  ParameterServerClientBase.h ParameterServerClientBase.cpp
  ParameterServer.h ParameterServer.cpp
  PdMaxUtils.h
  Threading.h Threading.cpp
  IServerTransporter.h
  PdServerTransporter.h PdServerTransporter.cpp
)

pd_add_external(${RCP_SERVER} "${RCP_SERVER_SOURCES}")

scaryws_setup_target(${RCP_SERVER})
target_link_libraries(${RCP_SERVER} PRIVATE scaryws)
target_link_libraries(${RCP_SERVER} PRIVATE rcpc)
