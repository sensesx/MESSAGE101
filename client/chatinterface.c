#include <ncurses.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include "../server/localfunc.h"

#define INPUT_HEIGHT 3  // Height for the input textbox

char servbuf[1024];

int livechat(int clifd) {
	initscr();
	cbreak();            // Disable line buffering
	keypad(stdscr, TRUE); // Enable special keys

	int chat_height = LINES - INPUT_HEIGHT;
	int chat_width = COLS;
	int input_height = INPUT_HEIGHT;
	int input_width = COLS;
	WINDOW *chat_win = newwin(chat_height, chat_width, 0, 0);
	WINDOW *input_win = newwin(input_height, input_width, chat_height, 0);

	scrollok(chat_win, TRUE);

	box(input_win, 0, 0);
	wrefresh(chat_win);
	wrefresh(input_win);

	char input[256] = {0};  // Buffer to hold user input
	int bytes = -1;

	fcntl(clifd, F_SETFL, O_NONBLOCK);
	struct epoll_event event, events[5];
		int epollFd = epoll_create(0);
	if(epollFd == -1){
		wprintw(chat_win, "Could not create epoll.");
		return -1;
	}
	event.events = EPOLLIN;
	event.data.fd = clifd;
	if(epoll_ctl(epollFd, EPOLL_CTL_ADD, clifd, &event)){ 
		wprintw(chat_win, "Could not create epoll.");
		close(epollFd);
		return -1;
	}
    	while (1) {
		werase(input_win);
		box(input_win, 0, 0);
		mvwprintw(input_win, 1, 1, "Enter message: ");
		wrefresh(input_win);
		epoll_wait(epollFd, events, 5, 30000);
		wgetnstr(input_win, input, 256); // WE GET THE INPUT BUFFER
						 
		memset(servbuf, 0, strlen(servbuf));
		bytes = recv(clifd, servbuf, sizeof(servbuf), 0);
		send(clifd, input, strlen(input), 0);
		
		if(strlen(input) != 0){
			wprintw(chat_win, "user: %s\n", input);
			memset(input, 0, sizeof(input));
		}
		wprintw(chat_win, "user: %s\n", servbuf);
		wrefresh(chat_win);
		    }
	if (close(epollFd)) {
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		return 1;
	}


    // Cleanup: delete windows and end ncurses mode
    delwin(chat_win);
    delwin(input_win);
    endwin();
    return 0;
}

