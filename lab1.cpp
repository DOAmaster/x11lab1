//modified by: Derrick Alden
//
//program: lab1.cpp
//author:  Gordon Griesel
//purpose: Framework for graphics
//         Using X11 (Xlib) for drawing
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

class Image {
public:
  char name[200];
  int width;
  int height;
  unsigned char *data;
  ~Image() { delete [] data; }
  Image() {
    //opens jpeg image
    system("convert lab1.jpg lab1.ppm");
    FILE *fpi = fopen("lab1.ppm", "r");
    if (fpi) {
      char line[250];
      fgets(line, 250, fpi);
      fgets(line, 250, fpi);
      //checks if the ppm has comments and skips them
      while (line[0] == '#') 
        fgets(line, 250, fpi);
      
      sscanf(line, "%i %i", &width, &height);
      printf("dimensions: %i %i\n", width, height);
      fgets(line, 250, fpi);
      //allocates memory for data
      data = new unsigned char[width * height * 3];
      for ( int i=0; i<width*height*3; i++)
        //returns one char at a time
        data[i] = fgetc(fpi);



      fclose(fpi);
    } else {
      printf("ERROR: opening input lab1.ppm\n");
      exit(0);
    }
    

    //deletes the temp ppm file
    unlink("lab1.ppm");
  }



} img;


class Global {
public:
	int xres, yres;
  int grayscale;
  int rotate;
	Global() {
		srand((unsigned)time(NULL));
		xres = 640;
		yres = 480;
    grayscale = 0;
    rotate = 0;
	}
} g;

//only renders on events
class X11 {
private:
	Display *dpy;
	Window win;
	GC gc;
public:
	X11() {
		//constructor
		if (!(dpy=XOpenDisplay(NULL))) {
			fprintf(stderr, "ERROR: could not open display\n"); fflush(stderr);
			exit(EXIT_FAILURE);
		}
		int scr = DefaultScreen(dpy);
		win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 1, 1,
			g.xres, g.yres, 0, 0x00ffffff, 0x00000000);
		XStoreName(dpy, win, "X11 sample program");
		gc = XCreateGC(dpy, win, 0, NULL);
		XMapWindow(dpy, win);
    //stating which events you want to track
		XSelectInput(dpy, win, ExposureMask | StructureNotifyMask |
			PointerMotionMask | ButtonPressMask |
			ButtonReleaseMask | KeyPressMask);
	}
	~X11() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void check_resize(XEvent *e) {
		//ConfigureNotify is sent when window size changes.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		g.xres = xce.width;
		g.yres = xce.height;
	}
	void clear_screen() {
		XClearWindow(dpy, win);
	}
	void setColor3i(int r, int g, int b) {
		unsigned long cref = 0L;
		cref += r;
		cref <<= 8;
		cref += g;
		cref <<= 8;
		cref += b;
		XSetForeground(dpy, gc, cref);
	}
	bool getXPending() {
		return (XPending(dpy));
	}
	void getXNextEvent(XEvent *e) {
		XNextEvent(dpy, e);
	}
	void drawLine(int x0, int y0, int x1, int y1) {
		XDrawLine(dpy, win, gc, x0, y0, x1, y1);
	}
	void drawPixel(int x, int y) {
    //display windows graphic context
		XDrawPoint(dpy, win, gc, x, y);
	}

	void drawText(int x, int y, const char *text) {
    //display windows graphic context
    XDrawString(dpy, win, gc, x, y, text, strlen(text));
		
	}
  void showImage(Image *img, int x, int y) {
    int offsetx = g.xres/2 - img->width/2;
    int offsety = g.yres/2 - img->height/2;
    //stepping through the height of the image loop
    for (int i = 0; i<img->height; i++){
      //stepping through the width of the image loop
      for (int j = 0; j<img->width; j++){
        int r1 = img->data[i*img->width*3 + j*3 + 0];
        int g1 = img->data[i*img->width*3 + j*3 + 1];
        int b1 = img->data[i*img->width*3 + j*3 + 2];
        setColor3i(r1, g1, b1);
        //checks if grayscale flag is on for black and white
        if (g.grayscale == 1) {
          int c = (r1+g1+b1)/3;
          setColor3i(c, c, c);
        }
        if (g.rotate == 90) {
          drawPixel(i+offsety, j+offsetx);
        }
        else if (g.rotate == 180) {
          //rotates images 180 degrees using img->width-1- j+offsetx
          drawPixel(img->width-1-j+offsetx, img->height-1- i+offsety);
        }
        else if (g.rotate == 270) {
          //rotates images 180 degrees using img->width-1- j+offsetx
          //drawPixel(img->width-1-j+offsetx, img->height-1- i+offsety);
        }
        else {
          drawPixel(j+offsetx, i+offsety);
        }

      }


    }

    
  }
  
} x11;

//function prototypes
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();


int main(void)
{
	int done = 0;
	while (!done) {
		//Check the event queue
		while (x11.getXPending()) {
			//Handle them one-by-one
			XEvent e;
			x11.getXNextEvent(&e);
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
			//Render when any event happens
			render();
		}
	}
	return 0;
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;
	int mx = e->xbutton.x;
	int my = e->xbutton.y;
	//
	if (e->type == ButtonRelease)
		return;
	if (e->type == ButtonPress) {
		//A mouse button was pressed.
		if (e->xbutton.button==1) {
			//Left button pressed
		}
		if (e->xbutton.button==3) {
			//Right button pressed
		}
	}
	if (savex != mx || savey != my) {
		//mouse moved
		savex = mx;
		savey = my;
	}
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	switch (key) {
		case XK_Escape:
			//program is ending
			return 1;
    //push g to toggle grayscape using exclusive or ^=
    case XK_g:
      g.grayscale ^= 1;
      break;
    case XK_r:
      if (g.rotate <= 270) {
          g.rotate += 90;
      } else {
        g.rotate = 0;
      }
     
      break;

	}
	return 0;
}

void physics(void)
{
	//no physics yet.
}

void render(void)
{
	//render function is always at the bottom of program.
	x11.setColor3i(255, 255, 0);
	//x11.drawLine(100, 100, 200, 200);
	//x11.drawPixel(g.xres/2, g.yres/2);
  x11.showImage(&img, g.xres/2, g.yres/2);

  x11.setColor3i(0, 255, 0);
  x11.drawText(10, 20, "Press G for grayscale");
  x11.drawText(10, 30, "Press R to rotate");
 // x11.drawText(40, 30, (char) g.rotate);
}









