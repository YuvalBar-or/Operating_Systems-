void* createCodec (char key[62]);

int place_in_62(char ch);

char index_to_char(int idx);

int encode(char * textin, char * textout, int len, void * codec);

int decode(char * textin, char * textout, int len, void * codec);

void freecodec(void * codec); 