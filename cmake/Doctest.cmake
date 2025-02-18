if(ENABLE_DOCTESTS)
    add_definitions(-DENABLE_DOCTEST_IN_LIBRARY)
    include(FetchContent)
    FetchContent_Declare(
            DocTest
            GIT_REPOSITORY "https://github.com/onqtam/doctest"
            GIT_TAG "ae7a13539fb71f270b87eb2e874fbac80bc8dda2"
    )

    FetchContent_MakeAvailable(DocTest)
    include_directories(${DOCTEST_INCLUDE_DIR})
endif()
