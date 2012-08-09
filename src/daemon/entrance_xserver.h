#ifndef ENTRANCE_XLIB_H_
#define ENTRANCE_XLIB_H_
typedef int (*Entrance_X_Cb)(const char *data);
int entrance_xserver_init(Entrance_X_Cb start, const char *dname);
void entrance_xserver_wait();
void entrance_xserver_end();
void entrance_xserver_shutdown();
#endif /* ENTRANCE_XLIB_H_ */
