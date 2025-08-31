set(SLIP_ENCODER slipencoder)

pd_add_external(${SLIP_ENCODER} "slipencoder.cpp")

target_link_libraries(${SLIP_ENCODER} rcpc)
