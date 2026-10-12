
#include <iostream>
#include <string>

#include <unistd.h>
#include <sys/inotify.h>
#include <linux/limits.h>

/*
struct inotify_event {
    int      wd;
    uint32_t mask;
    uint32_t cookie;
    uint32_t len;
    char     name[];
};
*/


std::string event_to_string(uint32_t mask) {
    switch (mask) {
    case IN_ACCESS: return "IN_ACCESS";
    case IN_ATTRIB: return "IN_ATTRIB";
    case IN_CLOSE_WRITE: return "IN_CLOSE_WRITE";
    case IN_CLOSE_NOWRITE: return "IN_CLOSE_NOWRITE";
    case IN_CREATE: return "IN_CREATE";
    case IN_DELETE: return "IN_DELETE";
    case IN_DELETE_SELF: return "IN_DELETE_SELF";
    case IN_MODIFY: return "IN_MODIFY";
    case IN_MOVE_SELF: return "IN_MOVE_SELF";
    case IN_MOVED_FROM: return "IN_MOVED_FROM";
    case IN_MOVED_TO: return "IN_MOVED_TO";
    case IN_OPEN: return "IN_OPEN";
    case IN_ISDIR: return "IN_ISDIR";
    default: return "UNKNOWN";
    }
}

std::string mask_to_string(uint32_t mask) {
    std::string result;

    const uint32_t elements[] = {
        IN_ISDIR,
        IN_ACCESS,
        IN_ATTRIB,
        IN_CLOSE_WRITE,
        IN_CLOSE_NOWRITE,
        IN_CREATE,
        IN_DELETE,
        IN_DELETE_SELF,
        IN_MODIFY,
        IN_MOVE_SELF,
        IN_MOVED_FROM,
        IN_MOVED_TO,
        IN_OPEN
    };

    for (uint32_t element : elements) {
        if (mask & element) {
            result += event_to_string(element) + " ";
        }
    }

    return result;
}


int main() {
    const std::string folder = "/home/fapablaza/Desktop";

    int inotfd = inotify_init();
    uint32_t mask = IN_ALL_EVENTS;
    int watchdesc = inotify_add_watch(inotfd, folder.c_str(), mask);

    int bufsize = sizeof(inotify_event) + PATH_MAX + 1;

    inotify_event *event_buffer = (inotify_event *)std::malloc(bufsize);

    std::cout << "Waiting for events at " << folder << std::endl;

    int current = 0;

    while(true) {
        int n = read(inotfd, event_buffer, bufsize);

        std::cout << "Event(s) received:" << std::endl;
        std::cout << bufsize << ", " << sizeof(inotify_event) << ", " << n << std::endl;

        int offset = 0;
        inotify_event *event = event_buffer;

        while (offset < n) {
            std::cout << "  <id>:" << current << ", ";
            std::cout << "  wd:" << event->wd << ", ";
            std::cout << "  cookie:" << event->cookie << ", ";

            if (event->len > 0) {
                std::cout << "  name: '" << event->name << "', ";
            }
            std::cout << "  mask:" << mask_to_string(event->mask) << ", ";

            std::cout << std::endl;

            offset += sizeof(inotify_event) + event->len;
            event ++;

            current++;
        }
    }

    return 0;
}
