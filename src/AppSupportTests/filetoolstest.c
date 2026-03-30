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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AppSupport/filetools.h"
#include "AppSupport/testutils.h"

int main() {
    testInit("FileTools Path Extraction Tests");

    // Test basename
    ASSERT_STR_EQ("bar.txt", basename("foo/bar.txt"));
    ASSERT_STR_EQ("bar.txt", basename("bar.txt"));
    ASSERT_STR_EQ("bar", basename("foo/bar/"));
    ASSERT_STR_EQ("bar", basename("foo/bar"));
    ASSERT_STR_EQ("bar.txt", basename("work:foo/bar.txt"));
    ASSERT_STR_EQ("bar.txt", basename("work:bar.txt"));
    ASSERT_STR_EQ("work:", basename("work:"));
    ASSERT_STR_EQ("", basename(""));
    ASSERT_STR_EQ(".", basename("."));
    ASSERT_STR_EQ("/", basename("/"));

    // Test dirname
    ASSERT_STR_EQ("foo", dirname("foo/bar.txt"));
    ASSERT_STR_EQ("", dirname("bar.txt"));
    ASSERT_STR_EQ("foo", dirname("foo/bar/"));
    ASSERT_STR_EQ("foo", dirname("foo/bar"));
    ASSERT_STR_EQ("work:foo", dirname("work:foo/bar.txt"));
    ASSERT_STR_EQ("work:", dirname("work:bar.txt"));
    ASSERT_STR_EQ("work:", dirname("work:"));
    ASSERT_STR_EQ("", dirname(""));
    ASSERT_STR_EQ("", dirname("."));
    ASSERT_STR_EQ("/", dirname("/"));

    return testSummary();
}
