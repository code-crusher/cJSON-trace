/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"

static cJSON_bool compare_from_string(const char * const a, const char * const b, const cJSON_bool case_sensitive)
{
    cJSON *a_json = NULL;
    cJSON *b_json = NULL;
    cJSON_bool result = false;

    a_json = cJSON_Parse(a);
    TEST_ASSERT_NOT_NULL_MESSAGE(a_json, "Failed to parse a.");
    b_json = cJSON_Parse(b);
    TEST_ASSERT_NOT_NULL_MESSAGE(b_json, "Failed to parse b.");

    result = cJSON_Compare(a_json, b_json, case_sensitive);

    cJSON_Delete(a_json);
    cJSON_Delete(b_json);

    return result;
}

static void cjson_compare_should_compare_null_pointer_as_not_equal(void)
{
    TEST_ASSERT_FALSE(cJSON_Compare(NULL, NULL, true));
    TEST_ASSERT_FALSE(cJSON_Compare(NULL, NULL, false));
}

static void cjson_compare_should_compare_invalid_as_not_equal(void)
{
    cJSON invalid[1];
    memset(invalid, '\0', sizeof(invalid));

    TEST_ASSERT_FALSE(cJSON_Compare(invalid, invalid, false));
    TEST_ASSERT_FALSE(cJSON_Compare(invalid, invalid, true));
}

static void cjson_compare_should_compare_numbers(void)
{
    TEST_ASSERT_TRUE(compare_from_string("1", "1", true));
    TEST_ASSERT_TRUE(compare_from_string("1", "1", false));
    TEST_ASSERT_TRUE(compare_from_string("0.0001", "0.0001", true));
    TEST_ASSERT_TRUE(compare_from_string("0.0001", "0.0001", false));
    TEST_ASSERT_TRUE(compare_from_string("1E100", "10E99", false));

    TEST_ASSERT_FALSE(compare_from_string("0.5E-100", "0.5E-101", false));

    TEST_ASSERT_FALSE(compare_from_string("1", "2", true));
    TEST_ASSERT_FALSE(compare_from_string("1", "2", false));
}

static void cjson_compare_should_compare_booleans(void)
{
    /* true */
    TEST_ASSERT_TRUE(compare_from_string("true", "true", true));
    TEST_ASSERT_TRUE(compare_from_string("true", "true", false));

    /* false */
    TEST_ASSERT_TRUE(compare_from_string("false", "false", true));
    TEST_ASSERT_TRUE(compare_from_string("false", "false", false));

    /* mixed */
    TEST_ASSERT_FALSE(compare_from_string("true", "false", true));
    TEST_ASSERT_FALSE(compare_from_string("true", "false", false));
    TEST_ASSERT_FALSE(compare_from_string("false", "true", true));
    TEST_ASSERT_FALSE(compare_from_string("false", "true", false));
}

static void cjson_compare_should_compare_null(void)
{
    TEST_ASSERT_TRUE(compare_from_string("null", "null", true));
    TEST_ASSERT_TRUE(compare_from_string("null", "null", false));

    TEST_ASSERT_FALSE(compare_from_string("null", "true", true));
    TEST_ASSERT_FALSE(compare_from_string("null", "true", false));
}

static void cjson_compare_should_not_accept_invalid_types(void)
{
    cJSON invalid[1];
    memset(invalid, '\0', sizeof(invalid));

    invalid->type = cJSON_Number | cJSON_String;

    TEST_ASSERT_FALSE(cJSON_Compare(invalid, invalid, true));
    TEST_ASSERT_FALSE(cJSON_Compare(invalid, invalid, false));
}

static void cjson_compare_should_compare_strings(void)
{
    TEST_ASSERT_TRUE(compare_from_string("\"abcdefg\"", "\"abcdefg\"", true));
    TEST_ASSERT_TRUE(compare_from_string("\"abcdefg\"", "\"abcdefg\"", false));

    TEST_ASSERT_FALSE(compare_from_string("\"ABCDEFG\"", "\"abcdefg\"", true));
    TEST_ASSERT_FALSE(compare_from_string("\"ABCDEFG\"", "\"abcdefg\"", false));
}

static void cjson_compare_should_compare_raw(void)
{
    cJSON *raw1 = NULL;
    cJSON *raw2 = NULL;

    raw1 = cJSON_Parse("\"[true, false]\"");
    TEST_ASSERT_NOT_NULL(raw1);
    raw2 = cJSON_Parse("\"[true, false]\"");
    TEST_ASSERT_NOT_NULL(raw2);

    raw1->type = cJSON_Raw;
    raw2->type = cJSON_Raw;

    TEST_ASSERT_TRUE(cJSON_Compare(raw1, raw2, true));
    TEST_ASSERT_TRUE(cJSON_Compare(raw1, raw2, false));

    cJSON_Delete(raw1);
    cJSON_Delete(raw2);
}

static void cjson_compare_should_compare_arrays(void)
{
    TEST_ASSERT_TRUE(compare_from_string("[]", "[]", true));
    TEST_ASSERT_TRUE(compare_from_string("[]", "[]", false));

    TEST_ASSERT_TRUE(compare_from_string("[false,true,null,42,\"string\",[],{}]", "[false, true, null, 42, \"string\", [], {}]", true));
    TEST_ASSERT_TRUE(compare_from_string("[false,true,null,42,\"string\",[],{}]", "[false, true, null, 42, \"string\", [], {}]", false));

    TEST_ASSERT_TRUE(compare_from_string("[[[1], 2]]", "[[[1], 2]]", true));
    TEST_ASSERT_TRUE(compare_from_string("[[[1], 2]]", "[[[1], 2]]", false));

    TEST_ASSERT_FALSE(compare_from_string("[true,null,42,\"string\",[],{}]", "[false, true, null, 42, \"string\", [], {}]", true));
    TEST_ASSERT_FALSE(compare_from_string("[true,null,42,\"string\",[],{}]", "[false, true, null, 42, \"string\", [], {}]", false));

    /* Arrays that are a prefix of another array */
    TEST_ASSERT_FALSE(compare_from_string("[1,2,3]", "[1,2]", true));
    TEST_ASSERT_FALSE(compare_from_string("[1,2,3]", "[1,2]", false));
}

static void cjson_compare_should_compare_objects(void)
{
    TEST_ASSERT_TRUE(compare_from_string("{}", "{}", true));
    TEST_ASSERT_TRUE(compare_from_string("{}", "{}", false));

    TEST_ASSERT_TRUE(compare_from_string(
                "{\"false\": false, \"true\": true, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                "{\"true\": true, \"false\": false, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                true));
    TEST_ASSERT_FALSE(compare_from_string(
                "{\"False\": false, \"true\": true, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                "{\"true\": true, \"false\": false, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                true));
    TEST_ASSERT_TRUE(compare_from_string(
                "{\"False\": false, \"true\": true, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                "{\"true\": true, \"false\": false, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                false));
    TEST_ASSERT_FALSE(compare_from_string(
                "{\"Flse\": false, \"true\": true, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                "{\"true\": true, \"false\": false, \"null\": null, \"number\": 42, \"string\": \"string\", \"array\": [], \"object\": {}}",
                false));
    /* test objects that are a subset of each other */
    TEST_ASSERT_FALSE(compare_from_string(
                "{\"one\": 1, \"two\": 2}",
                "{\"one\": 1, \"two\": 2, \"three\": 3}",
                true))
    TEST_ASSERT_FALSE(compare_from_string(
                "{\"one\": 1, \"two\": 2}",
                "{\"one\": 1, \"two\": 2, \"three\": 3}",
                false))
}

static void cjson_compare_should_compare_nested_objects(void)
{
    /* Member order must not matter at any depth. */
    TEST_ASSERT_TRUE(compare_from_string(
                "{\"b\": {\"d\": 4, \"c\": 3}, \"a\": [1, 2]}",
                "{\"a\": [1, 2], \"b\": {\"c\": 3, \"d\": 4}}",
                true));
    TEST_ASSERT_TRUE(compare_from_string(
                "{\"B\": 1, \"a\": 2}",
                "{\"b\": 1, \"A\": 2}",
                false));

    /* Differences below the top level are still detected. */
    TEST_ASSERT_FALSE(compare_from_string(
                "{\"b\": {\"d\": 4, \"c\": 3}, \"a\": [1, 2]}",
                "{\"a\": [1, 2], \"b\": {\"c\": 3, \"d\": 5}}",
                true));

    /* Objects with a different number of members are never equal. */
    TEST_ASSERT_FALSE(compare_from_string("{\"a\": 1}", "{\"a\": 1, \"b\": 2}", true));
    TEST_ASSERT_FALSE(compare_from_string("{\"a\": 1, \"b\": 2}", "{\"a\": 1}", false));
}

static cJSON *create_nested_object(size_t depth)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *current = root;
    size_t i = 0;

    if (root == NULL)
    {
        return NULL;
    }

    for (i = 0; i < depth; i++)
    {
        cJSON *child = cJSON_CreateObject();
        if (child == NULL)
        {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToObject(current, "child", child);
        current = child;
    }

    if (cJSON_AddNumberToObject(current, "value", 1) == NULL)
    {
        cJSON_Delete(root);
        return NULL;
    }

    return root;
}

static cJSON *create_nested_array(size_t depth)
{
    cJSON *root = cJSON_CreateArray();
    cJSON *current = root;
    size_t i = 0;

    if (root == NULL)
    {
        return NULL;
    }

    for (i = 0; i < depth; i++)
    {
        cJSON *child = cJSON_CreateArray();
        if (child == NULL)
        {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToArray(current, child);
        current = child;
    }

    if (cJSON_AddItemToArray(current, cJSON_CreateNumber(1)) == false)
    {
        cJSON_Delete(root);
        return NULL;
    }

    return root;
}

static void cjson_compare_should_compare_wide_objects_in_reasonable_time(void)
{
    /* CVE-2026-67216: comparing objects used to look every member up in the
     * other object, which is O(n^2) per level. Nesting multiplied that into
     * exponential work, so this comparison used to take minutes. It must now
     * finish quickly (and, of course, report equality). */
    cJSON *a = create_nested_object(25);
    cJSON *b = create_nested_object(25);
    size_t i = 0;

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    for (i = 0; i < 30; i++)
    {
        char name[16];
        sprintf(name, "member%02u", (unsigned int)i);
        TEST_ASSERT_NOT_NULL(cJSON_AddNumberToObject(a, name, (double)i));
        TEST_ASSERT_NOT_NULL(cJSON_AddNumberToObject(b, name, (double)i));
    }

    TEST_ASSERT_TRUE(cJSON_Compare(a, b, true));
    TEST_ASSERT_TRUE(cJSON_Compare(a, b, false));

    cJSON_Delete(a);
    cJSON_Delete(b);
}

static void cjson_compare_should_not_reorder_the_compared_objects(void)
{
    cJSON *a = cJSON_Parse("{\"z\": 1, \"a\": 2, \"m\": 3}");
    cJSON *b = cJSON_Parse("{\"m\": 3, \"z\": 1, \"a\": 2}");
    cJSON *first = NULL;

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    first = a->child;
    TEST_ASSERT_TRUE(cJSON_Compare(a, b, true));
    TEST_ASSERT_TRUE(cJSON_Compare(a, b, false));

    /* Comparing must not mutate the caller's objects. */
    TEST_ASSERT_EQUAL_PTR(first, a->child);
    TEST_ASSERT_EQUAL_STRING("z", a->child->string);
    TEST_ASSERT_EQUAL_STRING("a", a->child->next->string);
    TEST_ASSERT_EQUAL_STRING("m", a->child->next->next->string);

    cJSON_Delete(a);
    cJSON_Delete(b);
}

static void cjson_compare_should_fail_safe_past_nesting_limit(void)
{
    /* CVE-2026-67215: trees can be built through the API (or by grafting
     * patch values) deeper than the parser would ever accept. Comparison
     * must stop at the nesting limit instead of exhausting the stack. */
    cJSON *arrays_a = create_nested_array(CJSON_NESTING_LIMIT + 500);
    cJSON *arrays_b = create_nested_array(CJSON_NESTING_LIMIT + 500);
    cJSON *objects_a = create_nested_object(CJSON_NESTING_LIMIT + 500);
    cJSON *objects_b = create_nested_object(CJSON_NESTING_LIMIT + 500);

    TEST_ASSERT_NOT_NULL(arrays_a);
    TEST_ASSERT_NOT_NULL(arrays_b);
    TEST_ASSERT_NOT_NULL(objects_a);
    TEST_ASSERT_NOT_NULL(objects_b);

    TEST_ASSERT_FALSE(cJSON_Compare(arrays_a, arrays_b, true));
    TEST_ASSERT_FALSE(cJSON_Compare(objects_a, objects_b, true));

    cJSON_Delete(arrays_a);
    cJSON_Delete(arrays_b);
    cJSON_Delete(objects_a);
    cJSON_Delete(objects_b);
}

static void cjson_compare_should_handle_trees_at_the_nesting_limit(void)
{
    /* Documents the parser accepts must still compare equal. */
    cJSON *a = create_nested_object(CJSON_NESTING_LIMIT - 1);
    cJSON *b = create_nested_object(CJSON_NESTING_LIMIT - 1);

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    TEST_ASSERT_TRUE(cJSON_Compare(a, b, true));

    cJSON_Delete(a);
    cJSON_Delete(b);
}

int CJSON_CDECL main(void)
{
    UNITY_BEGIN();

    RUN_TEST(cjson_compare_should_compare_null_pointer_as_not_equal);
    RUN_TEST(cjson_compare_should_compare_invalid_as_not_equal);
    RUN_TEST(cjson_compare_should_compare_numbers);
    RUN_TEST(cjson_compare_should_compare_booleans);
    RUN_TEST(cjson_compare_should_compare_null);
    RUN_TEST(cjson_compare_should_not_accept_invalid_types);
    RUN_TEST(cjson_compare_should_compare_strings);
    RUN_TEST(cjson_compare_should_compare_raw);
    RUN_TEST(cjson_compare_should_compare_arrays);
    RUN_TEST(cjson_compare_should_compare_objects);
    RUN_TEST(cjson_compare_should_compare_nested_objects);
    RUN_TEST(cjson_compare_should_compare_wide_objects_in_reasonable_time);
    RUN_TEST(cjson_compare_should_not_reorder_the_compared_objects);
    RUN_TEST(cjson_compare_should_fail_safe_past_nesting_limit);
    RUN_TEST(cjson_compare_should_handle_trees_at_the_nesting_limit);

    return UNITY_END();
}
