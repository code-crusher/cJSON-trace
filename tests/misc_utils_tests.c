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

static void * CJSON_CDECL failing_malloc(size_t size)
{
    (void)size;
    return NULL;
}

/* work around MSVC error C2322: '...' address of dllimport '...' is not static */
static void CJSON_CDECL normal_free(void *pointer)
{
    free(pointer);
}

static cJSON_Hooks failing_hooks = {
    failing_malloc,
    normal_free
};

static void * CJSON_CDECL failing_path_malloc(size_t size)
{
    /* the empty path returned when the target is found is allocated as a
     * 1 byte string ('', plus the null terminator), any other buffer used
     * to assemble the pointer path is bigger than that */
    if (size > 1)
    {
        return NULL;
    }
    return malloc(size);
}

static cJSON_Hooks failing_path_hooks = {
    failing_path_malloc,
    normal_free
};

static void find_pointer_should_fail_gracefully_on_allocation_failure(void)
{
    cJSON *root = NULL;
    cJSON *target = NULL;

    /* array path: the full_pointer of "/0" fails to allocate */
    root = cJSON_CreateArray();
    TEST_ASSERT_NOT_NULL(root);
    target = cJSON_CreateNumber(42);
    TEST_ASSERT_NOT_NULL(target);
    TEST_ASSERT_TRUE(cJSON_AddItemToArray(root, target));

    TEST_ASSERT_EQUAL_STRING("/0", cJSONUtils_FindPointerFromObjectTo(root, target));
    free(cJSONUtils_FindPointerFromObjectTo(root, target));

    cJSON_InitHooks(&failing_path_hooks);
    TEST_ASSERT_NULL(cJSONUtils_FindPointerFromObjectTo(root, target));
    cJSON_InitHooks(NULL);

    cJSON_Delete(root);

    /* object path: the full_pointer of "/key" fails to allocate */
    root = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(root);
    target = cJSON_CreateNumber(42);
    TEST_ASSERT_NOT_NULL(target);
    TEST_ASSERT_TRUE(cJSON_AddItemToObject(root, "key", target));

    TEST_ASSERT_EQUAL_STRING("/key", cJSONUtils_FindPointerFromObjectTo(root, target));
    free(cJSONUtils_FindPointerFromObjectTo(root, target));

    cJSON_InitHooks(&failing_path_hooks);
    TEST_ASSERT_NULL(cJSONUtils_FindPointerFromObjectTo(root, target));
    cJSON_InitHooks(NULL);

    cJSON_Delete(root);

    /* every allocation fails, including the empty path itself */
    cJSON_InitHooks(&failing_hooks);
    TEST_ASSERT_NULL(cJSONUtils_FindPointerFromObjectTo(root, NULL));
    cJSON_InitHooks(NULL);
}

static void generate_patches_should_fail_gracefully_on_allocation_failure(void)
{
    cJSON *from = NULL;
    cJSON *to = NULL;
    cJSON *patches = NULL;

    /* nested objects and arrays, so all of the path allocations in
     * compose_patch and create_patches run at least once */
    from = cJSON_Parse("{\"key\":[1,2],\"other\":1}");
    TEST_ASSERT_NOT_NULL(from);
    to = cJSON_Parse("{\"key\":[1,3,4],\"new\":2}");
    TEST_ASSERT_NOT_NULL(to);

    /* the patches are generated successfully with the default allocator */
    cJSON_InitHooks(&failing_hooks);
    patches = cJSONUtils_GeneratePatches(from, to);
    cJSON_InitHooks(NULL);
    TEST_ASSERT_NULL(patches);

    cJSON_Delete(from);
    cJSON_Delete(to);
}

static void add_patch_to_array_should_fail_gracefully_on_allocation_failure(void)
{
    cJSON *array = NULL;
    cJSON *value = NULL;

    array = cJSON_CreateArray();
    TEST_ASSERT_NOT_NULL(array);
    value = cJSON_CreateNumber(42);
    TEST_ASSERT_NOT_NULL(value);

    /* the full_path of the patch is assembled with cJSON_malloc */
    cJSON_InitHooks(&failing_path_hooks);
    cJSONUtils_AddPatchToArray(array, "add", "root", value);
    cJSON_InitHooks(NULL);
    TEST_ASSERT_EQUAL_INT(0, cJSON_GetArraySize(array));

    cJSON_Delete(array);
    cJSON_Delete(value);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(cjson_utils_functions_shouldnt_crash_with_null_pointers);
    RUN_TEST(find_pointer_should_fail_gracefully_on_allocation_failure);
    RUN_TEST(generate_patches_should_fail_gracefully_on_allocation_failure);
    RUN_TEST(add_patch_to_array_should_fail_gracefully_on_allocation_failure);

    return UNITY_END();
}
