#include <stdint.h>

typedef uintptr_t merklesquareclient_handle_t;

merklesquareclient_handle_t merklesquareclient_new(char* serverAddr, char* auditorAddr, char* verifierAddr);
void merklesquareclient_delete(merklesquareclient_handle_t p);

int merklesquareclient_set(merklesquareclient_handle_t p, char* key, char* value);
int merklesquareclient_get(merklesquareclient_handle_t p, char* key, char* buf);
