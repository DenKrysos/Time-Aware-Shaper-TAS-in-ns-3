set(source_files
    helper/tsn-helper.cc
    model/transmisson-gate-qdisc.cc
    model/net-device-list-config.cc
    model/TsnIpv4PacketFilter.cc
    model/TsnIpv6PacketFilter.cc
    model/tas-queue-disc.cc
)

set(header_files
    model/transmisson-gate-qdisc.h
    model/net-device-list-config.h
    model/TsnIpv4PacketFilter.h
    model/TsnIpv6PacketFilter.h
    model/tas-queue-disc.h
    helper/tsn-helper.h
    test/Ipv4Filter.h
)

set(test_sources
    test/Ipv4Filter.cc
    test/tas-test.cc
    test/tsn-test-suite.cc
)

build_lib(
  LIBNAME tsn
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK ${libcore}
                    ${libtraffic-control}
                    ${libconfig-store}
  TEST_SOURCES ${test_sources}
)
