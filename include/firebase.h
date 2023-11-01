
#include <Firebase_ESP_Client.h>
extern FirebaseData fbdo;
void Firebase_Init(const String& streamPath);
void onFirebaseStream(FirebaseStream data);
void onFirebaseStreamTimeout(bool timeout);
void firebaseSendData(const String& path, const String& payload);