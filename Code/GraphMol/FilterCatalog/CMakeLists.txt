if(NOT RDK_USE_BOOST_SERIALIZATION)
    message("== Making FilterCatalog without boost Serialization support")
endif()

find_package(Python3 COMPONENTS Interpreter)
if (Python3::Interpreter)
        message("== Updating Filters.cpp from pains file")
        execute_process(
          COMMAND
          ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/update_pains.py
          RESULT_VARIABLE rc
        )
        if(NOT rc EQUAL 0)
          message(FATAL_ERROR "Failed updating Filters.cpp from wehi_pains.csv")
        endif(NOT rc EQUAL 0)
endif()


rdkit_library(FilterCatalog
              Filters.cpp
              FilterCatalog.cpp
              FilterCatalogEntry.cpp
	      FilterCatalogRunner.cpp
              FilterMatchers.cpp
              FunctionalGroupHierarchy.cpp
              LINK_LIBRARIES Subgraphs SubstructMatch SmilesParse
              GraphMol Catalogs)
target_compile_definitions(FilterCatalog PRIVATE RDKIT_FILTERCATALOG_BUILD)

rdkit_headers(FilterCatalogEntry.h
              FilterCatalog.h
              FilterMatcherBase.h
              FilterMatchers.h
              FunctionalGroupHierarchy.h
              DEST GraphMol/FilterCatalog)

if(RDK_BUILD_PYTHON_WRAPPERS)
add_subdirectory(Wrap)
endif()

rdkit_test(filterCatalogTest filtercatalogtest.cpp
           LINK_LIBRARIES FilterCatalog SmilesParse FileParsers )
