#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

/* a string with its own heap-allocated buffer */
typedef struct {
    char *data;          /* the actual string (heap) */
    unsigned int length; /* number of characters (excluding '\0') */
    size_t size;         /* total bytes used by the string */
} String_Slice;

String_Slice string_slice_new(char *data);
void string_slice_trim(String_Slice *str);
void string_slice_to_upper(String_Slice *str);
void string_slice_to_lower(String_Slice *str);
String_Slice *string_slice_split(const String_Slice *str, char delim, size_t *count);
String_Slice string_slice_splice(const String_Slice *str, size_t pos, const char *insert);

int main(void)
{
    /* create a new String_Slice from a literal */
    String_Slice s = string_slice_new("Hello World");

    /* case conversion (in place) */
    string_slice_to_lower(&s);
    printf("%s\n", s.data);

    string_slice_to_upper(&s);
    printf("%s\n", s.data);

    /* split on space, print each token */
    size_t n;
    String_Slice *parts = string_slice_split(&s, ' ', &n);
    for (size_t i = 0; i < n; i++)
        printf("  [%zu] %s\n", i, parts[i].data);

    /* insert " there" at position 5 */
    String_Slice spliced = string_slice_splice(&s, 5, " there");
    printf("%s\n", spliced.data);

    /* free everything we allocated */
    free(spliced.data);
    for (size_t i = 0; i < n; i++) free(parts[i].data);
    free(parts);
    free(s.data);

    return 0;
}

/* allocate a new String_Slice with a deep copy of `data` */
String_Slice string_slice_new(char *data)
{
    String_Slice str;

    /* allocate buffer: string length + null terminator */
    str.data = malloc(strlen(data) + 1);
    if (str.data == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for new string_slice.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(str.data, data);
    str.length = strlen(str.data);
    str.size   = str.length * sizeof(char);

    return str;
}

/* remove leading and trailing spaces (in place) */
void string_slice_trim(String_Slice *str)
{
    /* find first non-space */
    char *start = str->data;
    while (*start == ' ')
        start++;

    /* find last non-space */
    char *end = str->data + str->length - 1;
    while (end >= start && *end == ' ')
        end--;

    /* shift trimmed content to the front */
    size_t new_len = (size_t)(end - start + 1);
    memmove(str->data, start, new_len);
    str->data[new_len] = '\0';

    /* update metadata */
    str->length = (unsigned int)new_len;
    str->size   = new_len * sizeof(char);
}

/* convert all characters to uppercase (in place) */
void string_slice_to_upper(String_Slice *str)
{
    for (size_t i = 0; i < str->length; i++)
        str->data[i] = toupper((unsigned char)str->data[i]);
}

/* convert all characters to lowercase (in place) */
void string_slice_to_lower(String_Slice *str)
{
    for (size_t i = 0; i < str->length; i++)
        str->data[i] = tolower((unsigned char)str->data[i]);
}

/* split on `delim`, return a dynamically-sized array of String_Slice.
   *count is set to the number of tokens. caller frees each .data and the array. */
String_Slice *string_slice_split(const String_Slice *str, char delim, size_t *count)
{
    *count = 0;
    size_t cap = 4; /* initial capacity for the parts array */
    String_Slice *parts = malloc(cap * sizeof(String_Slice));
    if (!parts) { fprintf(stderr, "alloc fail\n"); exit(EXIT_FAILURE); }

    size_t start = 0; /* start index of the current token */
    for (size_t i = 0; i <= str->length; i++)
    {
        /* token ends at delimiter or end of string */
        if (i == str->length || str->data[i] == delim)
        {
            if (i > start) /* skip empty tokens (consecutive delims) */
            {
                /* grow parts array if full */
                if (*count == cap) {
                    cap *= 2;
                    parts = realloc(parts, cap * sizeof(String_Slice));
                    if (!parts) { fprintf(stderr, "realloc fail\n"); exit(EXIT_FAILURE); }
                }

                /* copy the token into a new String_Slice */
                parts[*count].data = malloc((i - start + 1) * sizeof(char));
                memcpy(parts[*count].data, str->data + start, i - start);
                parts[*count].data[i - start] = '\0';
                parts[*count].length = (unsigned int)(i - start);
                parts[*count].size   = (i - start) * sizeof(char);
                (*count)++;
            }
            start = i + 1; /* next token starts after delimiter */
        }
    }
    return parts;
}

/* insert `insert` at position `pos`. returns a new String_Slice (original is unchanged).
   caller must free the returned .data. */
String_Slice string_slice_splice(const String_Slice *str, size_t pos, const char *insert)
{
    /* clamp pos to valid range */
    if (pos > str->length) pos = str->length;
    size_t ins_len = strlen(insert);
    size_t new_len = str->length + ins_len;

    /* allocate buffer for the result */
    String_Slice out;
    out.data = malloc((new_len + 1) * sizeof(char));
    if (!out.data) { fprintf(stderr, "alloc fail\n"); exit(EXIT_FAILURE); }

    /* copy: [0..pos) + insert + [pos..end) */
    memcpy(out.data, str->data, pos);
    memcpy(out.data + pos, insert, ins_len);
    memcpy(out.data + pos + ins_len, str->data + pos, str->length - pos);
    out.data[new_len] = '\0';

    out.length = (unsigned int)new_len;
    out.size   = new_len * sizeof(char);
    return out;
}   
