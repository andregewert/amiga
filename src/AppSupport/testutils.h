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

#ifndef APPSUPPORT_TESTUTILS_H
#define APPSUPPORT_TESTUTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "shell.h"

/**
 * Initializes the testing framework.
 * @param testSuiteName The name of the test suite being run.
 */
void testInit(const char* testSuiteName);

/**
 * Prints a summary of the test results and returns the number of failures.
 * @return Number of failed tests.
 */
int testSummary(void);

/**
 * Logs a test result.
 * @param success Whether the test passed or failed.
 * @param expression The expression that was tested.
 * @param file The file where the test was performed.
 * @param line The line number where the test was performed.
 */
void testLog(bool success, const char* expression, const char* file, int line);

#define ASSERT_TRUE(expr) testLog((expr), #expr, __FILE__, __LINE__)
#define ASSERT_FALSE(expr) testLog(!(expr), "!(" #expr ")", __FILE__, __LINE__)

#define ASSERT_INT_EQ(expected, actual) \
    do { \
        int _expected = (expected); \
        int _actual = (actual); \
        bool _success = (_expected == _actual); \
        if (!_success) { \
            shellPrintf(SHELL_FG_RED, "  FAILED: %d == %d (expected == actual)\n", _expected, _actual); \
        } \
        testLog(_success, #expected " == " #actual, __FILE__, __LINE__); \
    } while (0)

#define ASSERT_STR_EQ(expected, actual) \
    do { \
        const char* _expected = (expected); \
        const char* _actual = (actual); \
        bool _success = (strcmp(_expected, _actual) == 0); \
        if (!_success) { \
            shellPrintf(SHELL_FG_RED, "  FAILED: \"%s\" == \"%s\" (expected == actual)\n", _expected, _actual); \
        } \
        testLog(_success, "strcmp(" #expected ", " #actual ") == 0", __FILE__, __LINE__); \
    } while (0)

#define ASSERT_NOT_NULL(ptr) testLog((ptr) != NULL, #ptr " != NULL", __FILE__, __LINE__)
#define ASSERT_NULL(ptr) testLog((ptr) == NULL, #ptr " == NULL", __FILE__, __LINE__)

#endif // APPSUPPORT_TESTUTILS_H
