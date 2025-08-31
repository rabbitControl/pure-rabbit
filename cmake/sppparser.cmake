set(RCP_SPPP sppp)

pd_add_external(${RCP_SPPP} "sppp.c")

target_link_libraries(${RCP_SPPP} rcpc)
