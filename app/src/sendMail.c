#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <curl/curl.h>

#define MAX_TEXT_WIDTH 79
#define MAX_EMAIL_LENGTH 1000

struct upload_status {
    size_t bytes_read;
};


// Most code referenced from: https://curl.se/libcurl/c/smtp-mail.html
static const char googleSSL[] = "smtp://smtp.gmail.com:587";

static char *payload_text;

static void writeEmail(char *to, char *from) {
    payload_text = malloc(sizeof(payload_text) * MAX_EMAIL_LENGTH);
    payload_text[0] = '\0';

    // Write date
    char buf[MAX_TEXT_WIDTH];
    time_t t = time(NULL);
    // char time[64];
    struct tm *tm = localtime(&t);
    size_t ret = strftime(buf, sizeof(buf), "Date: %c\r\n", tm);
    if (ret == 0) {
        perror("failed to write date");
        exit(EXIT_FAILURE);
    }
    // snprintf(buf, MAX_TEXT_WIDTH, "Date: ", );
    strncat(payload_text, buf, MAX_EMAIL_LENGTH);

    // Set "To"
    snprintf(buf, MAX_TEXT_WIDTH, "To: %s\r\n", to);
    strncat(payload_text, buf, MAX_EMAIL_LENGTH);

    // Set "From"
    snprintf(buf, MAX_TEXT_WIDTH, "From: %s\r\n", from);
    strncat(payload_text, buf, MAX_EMAIL_LENGTH);

    // Set "Subject"
    strncat(payload_text, "Subject: CCTV notification\r\n", MAX_EMAIL_LENGTH);

    // Empty lines before body
    strncat(payload_text, "\r\n", MAX_EMAIL_LENGTH);

    // Set "Body"
    strncat(payload_text, "Your camera is being attacked.\r\n", MAX_EMAIL_LENGTH);

    // Add an image
    // can be used to attach a photo of the movement detected on the cameras
    // strncat(payload_text, "Content-Type: image/jpeg\r\n", MAX_EMAIL_LENGTH);
    // strncat(payload_text, "filename=\"screenshot.jpeg\"\r\n", MAX_EMAIL_LENGTH);
    // strncat(payload_text, "put base 64 data here\r\n", MAX_EMAIL_LENGTH);
    // https://curl.se/mail/lib-2017-02/0139.html 
}

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct upload_status *upload_ctx = (struct upload_status *) userp;
    const char *data;
    size_t room = size * nmemb;

    if (size == 0 || nmemb == 0 || (size * nmemb) < 1) {
        return 0;
    }

    data = &payload_text[upload_ctx->bytes_read];
    
    if(data) {
        size_t len = strlen(data);
        if (room < len) {
            len = room;
        }
        printf("a\n");
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
    // Get credentials from envrionment variables
    const char* user = getenv("GOOGLE_USER");
    const char* pass = getenv("GOOGLE_PASS");

    // create email text
    writeEmail(sendTo, (char *) user);

    if (!user || !pass) {
        printf("missing environment variables to send email.\n");
        return;
    }

    // Construct email
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *recipients = NULL;
        // Authentication
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
        curl_easy_setopt(curl, CURLOPT_URL, googleSSL);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

        // Email content
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, user);
        recipients = curl_slist_append(recipients, sendTo);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        // Send email
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl easy perform failed\n");
        }

        // Cleanup
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}

