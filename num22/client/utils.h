void base64_encode_string(const char *input, int length, char **output);
void base64_decode_string(const char *input, int length, char **output);

void pad_correctly_base_64(char **output, int cur_len);
int min(int a, int b);

int size_of_encoded_string(int length);
int size_of_decoded_string(int length);