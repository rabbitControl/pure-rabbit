set(SLIP_DECODER slipdecoder)

pd_add_external(${SLIP_DECODER} "slipdecoder.c")

target_link_libraries(${SLIP_DECODER} rcpc)
