#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *data;
    unsigned int length;
    size_t size;
} String_Slice;

String_Slice string_slice_new(char *data);
void string_slice_trim(String_Slice *str);
void string_slice_to_upper(String_Slice *str);
void string_slice_to_lower(String_Slice *str);
String_Slice *string_slice_split(const String_Slice *str, char delim, size_t *count);
String_Slice string_slice_splice(const String_Slice *str, size_t pos, const char *insert);


int main(void)
{
    String_Slice s = string_slice_new("Hello World");

    string_slice_to_lower(&s);
    printf("%s\n", s.data);

    string_slice_to_upper(&s);
    printf("%s\n", s.data);

    size_t n;
    String_Slice *parts = string_slice_split(&s, ' ', &n);
    for (size_t i = 0; i < n; i++)
        printf("  [%zu] %s\n", i, parts[i].data);

    String_Slice spliced = string_slice_splice(&s, 5, " there");
    printf("%s\n", spliced.data);

    free(spliced.data);
    for (size_t i = 0; i < n; i++) free(parts[i].data);
    free(parts);
    free(s.data);

    return 0;
}

String_Slice string_slice_new(char *data)
{
    String_Slice str;

    str.data = malloc(strlen(data) + 1);
    if (str.data == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for new string_slice.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(str.data, data);
    str.length = strlen(str.data);
    str.size = str.length * sizeof(char);

    return str;
}

void string_slice_trim(String_Slice *str)
{
    char *start = str->data;
    char *end   = str->data + str->length - 1;

    while (*start == ' ')
        start++;

    while (end >= start && *end == ' ')
        end--;

    size_t new_len = (size_t)(end - start + 1);
    memmove(str->data, start, new_len);
    str->data[new_len] = '\0';

    str->length = (unsigned int)new_len;
    str->size   = new_len * sizeof(char);
}

void string_slice_to_upper(String_Slice *str)
{
    for (size_t i = 0; i < str->length; i++)
        str->data[i] = toupper((unsigned char)str->data[i]);
}

void string_slice_to_lower(String_Slice *str)
{
    for (size_t i = 0; i < str->length; i++)
        str->data[i] = tolower((unsigned char)str->data[i]);
}

String_Slice *string_slice_split(const String_Slice *str, char delim, size_t *count)
{
    *count = 0;
    size_t cap = 4;
    String_Slice *parts = malloc(cap * sizeof(String_Slice));
    if (!parts) { fprintf(stderr, "alloc fail\n"); exit(EXIT_FAILURE); }

    size_t start = 0;
    for (size_t i = 0; i <= str->length; i++)
    {
        if (i == str->length || str->data[i] == delim)
        {
            if (i > start)
            {
                if (*count == cap) {
                    cap *= 2;
                    parts = realloc(parts, cap * sizeof(String_Slice));
                    if (!parts) { fprintf(stderr, "realloc fail\n"); exit(EXIT_FAILURE); }
                }
                parts[*count].data = malloc((i - start + 1) * sizeof(char));
                memcpy(parts[*count].data, str->data + start, i - start);
                parts[*count].data[i - start] = '\0';
                parts[*count].length = (unsigned int)(i - start);
                parts[*count].size   = (i - start) * sizeof(char);
                (*count)++;
            }
            start = i + 1;
        }
    }
    return parts;
}

String_Slice string_slice_splice(const String_Slice *str, size_t pos, const char *insert)
{
    if (pos > str->length) pos = str->length;
    size_t ins_len = strlen(insert);
    size_t new_len = str->length + ins_len;

    String_Slice out;
    out.data = malloc((new_len + 1) * sizeof(char));
    if (!out.data) { fprintf(stderr, "alloc fail\n"); exit(EXIT_FAILURE); }

    memcpy(out.data, str->data, pos);
    memcpy(out.data + pos, insert, ins_len);
    memcpy(out.data + pos + ins_len, str->data + pos, str->length - pos);
    out.data[new_len] = '\0';

    out.length = (unsigned int)new_len;
    out.size   = new_len * sizeof(char);
    return out;
}
