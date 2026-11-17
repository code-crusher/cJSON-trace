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

static void parse_with_opts_should_handle_null(void)
{
    const char *error_pointer = NULL;
    cJSON *item = NULL;
    TEST_ASSERT_NULL_MESSAGE(cJSON_ParseWithOpts(NULL, &error_pointer, false), "Failed to handle NULL input.");
    item = cJSON_ParseWithOpts("{}", NULL, false);
    TEST_ASSERT_NOT_NULL_MESSAGE(item, "Failed to handle NULL error pointer.");
    cJSON_Delete(item);
    TEST_ASSERT_NULL_MESSAGE(cJSON_ParseWithOpts(NULL, NULL, false), "Failed to handle both NULL.");
    TEST_ASSERT_NULL_MESSAGE(cJSON_ParseWithOpts("{", NULL, false), "Failed to handle NULL error pointer with parse error.");
}

static void parse_with_opts_should_handle_empty_strings(void)
{
    const char empty_string[] = "";
    const char *error_pointer = NULL;

    TEST_ASSERT_NULL(cJSON_ParseWithOpts(empty_string, NULL, false));
    TEST_ASSERT_EQUAL_PTR(empty_string, cJSON_GetErrorPtr());

    TEST_ASSERT_NULL(cJSON_ParseWithOpts(empty_string, &error_pointer, false));
    TEST_ASSERT_EQUAL_PTR(empty_string, error_pointer);
    TEST_ASSERT_EQUAL_PTR(empty_string, cJSON_GetErrorPtr());
}

static void parse_with_opts_should_handle_incomplete_json(void)
{
    const char json[] = "{ \"name\": ";
    const char *parse_end = NULL;

    TEST_ASSERT_NULL(cJSON_ParseWithOpts(json, &parse_end, false));
    TEST_ASSERT_EQUAL_PTR(json + strlen(json), parse_end);
    TEST_ASSERT_EQUAL_PTR(json + strlen(json), cJSON_GetErrorPtr());
}

static void parse_with_opts_should_require_null_if_requested(void)
{
    cJSON *item = cJSON_ParseWithOpts("{}", NULL, true);
    TEST_ASSERT_NOT_NULL(item);
    cJSON_Delete(item);
    item = cJSON_ParseWithOpts("{} \n", NULL, true);
    TEST_ASSERT_NOT_NULL(item);
    cJSON_Delete(item);
    TEST_ASSERT_NULL(cJSON_ParseWithOpts("{}x", NULL, true));
}

static void parse_with_opts_should_return_parse_end(void)
{
    const char json[] = "[] empty array XD";
    const char *parse_end = NULL;

    cJSON *item = cJSON_ParseWithOpts(json, &parse_end, false);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_EQUAL_PTR(json + 2, parse_end);
    cJSON_Delete(item);
}

static void parse_with_opts_should_parse_utf8_bom(void)
{
    cJSON *with_bom = NULL;
    cJSON *without_bom = NULL;

    with_bom = cJSON_ParseWithOpts("\xEF\xBB\xBF{}", NULL, true);
    TEST_ASSERT_NOT_NULL(with_bom);
    without_bom = cJSON_ParseWithOpts("{}", NULL, true);
    TEST_ASSERT_NOT_NULL(with_bom);

    TEST_ASSERT_TRUE(cJSON_Compare(with_bom, without_bom, true));

    cJSON_Delete(with_bom);
    cJSON_Delete(without_bom);
}

static void parse_with_opts_should_only_accept_json_whitespace(void)
{
    /* JSON whitespace is only space, tab, LF and CR. Other control bytes must not be skipped. */
    const unsigned char other_whitespace[][8] = {
        { '[', 0x00, '1', ']', '\0' },
        { '[', 0x01, '1', ']', '\0' },
        { '[', 0x02, '1', ']', '\0' },
        { '[', 0x0b, '1', ']', '\0' },
        { '[', 0x0c, '1', ']', '\0' },
        { '[', 0x1f, '1', ']', '\0' },
        { '[', 0x7f, '1', ']', '\0' }
    };
    size_t i = 0;

    for (i = 0; i < sizeof(other_whitespace) / sizeof(other_whitespace[0]); i++)
    {
        const unsigned char *input = other_whitespace[i];
        size_t length = sizeof(other_whitespace[i]);
        cJSON *item = cJSON_ParseWithLengthOpts((const char *)input, length, NULL, true);
        TEST_ASSERT_NULL_MESSAGE(item, "Control byte was accepted as whitespace.");
        cJSON_Delete(item);
    }
}

static void parse_with_opts_should_accept_json_whitespace(void)
{
    const unsigned char valid_whitespace[][8] = {
        { '[', ' ',  '1', ']', '\0' },
        { '[', '\t', '1', ']', '\0' },
        { '[', '\n', '1', ']', '\0' },
        { '[', '\r', '1', ']', '\0' }
    };
    size_t i = 0;

    for (i = 0; i < sizeof(valid_whitespace) / sizeof(valid_whitespace[0]); i++)
    {
        const unsigned char *input = valid_whitespace[i];
        size_t length = sizeof(valid_whitespace[i]);
        cJSON *item = cJSON_ParseWithLengthOpts((const char *)input, length, NULL, true);
        TEST_ASSERT_NOT_NULL_MESSAGE(item, "Valid JSON whitespace was rejected.");
        cJSON_Delete(item);
    }
}

static void parse_with_opts_should_not_skip_null_literal_as_whitespace(void)
{
    const unsigned char input[] = { 0x00, 'n', 'u', 'l', 'l', '\0' };
    cJSON *item = cJSON_ParseWithLengthOpts((const char *)input, sizeof(input), NULL, true);
    TEST_ASSERT_NULL_MESSAGE(item, "Leading NUL byte was accepted as whitespace.");
    cJSON_Delete(item);
}

int CJSON_CDECL main(void)
{
    UNITY_BEGIN();

    RUN_TEST(parse_with_opts_should_handle_null);
    RUN_TEST(parse_with_opts_should_handle_empty_strings);
    RUN_TEST(parse_with_opts_should_handle_incomplete_json);
    RUN_TEST(parse_with_opts_should_require_null_if_requested);
    RUN_TEST(parse_with_opts_should_return_parse_end);
    RUN_TEST(parse_with_opts_should_parse_utf8_bom);
    RUN_TEST(parse_with_opts_should_only_accept_json_whitespace);
    RUN_TEST(parse_with_opts_should_accept_json_whitespace);
    RUN_TEST(parse_with_opts_should_not_skip_null_literal_as_whitespace);

    return UNITY_END();
}
