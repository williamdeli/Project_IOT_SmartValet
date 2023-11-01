#include "firebase.h"
// change this with your google firebase realtime database
#define FIREBASE_HOST "https://project-sps-dbe99-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "j3WJDzD7zF8U3XzMxC3ZTDaWcbxD3oCuVb8vAwCE"
FirebaseData fbdo;
FirebaseConfig fbConfig;
FirebaseData fbdoStream;
void Firebase_Init(const String &streamPath)
{
    FirebaseAuth fbAuth;
    fbConfig.host = FIREBASE_HOST;
    fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&fbConfig, &fbAuth);
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(1024);
    Firebase.RTDB.setwriteSizeLimit(&fbdo, "small");
    while (!Firebase.ready())
    {
        Serial.println("Co nnecting to firebase...");
        delay(1000);
    }
    String path = streamPath;
    if (Firebase.RTDB.beginStream(&fbdoStream, path.c_str()))
    {
        Serial.println("Firebase stream on " + path);
        Firebase.RTDB.setStreamCallback(&fbdoStream, onFirebaseStream,
                                        onFirebaseStreamTimeout);
    }
    else
        Serial.println("Firebase stream failed: " + fbdoStream.errorReason());
}

void onFirebaseStreamTimeout(bool timeout)
{
    Serial.printf("Firebase stream timeout: %d\n", timeout);
}