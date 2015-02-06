#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream.h>
#include <SDL.h>
#include <SDL/SDL_image.h>

#define NO_TYPICAL_COLORS 200

int typical_foreground[NO_TYPICAL_COLORS][3];
int typical_background[NO_TYPICAL_COLORS][3];

int last_foreground[3];
int last_background[3];

int object_midpoint_x, object_midpoint_y;

enum interact_mode {SEGMENT, MANUAL, TRIVIAL};
enum background{CHECKERBOARD, WHITE, ALPHA};

const int ACTIVE_ALPHA_SIZE = 64;

int last_typical_fg, last_typical_bg;

int sqr(int v) {
  return v*v;
} // sqr

void rgb_swap(int c1[3], int c2[3])
{
  unsigned char temp;

  for(int c = 0; c < 3; c++) {
    temp = c1[c];
    c1[c] = c2[c];
    c2[c] = temp;
  } // for
} // rgb_swap

int rgb_diff(int c1[3], int c2[3])
{
  int diff = 0;

  for(int c = 0; c < 3; c++) {
    diff += sqr(c1[c] - c2[c]);
  } // for

  return (int)sqrt(diff);
} // rgb_diff

int rgb_avg(int c[3])
{
  return (c[0]+c[1]+c[2])/3;
} // rgb_avg

void k_means(unsigned char point_cloud[], int no_points,
	     int* center_1, int* center_2)
{    
  // find out the two clusters
  
  long no_clusterpoints_1, no_clusterpoints_2;
  
  do {
    for(int c = 0; c < 3; c++) {
      center_2[c] = rand() % 256;
      center_1[c] = rand() % 256;
    } // for
    
    int new_center_1[3], new_center_2[3];
    
    long dist_1, dist_2, difference;
    
    // loop as long as cluster centers change significantly
    
    do {      
      no_clusterpoints_1 = 0; no_clusterpoints_2 = 0;
      
      for(int c = 0; c < 3; c++)
	new_center_1[c] = new_center_2[c] = 0;
      
      for(int i = 0; i < no_points; i++) {
	
	dist_1 = dist_2 = 0;
	
	for(int c = 0; c < 3; c++) {
	  dist_1 += sqr((int)point_cloud[i*3+c] - center_1[c]);
	  dist_2 += sqr((int)point_cloud[i*3+c] - center_2[c]);
	} // for
	
	if(dist_1 < dist_2) {
	  
	  for(int c = 0; c < 3; c++)
	    new_center_1[c] += (int)point_cloud[i*3+c];
	  
	  no_clusterpoints_1++;
	} // if
	else {
	  
	  for(int c = 0; c < 3; c++)
	    new_center_2[c] += (int)point_cloud[i*3+c];
	  
	  no_clusterpoints_2++;
	} // else
      } // for
      
      difference = 0;
      
      for(int c = 0; c < 3; c++) {
	
	if(no_clusterpoints_1 > 0) new_center_1[c] /= no_clusterpoints_1;
	if(no_clusterpoints_2 > 0) new_center_2[c] /= no_clusterpoints_2;
	
	difference += abs(center_1[c] - new_center_1[c]);
	difference += abs(center_2[c] - new_center_2[c]);
      } // for
      
      for(int c = 0; c < 3; c++) {
	//	cout << "center_1[" << c << "]: " << (int)center_1[c] << endl;
	// cout << "center_2[" << c << "]: " << (int)center_2[c] << endl;
	
	center_1[c] = new_center_1[c];
	center_2[c] = new_center_2[c];
      } // for
      
    } while(difference > 1.0);    
  } while((no_clusterpoints_1 == 0) || (no_clusterpoints_2 == 0));
} // k_means

void SampleTypicalColor(SDL_Surface* p_picture, unsigned char* p_alpha, 
			int cx, int cy, int radius, bool sample_foreground)
{
  int start_x, end_x, start_y, end_y;

  start_x = cx - radius; end_x = cx + radius;
  start_y = cy - radius; end_y = cy + radius;

  if(start_x < 0) start_x = 0; if(start_y < 0) start_y = 0;

  if(end_x >= p_picture->w) end_x = p_picture->w-1;
  if(end_y >= p_picture->h) end_y = p_picture->h-1;

  for(int y = start_y; y < end_y; y++) {
    for(int x = start_x; x < end_x; x++) {

      if(sqr(x-cx)+sqr(y-cy) <= radius*radius) {

	if(sample_foreground == true) {
	  
	  // p_alpha[x+y*p_picture->w] = 255;
	  
	  if((rand() % 7) == 0) {
	    
	    typical_foreground[last_typical_fg][2] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+2];
	    typical_foreground[last_typical_fg][1] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+1];
	    typical_foreground[last_typical_fg][0] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+0];
	    
	    if(last_typical_fg < NO_TYPICAL_COLORS)
	      last_typical_fg++;
	  } // if
	} // if
	else {
	  
	  // p_alpha[x+y*p_picture->w] = 0;
	  
	  if((rand() % 7) == 0) {
	    typical_background[last_typical_bg][2] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+2];
	    typical_background[last_typical_bg][1] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+1];
	    typical_background[last_typical_bg][0] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+0];
	    
	    if(last_typical_bg < NO_TYPICAL_COLORS)
	      last_typical_bg++;
	  } // if
	} // else
      } // if     
    } // for
  } // for
} // SampleTypicalColor

void PaintAlpha(SDL_Surface* p_picture, unsigned char* p_alpha, 
		int cx, int cy, int radius, bool smooth)
{
  int start_x, end_x, start_y, end_y;

  start_x = cx - radius; end_x = cx + radius;
  start_y = cy - radius; end_y = cy + radius;

  if(start_x < 0) start_x = 0; if(start_y < 0) start_y = 0;

  if(end_x >= p_picture->w) end_x = p_picture->w-1;
  if(end_y >= p_picture->h) end_y = p_picture->h-1;

  // are we rather in the foreground or in the background?

  long alpha_sum = 0, index = 0;

  for(int y = start_y; y < end_y; y++) {
    for(int x = start_x; x < end_x; x++) {

      if(sqr(x-cx)+sqr(y-cy) <= radius*radius) {

	alpha_sum += p_alpha[x+y*p_picture->w];
	index++;

      } // if     
    } // for
  } // for

  alpha_sum /= index;

  int alpha;

  for(int y = start_y; y < end_y; y++) {
    for(int x = start_x; x < end_x; x++) {

      long distance = (int)sqrt(sqr(x-cx)+sqr(y-cy));

      if(distance <= radius) {

	if(alpha_sum >= 128) {	  
	  alpha = 32;

	  //	  if(distance == radius) alpha = 128;
	} // if
	else {	  
	  alpha = -32;
	} // else

	int temp_alpha = p_alpha[x+y*p_picture->w];

	temp_alpha += alpha;

	if(temp_alpha > 255) temp_alpha = 255;
	if(temp_alpha < 0) temp_alpha = 0;

	if(smooth) {
	  if(temp_alpha > 127) temp_alpha = 255;
	  else temp_alpha = 0;
	} // if

	p_alpha[x+y*p_picture->w] = temp_alpha;
      } // if     
    } // for
  } // for
} // SampleTypicalColor

void AlphaMate(SDL_Surface* p_picture, SDL_Surface* screen, unsigned char* p_alpha, 
	       unsigned char* p_background, int cx, int cy, int radius, bool& first_fg_bg_guess)
{
  int start_x, end_x, start_y, end_y;
  int index_hist = 0;

  int fg_midpoint_x, fg_midpoint_y;
  int bg_midpoint_x, bg_midpoint_y;

  fg_midpoint_x = bg_midpoint_y = 0;

  unsigned char *hist_rgb;

  int           *hist_x;
  int           *hist_y;

  unsigned char red, green, blue;

  hist_rgb = new unsigned char [4*3*radius*radius];
  hist_x =   new int [4*radius*radius];
  hist_y =   new int [4*radius*radius];
  
  start_x = cx - radius; end_x = cx + radius;
  start_y = cy - radius; end_y = cy + radius;

  if(start_x < 0) start_x = 0; if(start_y < 0) start_y = 0;

  if(end_x >= p_picture->w) end_x = p_picture->w-1;
  if(end_y >= p_picture->h) end_y = p_picture->h-1;

  for(int y = start_y; y < end_y; y++) {
    for(int x = start_x; x < end_x; x++) {

      if(sqr(x-cx)+sqr(y-cy) <= radius*radius) {

	if((rand() % 3) == 0) {

	  // store this color
	  
	  red = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+0];
	  green = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+1];
	  blue = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+2];
	  
	  hist_rgb[index_hist*3+0] = red;
	  hist_rgb[index_hist*3+1] = green;
	  hist_rgb[index_hist*3+2] = blue;

	  hist_x[index_hist] = x;
	  hist_y[index_hist] = y;

	  index_hist++;
	} // if

      } // if

    } // for
  } // for

  int background[3], foreground[3];

  k_means(hist_rgb, index_hist, background, foreground);

  // which is foreground, which is background?

  long dist_1, dist_2, summands;

  if(first_fg_bg_guess == true) {

    int  index_bg, index_fg;

    index_bg = index_fg = 0;

    // first_fg_bg_guess = false;

    // calculate spacial midpoints of both clusters

    fg_midpoint_x = fg_midpoint_y = bg_midpoint_x= bg_midpoint_y = 0;

    int temp_color[3];

    for(int i = 0; i < index_hist; i++) {

      temp_color[0] = hist_rgb[i*3+0];
      temp_color[1] = hist_rgb[i*3+1];
      temp_color[2] = hist_rgb[i*3+2];

      dist_1 = rgb_diff(temp_color, background);
      dist_2 = rgb_diff(temp_color, foreground);

      if(dist_1 < dist_2) {
	bg_midpoint_x += hist_x[i];
	bg_midpoint_y += hist_y[i];

	index_bg++;
      } // if
      else {
	fg_midpoint_x += hist_x[i];
	fg_midpoint_y += hist_y[i];

	index_fg++;
      } // else
    } // for

    if(index_bg > 0) {
      bg_midpoint_x /= index_bg;
      bg_midpoint_y /= index_bg;
    } // if

    if(index_fg > 0) {
      fg_midpoint_x /= index_fg;
      fg_midpoint_y /= index_fg;
    } // if

    // based on proximity of object-center

    dist_1 = sqr(fg_midpoint_x - object_midpoint_x) +
             sqr(fg_midpoint_y - object_midpoint_y);
        
    dist_2 = sqr(bg_midpoint_x - object_midpoint_x) +
             sqr(bg_midpoint_y - object_midpoint_y);

    /*
    dist_1 = dist_2 = summands = 0;
    
    for(int i = 0; i < last_typical_fg; i++) {
      dist_1 += rgb_diff(foreground, typical_foreground[i]);
      dist_2 += rgb_diff(background, typical_foreground[i]);
      summands++;
    } // for
    
    for(int i = 0; i < last_typical_bg; i++) {
      dist_1 += rgb_diff(background, typical_background[i]);
      dist_2 += rgb_diff(foreground, typical_background[i]);
      summands++;
    } // for
    
    if(summands > 0) {
      dist_1 /= summands;
      dist_2 /= summands;
    } // if
    */

    if(dist_2 < dist_1) {
      rgb_swap(foreground, background);
    } // if
    
    for(int c = 0; c < 3; c++) {
      last_foreground[c] = foreground[c];
      last_background[c] = background[c];
      
      if(rgb_avg(background) < rgb_avg(foreground))
	rgb_swap(last_foreground, last_background);
    } // for
    
  } // if
  else {
    
    // based on the previous color
    
    dist_1 = rgb_diff(background, last_background) +
      rgb_diff(foreground, last_foreground);
    
    dist_2 = rgb_diff(foreground, last_background) +
      rgb_diff(background, last_foreground);
    
    if(dist_2 < dist_1) {
      rgb_swap(foreground, background);
    } // if
    
    if(last_foreground[0] == -1) {
      for(int c = 0; c < 3; c++) {
	last_foreground[c] = foreground[c];
	last_background[c] = background[c];
	
	if(rgb_avg(background) < rgb_avg(foreground))
	  rgb_swap(last_foreground, last_background);
      } // for
    } // if
    else {
      for(int c = 0; c < 3; c++) {
	last_foreground[c] = (10*foreground[c] + 90*last_foreground[c])/100;
	last_background[c] = (10*background[c] + 90*last_background[c])/100;
      } // for
    } // else
  } // else

  for(int y = start_y; y < end_y; y++) {
    for(int x = start_x; x < end_x; x++) {

      if(sqr(x-cx)+sqr(y-cy) <= radius*radius) {

	int current_color[3];
	
	current_color[0] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+0];
	current_color[1] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+1];
	current_color[2] = ((unsigned char*)(p_picture->pixels))[x*3+y*p_picture->pitch+2];
	
	int dist_foreground = rgb_diff(current_color, foreground);
	int dist_background = rgb_diff(current_color, background);
	
	int alpha = 255*dist_background/(dist_foreground+dist_background);
	int temp_alpha;
	
	if(alpha < 128-ACTIVE_ALPHA_SIZE) {
	  p_alpha[x+y*p_picture->w] = 0;
	} // if
	else
	  if(alpha > 128+ACTIVE_ALPHA_SIZE)
	    p_alpha[x+y*p_picture->w] = 255;
	  else {
	    temp_alpha = 255*(alpha-128+ACTIVE_ALPHA_SIZE)/(ACTIVE_ALPHA_SIZE*2);
	    if(temp_alpha < 0) temp_alpha = 0;
	    if(temp_alpha > 255) temp_alpha = 255;

	    p_alpha[x+y*p_picture->w] = temp_alpha;
	    
	    p_background[3*(x+y*p_picture->w)+2] = background[0];
	    p_background[3*(x+y*p_picture->w)+1] = background[1];
	    p_background[3*(x+y*p_picture->w)+0] = background[2];
	  } // else
      } // if
    } // for
  } // for

  delete[] hist_y;
  delete[] hist_x;
  delete[] hist_rgb;
} // AlphaMate

int main(int argc, char** argv)
{
    SDL_Surface      *screen;
    SDL_Surface      *p_picture;
    SDL_Event        event;
    bool             redraw_image, redraw_partly, l_button_down;
    unsigned char    *p_alpha, *p_background;
    int              radius, cursor_x, cursor_y;
    interact_mode    i_mode = SEGMENT;
    bool             first_fg_bg_guess;
    background       artificial_background = CHECKERBOARD;

    l_button_down = false;
    last_typical_fg = last_typical_bg = 0;

    radius        = 18; 
    artificial_background = CHECKERBOARD;

    p_picture = IMG_Load(argv[1]);

    if(p_picture != NULL) {

    int window_width = p_picture->w;      
    int window_height = p_picture->h;

    p_alpha = new unsigned char[window_width*window_height];
    p_background = new unsigned char[window_width*window_height*3];

    for(int y = 0; y < window_height; y++) {
	for(int x = 0; x < window_width; x++) {
	  p_alpha[x + y*window_width] = 255;

	  p_background[3*(x + y*window_width)+0] = 0;
	  p_background[3*(x + y*window_width)+1] = 0;
	  p_background[3*(x + y*window_width)+2] = 0;
	} // for
    } // 
    

    // SDL Bibliothek initialisieren

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
	cerr << "Konnte die SDL Bibliothek nicht initialisieren" << endl;
	exit(-1);
    } // if

    unsigned long flags = 0;

//    flags = flags | SDL_FULLSCREEN; // fehlt das Flag -> Window-Modus

    // screen mit 24 Bit Farbtiefe erzeugen (8 Bit f. Rot, Gruen, Blau)

    screen = SDL_SetVideoMode(window_width, window_height, 24, flags);

    if(!screen) {
	cerr << "Der angeforderte Videomodus SetVideoMode(...) konnte nicht erzeugt werden - Abbruch" << endl;
	exit(-1);
    } // if

    long size_line = screen->pitch;

    redraw_image = true;
    redraw_partly = false;

    bool exit_application = false; // damit wird die folgende Schleife beendet
    int  start_x, start_y, end_x, end_y;
	
    do {

      SDL_WaitEvent(&event);

	switch(event.type) {
		
	    case SDL_MOUSEMOTION:	      
	      cursor_x = event.motion.x;
	      cursor_y = event.motion.y;

	      if(l_button_down) {

		switch(i_mode) {
		case SEGMENT:
		  AlphaMate(p_picture, screen, p_alpha, p_background, 
			    cursor_x, cursor_y, radius, first_fg_bg_guess);
		  break;

		case MANUAL:
		  PaintAlpha(p_picture, p_alpha, cursor_x, cursor_y, 
			     radius, false);
		  break;

		case TRIVIAL:
		  PaintAlpha(p_picture, p_alpha, cursor_x, cursor_y, 
			     radius, false);
		  break;
		} // switch

		redraw_image = true;
		redraw_partly = true;
	      } // if
	      break;
		
	    case SDL_MOUSEBUTTONDOWN:
	      switch(event.button.button) {
	      case SDL_BUTTON_LEFT:
		i_mode = SEGMENT;
		l_button_down = true;
		first_fg_bg_guess = true;	      
		break;

	      case SDL_BUTTON_RIGHT:
		i_mode = MANUAL;
		l_button_down = true;
		first_fg_bg_guess = true;	      
		break;

	      case SDL_BUTTON_MIDDLE:
		object_midpoint_x = cursor_x;
		object_midpoint_y = cursor_y;		
		break;

	      } // switch
	      break;
		
	    case SDL_MOUSEBUTTONUP:
	      l_button_down = false;
	      break;
		
	    case SDL_KEYDOWN:
		
	      switch(event.key.keysym.sym) {
	     
	      case 'b':

		if(artificial_background == ALPHA) 
		  artificial_background = CHECKERBOARD;
		else
		  if(artificial_background == WHITE) 
		    artificial_background = ALPHA;
		  else
		    if(artificial_background == CHECKERBOARD) 
		      artificial_background = WHITE;

		redraw_image = true;
		break;

	      case 't':
		i_mode = TRIVIAL;
		redraw_image = true;
		break;

	      case '1':
		radius = 8;
		radius = radius*2/3;
		cerr << "radius small" << endl;
		break;

	      case '2':
		radius = 16;
		radius = radius*2/3;
		cerr << "radius medium" << endl;
		break;

	      case '3':
		radius = 32;
		radius = radius*2/3;
		cerr << "radius large" << endl;
		break;

	      case '4':
		radius = 64;
		radius = radius*2/3;
		cerr << "radius large" << endl;
		break;

	      case 'r':
		last_typical_fg = last_typical_bg = 0;
		cerr << "reset foreground/background samples" << endl;
		break;

	      case 'f':
		SampleTypicalColor(p_picture, p_alpha, cursor_x, cursor_y, 
				   radius, true);
		redraw_image = true;
		break;
		
	      case 27:
	      case 'q':
	      case 'Q':
		exit_application = true;
		
	      default:
		break;
	      } // switch
	} // switch

	if(redraw_image == true) {

	  if(redraw_partly) {
	    
	    redraw_partly = false;

	    start_x = event.motion.x-radius;
	    end_x = event.motion.x+radius;

	    start_y = event.motion.y-radius;
	    end_y = event.motion.y+radius;
	  } // if
	  else {
	    start_x = 0;
	    end_x = p_picture->w;

	    start_y = 0;
	    end_y = p_picture->h;
	  } // else

	  if(start_x < 0) start_x = 0;
	  if(start_y < 0) start_y = 0;

	  if(end_x >= p_picture->w) end_x = p_picture->w-1;
	  if(end_y >= p_picture->h) end_y = p_picture->h-1;

	  for(int y = start_y; y < end_y; y++) {
	    
	    for(int x = start_x; x < end_x; x++) {
	      
	      // screen->pixels ist ein Zeiger auf das erste Byte des Bitmap.
	      // Da screen->pixels ein Zeiger ohne Typ ist, also void* (siehe
	      // struktur weiter oben), wird der Zeiger einfach konvertiert,
	      // so dass er nun auf Elemente vom Typ (unsigned char) zeigt.
	      
	      // Der Zeiger selbst wird mit (unsigned char*) bezeichnet. Mit
	      // dem Index-Operator [1] kannman z. B. auf das erste Element 
	      // zugreifen, oder es auslesen
	      
	      int new_bg_red, new_bg_green, new_bg_blue;
	      int bg_red, bg_green, bg_blue;
	      int fg_red, fg_green, fg_blue;

	      if(artificial_background == CHECKERBOARD) {
		new_bg_red = (((x/8)+(y/8)) % 2)*64+64;
		new_bg_green = (((x/8)+(y/8)) % 2)*64+64;
		new_bg_blue = (((x/8)+(y/8)) % 2)*64+64;
	      } // if

	      if(artificial_background == WHITE) {
		new_bg_red = 255;
		new_bg_green = 255;
		new_bg_blue = 255;
	      } // else

	      fg_red = ((int)(((unsigned char*)(p_picture->pixels))[x*3+0 + y*p_picture->pitch]));
	      fg_green = ((int)(((unsigned char*)(p_picture->pixels))[x*3+1 + y*p_picture->pitch]));
	      fg_blue = ((int)(((unsigned char*)(p_picture->pixels))[x*3+2 + y*p_picture->pitch]));
      
	      int alpha = (int)(p_alpha[x+y*window_width]);
	      
	      // subtract background color from mixed color
	      
	      bg_red = p_background[x*3+0 + y*p_picture->pitch];
	      bg_green = p_background[x*3+1 + y*p_picture->pitch];
	      bg_blue = p_background[x*3+2 + y*p_picture->pitch];
	            
	      fg_red -= ((255-alpha)*bg_red)/255;
	      if(fg_red < 0) fg_red = 0;

	      fg_green -= ((255-alpha)*bg_green)/255;
	      if(fg_green < 0) fg_green = 0;

	      fg_blue -= ((255-alpha)*bg_blue)/255;
	      if(fg_blue < 0) fg_blue = 0;

	      if(artificial_background == ALPHA) {
		((unsigned char*)(screen->pixels))[x*3+2 + y*screen->pitch] = alpha;
		((unsigned char*)(screen->pixels))[x*3+1 + y*screen->pitch] = alpha;
		((unsigned char*)(screen->pixels))[x*3+0 + y*screen->pitch] = alpha;
	      } // if
	      else {
		((unsigned char*)(screen->pixels))[x*3+2 + y*screen->pitch] = (fg_red*alpha+(255-alpha)*new_bg_red)/255;
	      
		((unsigned char*)(screen->pixels))[x*3+1 + y*screen->pitch] = (fg_green*alpha+(255-alpha)*new_bg_green)/255;
	      
		((unsigned char*)(screen->pixels))[x*3+0 + y*screen->pitch] = (fg_blue*alpha+(255-alpha)*new_bg_blue)/255;
	      } // else
	    } // for
	  } // for
	  
	  SDL_UnlockSurface(screen);
	  SDL_UpdateRect(screen, start_x, start_y, end_x-start_x, end_y-start_y);
	  SDL_LockSurface(screen);

	  redraw_image = false;
	} // if

    } while(exit_application == false);
	    
 exit_app:

    delete[] p_background;
    delete[] p_alpha;

    SDL_UnlockSurface(screen);	    
    SDL_FreeSurface(p_picture);
    SDL_FreeSurface(screen);

    } // if

    exit(0);
} // main
