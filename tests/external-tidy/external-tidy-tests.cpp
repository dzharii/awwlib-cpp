#include "doctest/doctest.h"
#include <string>

// Wrap the C headers in extern "C" to ensure C linkage in C++.
extern "C" {
#include <tidy.h>
#include <tidybuffio.h>
#include <tidyenum.h>
}

TEST_CASE("Tidy: Clean malformed HTML and produce valid output") {
    // Create a new Tidy document.
    TidyDoc tdoc = tidyCreate();
    REQUIRE(tdoc != nullptr);

    // Configure Tidy options.
    // Force output so that Tidy attempts to repair the document.
    tidyOptSetBool(tdoc, TidyForceOutput, yes);
    // Quiet the output to avoid spurious warnings.
    tidyOptSetBool(tdoc, TidyQuiet, yes);
    // Disable warnings if not needed.
    tidyOptSetBool(tdoc, TidyShowWarnings, no);

    // Example of malformed HTML.
    const char* inputHTML = "<html><body><p>Unclosed paragraph";
    
    // Parse the input string.
    int err = tidyParseString(tdoc, inputHTML);
    CHECK(err >= 0);  // Ensure parsing did not return a fatal error.

    // Clean and repair the document.
    err = tidyCleanAndRepair(tdoc);
    CHECK(err >= 0);

    // Run additional diagnostics if needed.
    err = tidyRunDiagnostics(tdoc);
    CHECK(err >= 0);

    // Save the cleaned-up HTML to a buffer.
    TidyBuffer output = {0};
    err = tidySaveBuffer(tdoc, &output);
    CHECK(err >= 0);

    // Convert the output buffer to a std::string for easier verification.
    std::string cleanedHTML(reinterpret_cast<const char*>(output.bp), output.size);
    // Check that the output now contains a proper closing tag for the paragraph.
    CHECK(cleanedHTML.find("</p>") != std::string::npos);

    // Clean up resources.
    tidyBufFree(&output);
    tidyRelease(tdoc);
}

TEST_CASE("Tidy: Check option behavior and error buffering") {
    TidyDoc tdoc = tidyCreate();
    REQUIRE(tdoc != nullptr);

    // Enable error buffering.
    tidyOptSetBool(tdoc, TidyForceOutput, yes);
    tidyOptSetBool(tdoc, TidyQuiet, yes);

    // Allocate an error buffer and tell Tidy to use it.
    TidyBuffer errbuf = {0};
    tidySetErrorBuffer(tdoc, &errbuf);

    // Input HTML with a known error.
    const char* inputHTML = "<html><body><p>Missing closing tags";
    int err = tidyParseString(tdoc, inputHTML);
    CHECK(err >= 0);
    
    // Clean and repair the document.
    err = tidyCleanAndRepair(tdoc);
    CHECK(err >= 0);
    err = tidyRunDiagnostics(tdoc);
    CHECK(err >= 0);

    // Convert the error buffer to a string and check that errors were reported.
    std::string errors(reinterpret_cast<const char*>(errbuf.bp), errbuf.size);
    CHECK(!errors.empty());

    tidyBufFree(&errbuf);
    tidyRelease(tdoc);
}

TEST_CASE("Tidy: Verify indentation and wrapping options") {
    TidyDoc tdoc = tidyCreate();
    REQUIRE(tdoc != nullptr);

    tidyOptSetBool(tdoc, TidyForceOutput, yes);
    tidyOptSetBool(tdoc, TidyQuiet, yes);
    tidyOptSetBool(tdoc, TidyShowWarnings, no);
    // Set options for indenting and wrapping.
    tidyOptSetBool(tdoc, TidyIndentContent, yes);
    tidyOptSetInt(tdoc, TidyWrapLen, 80);

    const char* inputHTML = "<html><body><p>Some very long text that should be wrapped according to the wrap length specified in the options.</p></body></html>";
    int err = tidyParseString(tdoc, inputHTML);
    CHECK(err >= 0);
    err = tidyCleanAndRepair(tdoc);
    CHECK(err >= 0);
    err = tidyRunDiagnostics(tdoc);
    CHECK(err >= 0);

    TidyBuffer output = {0};
    err = tidySaveBuffer(tdoc, &output);
    CHECK(err >= 0);

    std::string cleanedHTML(reinterpret_cast<const char*>(output.bp), output.size);
    // Verify that the output contains newlines or breaks to indicate wrapping.
    CHECK(cleanedHTML.find("\n") != std::string::npos);

    tidyBufFree(&output);
    tidyRelease(tdoc);
}


