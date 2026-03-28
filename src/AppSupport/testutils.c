// Copyright (c) 2026 André Gewert <agewert@ubergeek.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "testutils.h"
#include <stdio.h>

static int totalTests = 0;
static int passedTests = 0;
static const char* suiteName = "Unknown";

void testInit(const char* testSuiteName) {
    suiteName = testSuiteName;
    totalTests = 0;
    passedTests = 0;
    shellPrintf(SHELL_FG_BRIGHT_BLUE, "Starting test suite: %s\n", suiteName);
}

void testLog(bool success, const char* expression, const char* file, int line) {
    totalTests++;
    if (success) {
        passedTests++;
        printf("  [");
        shellSetColor(SHELL_FG_GREEN);
        printf("PASS");
        shellResetColor();
        printf("] %s\n", expression);
    } else {
        printf("  [");
        shellSetColor(SHELL_FG_RED);
        printf("FAIL");
        shellResetColor();
        printf("] %s at %s:%d\n", expression, file, line);
    }
}

int testSummary(void) {
    printf("\n");
    shellPrintf(SHELL_COLOR_BOLD, "Test Summary for %s:\n", suiteName);
    printf("  Total tests:  %d\n", totalTests);
    printf("  Passed:       %d\n", passedTests);
    printf("  Failed:       %d\n", totalTests - passedTests);
    
    if (totalTests == passedTests) {
        shellPrintf(SHELL_FG_BRIGHT_GREEN, "ALL TESTS PASSED\n");
    } else {
        shellPrintf(SHELL_FG_BRIGHT_RED, "SOME TESTS FAILED\n");
    }
    shellResetColor();
    
    return totalTests - passedTests;
}
