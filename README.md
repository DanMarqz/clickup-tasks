# C ClickUp Tasks

A simple C command-line tool to fetch and display ClickUp tasks.

## Prerequisites

Before compiling and running this project, you need to have the following libraries installed:

*   **libcurl**: For making HTTP requests to the ClickUp API.
*   **jansson**: For parsing JSON responses from the API.

You can typically install these on a Debian-based system (like Ubuntu) using:

```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev libjansson-dev
```

## Compilation

To compile the project, simply run the `make` command in the project's root directory:

```bash
make
```

This will generate an executable file named `clickup_tasks`.

## Execution

To run the program, you need to set the following environment variables with your ClickUp credentials:

*   `CLICKUP_TOKEN`: Your personal ClickUp API token.
*   `CLICKUP_USERID`: Your numeric user ID.
*   `CLICKUP_TEAMID`: The ID of the team/workspace you want to fetch tasks from.

You can set them and run the executable like this:

```bash
export CLICKUP_TOKEN="your_api_token"
export CLICKUP_USERID="your_user_id"
export CLICKUP_TEAMID="your_team_id"

./clickup_tasks
```
