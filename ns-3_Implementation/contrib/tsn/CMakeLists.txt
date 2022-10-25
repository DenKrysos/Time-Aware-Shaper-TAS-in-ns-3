build_lib(
  LIBNAME tsn
  SOURCE_FILES
    model/transmisson-gate-qdisc.cc
    model/net-device-list-config.cc
    model/TsnIpv4PacketFilter.cc
    model/TsnIpv6PacketFilter.cc
    model/tas-queue-disc.cc
    helper/tsn-helper.cc
    test/Ipv4Filter.cc
    test/tas-test.cc
  HEADER_FILES
    model/transmisson-gate-qdisc.h
    model/net-device-list-config.h
    model/TsnIpv4PacketFilter.h
    model/TsnIpv6PacketFilter.h
    model/tas-queue-disc.h
    helper/tsn-helper.h
    test/Ipv4Filter.h
  LIBRARIES_TO_LINK ${libcore}
                    ${libtraffic-control}
  TEST_SOURCES test/tsn-test-suite.cc
)
