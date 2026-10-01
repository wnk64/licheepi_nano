#ifndef WFD_VIDEO_H
#define WFD_VIDEO_H
#include <stddef.h>
int wfd_video_start(int fd);
int wfd_video_push(const void *data, size_t bytes);
int wfd_video_report(void);
void wfd_video_stop(void);
#endif
