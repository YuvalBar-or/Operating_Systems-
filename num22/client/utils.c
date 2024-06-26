#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include "utils.h"

// Encode a string into base64-encoded representation
// args:
// input: the string
// length: the length of the input string
// output: a pointer to a char* that will be set to the encoded string
void base64_encode_string(const char *input, int length, char **output) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    int encoded_len = size_of_encoded_string(length);
    int minimum = min(encoded_len, bufferPtr->length - 1);
    *output = calloc(minimum + 1, sizeof(char));
    memcpy(*output, bufferPtr->data, minimum);
    (*output)[minimum] = 0;

    pad_correctly_base_64(output, minimum);
    BIO_free_all(bio);
}

// Minimum between 2 ints
int min(int a, int b)
{
    if (a < b) return a;
    return b;
}

// Function to add '=' padding so the encoded string size would be a multlipication of 4
void pad_correctly_base_64(char **output, int cur_len)
{
    int leftover = 4 - (cur_len%4);
    if (leftover < 4)
    {
        (*output) = realloc((*output), cur_len + 1 + leftover);

        for (int i = 0; i < leftover; i++)
        {
            (*output)[cur_len+i] = '=';
        }

        (*output)[cur_len + leftover] = '\0';
        
    }
    
}

// Function to get the size of a decoded string
int size_of_encoded_string(int length) {
    return (int)(4 * (length / 3.0));
}

// Function to get the size of an encoded string
int size_of_decoded_string(int length) {
    return (3 * length) / 4;
}

// Decode a base64-encoded string into its original binary representation
// args:
// input: the base64-encoded string
// length: the length of the input string
// output: a pointer to a char* that will be set to the decoded string
void base64_decode_string(const char *input, int length, char **output) {
    BIO *bio, *b64;

    int decoded_length = size_of_decoded_string(length);
    *output = calloc(decoded_length + 2, sizeof(char));
    
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(input, length);
    bio = BIO_push(b64, bio);

    int actual_read = BIO_read(bio, *output, decoded_length);
    if (actual_read == 0) {
        fprintf(stderr, "Decoding error: expected %d bytes, got %d bytes\n", decoded_length, actual_read);
        free(*output);
        *output = NULL;
    } else {
        (*output)[actual_read] = '\n';
        (*output)[actual_read + 1] = '\0'; // Null-terminate the output
    }

    BIO_free_all(bio);
}