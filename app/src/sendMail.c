#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

struct upload_status {
    size_t bytes_read;
};

static const char googleSSL[] = "smtp://smtp.gmail.com:587";

static const char *payload_text[] = {
    "To: ",
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct upload_status *upload_ctx = (struct upload_status *) userp;
    const char *data;
    size_t room = size * nmemb;

    if (size == 0 || nmemb == 0 || (size * nmemb) < 1) {
        return 0;
    }

    data = payload_text[upload_ctx->bytes_read];
    
    if(data) {
        size_t len = strlen(data);
        if (room < len) {
            len = room;
        }
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;

        return len;
    }

    return 0;
}

void sendMail_send(char *sendTo) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct upload_status upload_ctx = { 0 };
    const char* user = getenv("GMAIL_USER");
    const char* pass = getenv("GMAIL_PASS");

    if (!user || !pass) {
        printf("missing environment variables to send email.\n");
        return;
    }

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *recipients = NULL;

        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
        curl_easy_setopt(curl, CURLOPT_URL, googleSSL);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, user);

        recipients = curl_slist_append(recipients, sendTo);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl easy perform failed\n");
        }

        curl_slist_free_all(recipients);

        curl_easy_cleanup(curl);
    }
}

