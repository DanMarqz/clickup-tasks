#include "main.h"

// Callback function for libcurl to write response data into a memory buffer
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void print_table_separator() {

    printf("+");

    for (int i = 0; i < ID_WIDTH + 2; i++) printf("-");

    printf("+");

    for (int i = 0; i < NAME_WIDTH + 2; i++) printf("-");

    printf("+");

    for (int i = 0; i < STATUS_WIDTH + 2; i++) printf("-");

    printf("+");

    for (int i = 0; i < ASSIGNEES_WIDTH + 2; i++) printf("-");

    printf("+\n");

}



void print_table_header() {

    print_table_separator();

    printf("| %-*s | %-*s | %-*s | %-*s |\n", ID_WIDTH, "ID", NAME_WIDTH, "Name", STATUS_WIDTH, "Status", ASSIGNEES_WIDTH, "Assignees");

    print_table_separator();

}



void print_table_row(const char *id, const char *name, const char *status, const char *assignees) {

    char truncated_name[NAME_WIDTH + 1];



    strncpy(truncated_name, name, NAME_WIDTH);

    truncated_name[NAME_WIDTH] = '\0';

    if (strlen(name) > NAME_WIDTH) {

        strcpy(truncated_name + NAME_WIDTH - 3, "...");

    }



    printf("| %-*s | %-*s | %-*s | %-*s |\n", ID_WIDTH, id, NAME_WIDTH, truncated_name, STATUS_WIDTH, status, ASSIGNEES_WIDTH, assignees);

}





int main(void) {

    // 1. Get config from environment variables

    const char *token = getenv("CLICKUP_TOKEN");

    const char *user_id = getenv("CLICKUP_USERID");

    const char *team_id = getenv("CLICKUP_TEAMID");



    if (!token || !user_id || !team_id) {

        fprintf(stderr, "Error: Please set CLICKUP_TOKEN, CLICKUP_USERID, and CLICKUP_TEAMID environment variables.\n");

        return 1;

    }



    CURL *curl_handle;

    CURLcode res;



    struct MemoryStruct chunk;

    chunk.memory = malloc(1);

    chunk.size = 0;



    // 2. Prepare the API request

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();



    char url[512]; // Increased buffer size for URL

    snprintf(url, sizeof(url), "https://api.clickup.com/api/v2/team/%s/task?assignees[]=%s&include_closed=false", team_id, user_id);



    char auth_header[256];

    snprintf(auth_header, sizeof(auth_header), "Authorization: %s", token);



    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, auth_header);



    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");



    // 3. Perform the request

    res = curl_easy_perform(curl_handle);



    if(res != CURLE_OK) {

        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    } else {

        // 4. Parse JSON response

        json_error_t error;

        json_t *root = json_loads(chunk.memory, 0, &error);



        if (!root) {

            fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);

            // If JSON parsing fails, try to print the raw response for debugging

            if (chunk.size > 0) {

                fprintf(stderr, "Raw API response (first 500 chars): \n%.*s\n", (int)fmin(chunk.size, 500), chunk.memory);

            }

            json_decref(root);

            return 1;

        }



        json_t *tasks = json_object_get(root, "tasks");

        if (!json_is_array(tasks)) {

            fprintf(stderr, "error: 'tasks' is not an array in the JSON response.\n");

            json_decref(root);

            return 1;

        }



        if (json_array_size(tasks) == 0) {

            printf("No tasks found.\n");

        } else {

            print_table_header();



            // 5. Iterate through tasks and print them

            for (size_t i = 0; i < json_array_size(tasks); i++) {

                json_t *task = json_array_get(tasks, i);

                

                json_t *custom_id_json = json_object_get(task, "custom_id");

                const char *custom_id = (custom_id_json && json_is_string(custom_id_json)) ? json_string_value(custom_id_json) : "N/A";



                json_t *name_json = json_object_get(task, "name");

                const char *name = (name_json && json_is_string(name_json)) ? json_string_value(name_json) : "N/A";



                json_t *status_obj = json_object_get(task, "status");

                json_t *status_val = (status_obj && json_is_object(status_obj)) ? json_object_get(status_obj, "status") : NULL;

                const char *status_str = (status_val && json_is_string(status_val)) ? json_string_value(status_val) : "N/A";



                // Assignees
                json_t *assignees_arr = json_object_get(task, "assignees");
                char assignees_str_buf[ASSIGNEES_WIDTH + 1];
                assignees_str_buf[0] = '\0'; // Initialize as empty string
                size_t current_len = 0;

                if (json_is_array(assignees_arr) && json_array_size(assignees_arr) > 0) {
                    for (size_t j = 0; j < json_array_size(assignees_arr); j++) {
                        json_t *assignee = json_array_get(assignees_arr, j);
                        json_t *username_json = json_object_get(assignee, "username");
                        const char *username = (username_json && json_is_string(username_json)) ? json_string_value(username_json) : NULL;

                        if (username) {
                            if (current_len > 0) { // Add comma and space if not the first assignee
                                size_t remaining_space = ASSIGNEES_WIDTH - current_len;
                                if (remaining_space > 0) {
                                    strncat(assignees_str_buf, ", ", remaining_space);
                                    current_len += fmin(remaining_space, 2); // Add 2 for ", "
                                }
                            }
                            
                            size_t remaining_space = ASSIGNEES_WIDTH - current_len;
                            if (remaining_space > 0) {
                                strncat(assignees_str_buf, username, remaining_space);
                                current_len += fmin(remaining_space, strlen(username));
                            }
                        }
                        if (current_len >= ASSIGNEES_WIDTH) { // Stop if buffer is full
                            break;
                        }
                    }
                }
                if (assignees_str_buf[0] == '\0') { // If no assignees were added
                    strncpy(assignees_str_buf, "N/A", ASSIGNEES_WIDTH);
                    assignees_str_buf[ASSIGNEES_WIDTH] = '\0';
                } else {
                    // Add a trailing space for alignment if there's room
                    if (current_len < ASSIGNEES_WIDTH) {
                        assignees_str_buf[current_len] = ' ';
                        assignees_str_buf[current_len + 1] = '\0';
                    }
                }



                print_table_row(custom_id, name, status_str, assignees_str_buf);

            }

            print_table_separator();

        }



        json_decref(root);

    }



    // 6. Cleanup

    curl_slist_free_all(headers);

    curl_easy_cleanup(curl_handle);

    free(chunk.memory);

    curl_global_cleanup();



    return 0;

}




