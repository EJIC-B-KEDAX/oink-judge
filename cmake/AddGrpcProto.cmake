function(add_oink_grpc_proto MODULE_NAME PROTO_FILE)
  cmake_parse_arguments(ARG "NO_PYTHON" "" "" ${ARGN})

  set(PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE})
  get_filename_component(PROTO_BASENAME ${PROTO_FILE} NAME_WE)

  set(GEN_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/oink_judge/${PROTO_BASENAME})
  set(PB_CC ${GEN_PREFIX}.pb.cc)
  set(GRPC_PB_CC ${GEN_PREFIX}.grpc.pb.cc)
  set(PB_H ${GEN_PREFIX}.pb.h)
  set(GRPC_PB_H ${GEN_PREFIX}.grpc.pb.h)

  set(PROTOC_COMMAND protoc)
  set(PROTOC_DEPENDS)
  if(TARGET protobuf::protoc)
    set(PROTOC_COMMAND $<TARGET_FILE:protobuf::protoc>)
    list(APPEND PROTOC_DEPENDS protobuf::protoc)
  endif()

  add_custom_command(
    OUTPUT ${PB_CC} ${GRPC_PB_CC} ${PB_H} ${GRPC_PB_H}
    COMMAND
      ${PROTOC_COMMAND} -I ${CMAKE_CURRENT_SOURCE_DIR}
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
      --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
      ${PROTO_PATH}
    DEPENDS ${PROTO_PATH} ${PROTOC_DEPENDS} gRPC::grpc_cpp_plugin
    VERBATIM)

  if(NOT ARG_NO_PYTHON AND OINK_JUDGE_GRPC_TOOLS_AVAILABLE EQUAL 0)
    set(PYTHON_PROTO_STAMP
        ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_BASENAME}.python.stamp)
    add_custom_command(
      OUTPUT ${PYTHON_PROTO_STAMP}
      COMMAND
        ${Python3_EXECUTABLE} -m grpc_tools.protoc -I
        ${CMAKE_CURRENT_SOURCE_DIR}
        --python_out=${OINK_JUDGE_PYTHON_PROTO_OUT_DIR}
        --grpc_python_out=${OINK_JUDGE_PYTHON_PROTO_OUT_DIR}
        --mypy_out=${OINK_JUDGE_PYTHON_PROTO_OUT_DIR} ${PROTO_PATH}
      COMMAND ${CMAKE_COMMAND} -E touch ${PYTHON_PROTO_STAMP}
      DEPENDS ${PROTO_PATH}
      VERBATIM)

    add_custom_target(generate_${MODULE_NAME}_python_proto
                      DEPENDS ${PYTHON_PROTO_STAMP})
  elseif(NOT ARG_NO_PYTHON)
    message(
      WARNING
        "grpc_tools or mypy_protobuf not found; skipping Python proto generation for ${MODULE_NAME}"
    )
  endif()

  add_library(oink_judge_${MODULE_NAME}_proto_headers INTERFACE ${PB_H}
              ${GRPC_PB_H})
  target_include_directories(
    oink_judge_${MODULE_NAME}_proto_headers
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
              $<INSTALL_INTERFACE:include>)
  target_link_libraries(oink_judge_${MODULE_NAME}_proto_headers
                        INTERFACE protobuf::libprotobuf gRPC::grpc++)

  add_library(oink_judge_${MODULE_NAME}_proto SHARED ${PB_CC} ${GRPC_PB_CC})
  add_library(oink_judge::${MODULE_NAME}_proto ALIAS
              oink_judge_${MODULE_NAME}_proto)
  target_link_libraries(
    oink_judge_${MODULE_NAME}_proto
    PUBLIC oink_judge_${MODULE_NAME}_proto_headers protobuf::libprotobuf
           gRPC::grpc++)

  if(TARGET generate_${MODULE_NAME}_python_proto)
    add_dependencies(oink_judge_${MODULE_NAME}_proto
                     generate_${MODULE_NAME}_python_proto)
  endif()
endfunction()
