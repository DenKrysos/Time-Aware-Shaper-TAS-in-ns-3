set(base_examples
    tsn-example1
    tsn-example2
    tsn-example3
    tsn-example4
    tsn-example5
)

foreach(
  example
  ${base_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK ${libtsn}
                      ${libcore}
                      ${libcsma}
                      ${libpoint-to-point}
                      ${libinternet}
                      ${libapplications}
  )
endforeach()

