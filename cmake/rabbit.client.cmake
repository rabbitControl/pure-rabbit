set(RCP_CLIENT rabbit.client)

set(RCP_CLIENT_SOURCES
  rabbit.client.h rabbit.client.cpp
  PdRcp.h PdRcp.cpp
  WebsocketClientTransporter.h WebsocketClientTransporter.cpp
  ParameterServerClientBase.h ParameterServerClientBase.cpp
  ParameterClient.h ParameterClient.cpp
  PdMaxUtils.h
  Threading.h Threading.cpp
  IClientTransporter.h
  PdClientTransporter.h PdClientTransporter.cpp
)

pd_add_external(${RCP_CLIENT} "${RCP_CLIENT_SOURCES}")

scaryws_setup_target(${RCP_CLIENT})
target_link_libraries(${RCP_CLIENT} scaryws)
target_link_libraries(${RCP_CLIENT} rcpc)

