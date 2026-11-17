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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"
#include "../cJSON_Utils.h"

/* a malloc implementation that allows the first `malloc_fail_after` allocations to succeed
 * and fails all allocations afterwards. Used to test allocation failure paths. */
static size_t malloc_allocation_count = 0;
static size_t malloc_fail_after = (size_t)-1;

static void *CJSON_CDECL counting_malloc(size_t size)
{
    if (malloc_allocation_count >= malloc_fail_after)
    {
        return NULL;
    }

    malloc_allocation_count++;
    return malloc(size);
}

/* work around MSVC error C2322: '...' address of dllimport '...' is not static */
static void CJSON_CDECL normal_free(void *pointer)
{
    free(pointer);
}

static void check_generate_patches_with_failing_allocator(const char *from_json, const char *to_json, size_t max_allocations)
{
    cJSON *from = cJSON_Parse(from_json);
    cJSON *to = cJSON_Parse(to_json);
    size_t fail_at = 0;

    TEST_ASSERT_NOT_NULL(from);
    TEST_ASSERT_NOT_NULL(to);

    /* try failing every single allocation inside cJSONUtils_GeneratePatches.
     * the function must not crash regardless of which allocation fails. */
    for (fail_at = 0; fail_at <= max_allocations; fail_at++)
    {
        cJSON_Hooks hooks = { counting_malloc, normal_free };
        cJSON *patches = NULL;

        malloc_allocation_count = 0;
        malloc_fail_after = fail_at;
        cJSON_InitHooks(&hooks);

        /* must not crash, return value may be NULL or a (possibly partial) patch array */
        patches = cJSONUtils_GeneratePatches(from, to);

        cJSON_InitHooks(NULL);

        if (patches != NULL)
        {
            cJSON_Delete(patches);
        }
    }

    cJSON_Delete(from);
    cJSON_Delete(to);
}

static void cjson_utils_functions_shouldnt_crash_with_null_pointers(void)
{
    cJSON *item = cJSON_CreateString("item");
    TEST_ASSERT_NOT_NULL(item);

    TEST_ASSERT_NULL(cJSONUtils_GetPointer(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GetPointer(NULL, "pointer"));
    TEST_ASSERT_NULL(cJSONUtils_GetPointerCaseSensitive(NULL, "pointer"));
    TEST_ASSERT_NULL(cJSONUtils_GetPointerCaseSensitive(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatches(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatches(NULL, item));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatchesCaseSensitive(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatchesCaseSensitive(NULL, item));
    cJSONUtils_AddPatchToArray(item, "path", "add", NULL);
    cJSONUtils_AddPatchToArray(item, "path", NULL, item);
    cJSONUtils_AddPatchToArray(item, NULL, "add", item);
    cJSONUtils_AddPatchToArray(NULL, "path", "add", item);
    cJSONUtils_ApplyPatches(item, NULL);
    cJSONUtils_ApplyPatches(NULL, item);
    cJSONUtils_ApplyPatchesCaseSensitive(item, NULL);
    cJSONUtils_ApplyPatchesCaseSensitive(NULL, item);
    TEST_ASSERT_NULL(cJSONUtils_MergePatch(item, NULL));
    item = cJSON_CreateString("item");
    TEST_ASSERT_NULL(cJSONUtils_MergePatchCaseSensitive(item, NULL));
    item = cJSON_CreateString("item");
    /* these calls are actually valid */
    /* cJSONUtils_MergePatch(NULL, item); */
    /* cJSONUtils_MergePatchCaseSensitive(NULL, item);*/
    /* cJSONUtils_GenerateMergePatch(item, NULL); */
    /* cJSONUtils_GenerateMergePatch(NULL, item); */
    /* cJSONUtils_GenerateMergePatchCaseSensitive(item, NULL); */
    /* cJSONUtils_GenerateMergePatchCaseSensitive(NULL, item); */

    TEST_ASSERT_NULL(cJSONUtils_FindPointerFromObjectTo(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_FindPointerFromObjectTo(NULL, item));
    cJSONUtils_SortObject(NULL);
    cJSONUtils_SortObjectCaseSensitive(NULL);

    cJSON_Delete(item);
}

static void cjson_utils_generate_patches_shouldnt_crash_on_allocation_failure(void)
{
    /* object with an added key, exercises the compose_patch "suffix" path */
    check_generate_patches_with_failing_allocator("{\"a\":1}", "{\"a\":1,\"b\":2}", 40);
    /* object with a removed key, exercises the compose_patch \"remove\" path */
    check_generate_patches_with_failing_allocator("{\"a\":1,\"b\":2}", "{\"a\":1}", 40);
    /* array with an appended element, exercises the array branch of create_patches */
    check_generate_patches_with_failing_allocator("[1]", "[1,2]", 40);
    /* nested object, exercises the recursive new_path allocation */
    check_generate_patches_with_failing_allocator("{\"a\":{\"x\":1}}", "{\"a\":{\"x\":1,\"y\":2}}", 40);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(cjson_utils_functions_shouldnt_crash_with_null_pointers);
    RUN_TEST(cjson_utils_generate_patches_shouldnt_crash_on_allocation_failure);

    return UNITY_END();
}
