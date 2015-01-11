//Chaos.cpp
//
//	For educational purposes only.  Thanks to a bunch of sites for help/code:
//
//	OpenGL (http://www.opengl.com)
//	NeHe Graphics (http://nehe.gamedev.net)
//	Game Tutorials (http://www.gametutorials.com)
//	Lighthouse 3D (http://www.lighthouse3d.com/opengl/tutorials.shtml)
//	CS 1566 @ Pitt
//
//	Game completely based on Sqaures 2 by Gavin Shapiro
//		http://www.megastar.co.uk/gaming/arcade/squares2.html
//
//		-Jason Blatt (jlb213@pitt.edu)
//
//	Note: Please let me know if the above links do not work so I can change them.

/*

Game information:

	Move your cursor around to collect the flying boxes.  There are five different types of boxes:

	Blue:   Collect these- they are worth points.  The smaller ones are worth more than the larger ones.
	Red:    Avoid these- hit three and your game is over.
	Magenta:Avoid these - they are power downs and will cause one of the following to occur:
				-Unlucky: your cursor doubles in size and more red boxes appear (lasts 15 seconds)
				-Fast Motion: game speed increases by 50% (lasts 15 seconds)
				-Point deduction: A random amount of points are deducted from your score
	Teal:	Collect these - they are power ups and will cause one of the following to occur:
				-Lucky: your cursor size is halved and will miss red boxes 50% of the time (lasts 15 seconds)
				-Slo Motion: game speed is reduced by half (lasts 15 seconds)
				-Invinibility: you will be invincible to red boxes (but not yellow!) (lasts 15 seconds)
	White:  Collect these - they give you a point bonus.


	As you collect more boxes, more will spawn and begin to clutter the screen.  Other special effects
	will be activated at higher levels in order to distract you from playing.


Features yet to be implemented:

	-About box
	-High scores
	-Unique graphics for the different boxes (instead of just different colors)
	-Sound effects and music
	-More chaos!  Planned special effects include:
		-Fading cursor and object trails
		-Random explosions and flourishes
		-Anything else that sounds cool and that I'm willing to code before I have too much
			work to do for class again.


Known Bugs:

	-I wrote and tested this program on my computer, so basically I was lazy.  If you do not play the
		game at 1280x1024 resolution the statistics in the upper left will be offset.  Other strange things
		may also happen at a resolution different from this.  For best results, set your display to
		1280x1024 or greater.
	-The speed governor I implemented checks how many FPS you are running and sets the speed accordingly.
		If you hit 'p' as soon as the game loads, the game will begin at a very low FPS because it is still
		loading and will run ridiculously fast.  Wait until the squares at the title screen rotate smoothly
		and then begin the game.


Hint: stay near the center of the screen when things start to get difficult!


*/


#include <gl\glut.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>



void drawString(float x, float y, float z, void *font, char *string);
void setOrthographicProjection(void);
void resetPerpectiveProjection(void);
AUX_RGBImageRec *LoadBMP(char *Filename);
int LoadGLTextures(void);
void changeSize(int w, int h);
void processNormalKeys(unsigned char key, int x, int y);
void processSpecialKeys(int key, int x, int y);
void processMouse(int button, int state, int x, int y);
void processMouseMotion(int x, int y);
void main(int argc, char **argv);
void draw(void);
void drawObjects(int amt);
void drawBox(float size);
void drawCursor(void);
void checkCollisions(int amt);
void updateObjects(int amt);
void initNewObject(int id);
void setupNewGame(void);
void drawConsole(void);
void drawStarField(int amount);
float randNum(float low, float high);



//Window Dimensions
const int WINX = 1280, WINY = 1024;
//Global Variables
const int X = 0 , Y = 1 , Z = 2;
const int NUM_MODS = 6,
		  MOD_INVINCIBLE = 0,
		  MOD_SLOWMO = 1,
		  MOD_LUCKY = 2,
		  MOD_UNLUCKY = 3,
		  MOD_FASTMO = 4,
		  MOD_MINUSPTS = 5;
const int OBJ_ENEMY = 0,
          OBJ_FRIEND = 1,
		  OBJ_POWERUP = 2,
		  OBJ_POINTS = 3,
		  OBJ_POWERDOWN = 4;
const int NUM_CHAOS = 7,
		  CHAOS_STARFIELD = 0,
		  CHAOS_CURSOR_TRAILS = 1,
		  CHAOS_OBJECT_TRAILS = 2,
		  CHAOS_DEATH_FLOURISH = 3,
		  CHAOS_RANDOM_EXPLOSIONS = 4,
		  CHAOS_RANDOM_STREAKS = 5,
		  CHAOS_FLASHING = 6;
const int MAX_OBJECTS = 20;
const int MIN_X = -10,
		  MAX_X = 10,
		  MIN_Y = -7,
		  MAX_Y = 7,
		  STD_Z = -20;
const int MODE_TITLE = 1,
		  MODE_GAME = 2,
		  MODE_HIGHSCORES = 3,
		  MODE_ABOUT = 4;
		  

//Texture Maps
const int numTextures = 11;  
GLuint	texture[numTextures];


//Timing
int frame=0,currTime,timeBase=0;  
float fps=0, speed=1;


//Cursor Structure
typedef struct {
	float pos[3];
	float rot[3];
	boolean mods[NUM_MODS];
	float modTimers[NUM_MODS];
	float lastX , lastY;
	float size;
} Cursor;

Cursor mainCursor;


//Object Structure
typedef struct {
	float pos[3];
	float deltapos[3];
	float rot[3];
	float deltarot[3];
	float size;
	int type;
	boolean active;
} Object;

Object objects[MAX_OBJECTS];


//Game Structure
typedef struct {
	int numKilled;
	int score;
	int hits;
	boolean chaos[NUM_CHAOS];
} Game;

Game thisGame;


//Mode
int mode = MODE_TITLE;


//Misc
int i;
char c[50], temp[50];
float titleBoxRot = 0;



//Chaos Structures

//Starfield
const int NUM_STARS = 1500;
float speedMuliplier = 1;
float starRot = 0;
typedef struct {
	float pos[3];
	float speed;
	float size;
	float color[3];
} Star;

Star starField[NUM_STARS];














////
//Main Draw
////

void draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glPushMatrix();
	glLoadIdentity();
	

	//Draw Game
	if(mode == MODE_GAME) {

		int amount = thisGame.numKilled/20 + 4;
		if(amount > MAX_OBJECTS) amount = MAX_OBJECTS;

		//Draw objects
		drawObjects(amount);
		drawCursor();

		//Update positions and do collision detection
		checkCollisions(amount);
		updateObjects(amount);

		//Draw chaos based on number of objects on screen
		if(amount > 4) drawStarField(amount);

		//Draw Console
		drawConsole();

		//Display Information
		setOrthographicProjection();
		glPushMatrix();
		glLoadIdentity();
		glDisable(GL_LIGHTING) ;
		glColor4f(1,1,1,1);
		strcpy(c, "Points Earned: ");
		strcat(c, itoa(thisGame.score,temp,10));
		drawString(25,30,0,    GLUT_BITMAP_HELVETICA_18,c);
		strcpy(c, "Collected: ");
		strcat(c, itoa(thisGame.numKilled,temp,10));
		drawString(25,55,0,    GLUT_BITMAP_HELVETICA_18,c);
		strcpy(c, "Lives: ");
		strcat(c, itoa(3-thisGame.hits,temp,10));
		drawString(25,80,0,    GLUT_BITMAP_HELVETICA_18,c);
		strcpy(c, "FPS: ");
		strcat(c, gcvt(fps,3,temp));
		drawString(25,130,0,    GLUT_BITMAP_HELVETICA_18,c);
		glPopMatrix();
		resetPerpectiveProjection();


		//Draw mods
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glTranslatef(MAX_X,MIN_Y-1,STD_Z);
		for(i = 0 ; i < NUM_MODS-1 ; i++) {
			if(mainCursor.mods[i] == (char)true) {
				glEnable(GL_TEXTURE_2D);
				glColor4f(1,1,1,1);
				glBindTexture(GL_TEXTURE_2D, texture[10]);
				glBegin(GL_QUADS);  
					glTexCoord2f(0.0f, 1-(.1925+.2*(float)i)); glVertex3f(-7,0,0);
					glTexCoord2f(0.0f, 1-(.2*(float)i)); glVertex3f( -7,1.5,0);
					glTexCoord2f(1.0f, 1-(.2*(float)i)); glVertex3f(0,1.5,0);
					glTexCoord2f(1.0f, 1-(.1925+.2*(float)i)); glVertex3f(0,0,0);
				glEnd();

				float eTime = glutGet(GLUT_ELAPSED_TIME);
				float percentDone = (eTime - mainCursor.modTimers[i])/15000;
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_DEPTH_TEST);
				glColor4f(0,0,.5,1);
				glBegin(GL_QUADS);  
					glVertex3f(-5*(1-percentDone),0,0);
					glVertex3f( -5*(1-percentDone),.2,0);
					glVertex3f(0,.2,0);
					glVertex3f(0,0,0);
				glEnd();


				glTranslatef(0,1.5,0);
			}
				
		}
	
		glPopMatrix();
	}


	//Draw title
	else if(mode == MODE_TITLE){
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glDisable(GL_LIGHTING) ;

		glBindTexture(GL_TEXTURE_2D, texture[7]);
		glColor4f(1,1,1,1);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(MIN_Y,MIN_Y,STD_Z);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(MAX_Y,MIN_Y,STD_Z);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(MAX_Y,MAX_Y,STD_Z);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(MIN_Y,MAX_Y,STD_Z);
		glEnd();

		glPushMatrix();
		glTranslatef((float)MIN_X/1.5,(float)MIN_Y/6,STD_Z);
		glRotatef(titleBoxRot,1,1,1);
		glColor4f(0,0,1,.5);
		drawBox(1.5);
		glPopMatrix();

		glPushMatrix();
		glTranslatef((float)MAX_X/1.5,(float)MIN_Y/6,STD_Z);
		glRotatef(-titleBoxRot,1,1,1);
		glColor4f(0,0,1,.5);
		drawBox(1.5);
		glPopMatrix();

		titleBoxRot++;
		

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING) ;


		setOrthographicProjection();
		glPushMatrix();
		glLoadIdentity();
		strcpy(c, "Last score: ");
		strcat(c, itoa(thisGame.score,temp,10));
		glColor4f(1,1,1,1);
		drawString(25,30,0,  GLUT_BITMAP_HELVETICA_18,c);
		glPopMatrix();
		resetPerpectiveProjection();
	}


	glutSwapBuffers();


	//Timing
	frame++;
	currTime=glutGet(GLUT_ELAPSED_TIME);
	if (currTime - timeBase > 1000) {
		fps = frame*1000.0/(currTime-timeBase);
	 	timeBase = currTime;		
		frame = 0;
		if(fps >= 25) speed = 1;
		else speed = (20/fps) + .5;
	}

}





void drawCursor() {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glDisable(GL_LIGHTING) ;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(mainCursor.pos[X] , mainCursor.pos[Y] , mainCursor.pos[Z]);
	glRotatef(mainCursor.rot[X],1,0,0);
	glRotatef(mainCursor.rot[Y],0,1,0);
	glRotatef(mainCursor.rot[Z],0,0,1);

	//Draw Cursor and fading tail
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	float alpha = 1;
	for(i = 0; i < 5; i++) {
		//If invincible, flash cursor
		if(mainCursor.mods[MOD_INVINCIBLE]) glColor4f(1,randNum(0,1),1,alpha);
		else glColor4f(1,1,1,alpha);

		glBegin(GL_QUADS);  //Back
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-mainCursor.size,-mainCursor.size,-mainCursor.size);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( -mainCursor.size,mainCursor.size,-mainCursor.size);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( mainCursor.size,mainCursor.size,-mainCursor.size);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(mainCursor.size,-mainCursor.size,-mainCursor.size);
		glEnd();

		glRotatef(-10,0,0,1);
		alpha -= .2;
	}

	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING) ;
}




void drawObjects(int amt) {

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glDisable(GL_LIGHTING) ;

	for(i = 0 ; i < amt ; i++) {
		if(objects[i].active) {
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(objects[i].pos[X],objects[i].pos[Y],objects[i].pos[Z]);

			//Draw Object

			if(objects[i].type == OBJ_ENEMY) {
				glColor4f(1,.423,.423,.65);
			}
			else if(objects[i].type == OBJ_FRIEND) {
				glColor4f(.45,.478,1,.65);
			}
			else if(objects[i].type == OBJ_POWERUP) {
				glColor4f(0,1,.5,.65);
			}
			else if(objects[i].type == OBJ_POWERDOWN) {
				glColor4f(1,0,1,.65);
			}
			else if(objects[i].type == OBJ_POINTS) {
				glColor4f(1,1,1,.65);
			}

			glRotatef(objects[i].rot[X],1,0,0);
			glRotatef(objects[i].rot[Y],0,1,0);
			glRotatef(objects[i].rot[Z],0,0,1);
			
			drawBox(objects[i].size);

			glPopMatrix();
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING) ;
		
}


void drawBox(float size) {

			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_QUADS);  //Back
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-size,-size,-size);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( -size,size,-size);
				glTexCoord2f(1.0f, 1.0f); glVertex3f( size,size,-size);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(size,-size,-size);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[1]);
			glBegin(GL_QUADS);  //Front
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-size,-size,size);
				glTexCoord2f(0.0f, 1.0f); glVertex3f( -size,size,size);
				glTexCoord2f(1.0f, 1.0f); glVertex3f( size,size,size);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(size,-size,size);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[2]);
			glBegin(GL_QUADS);  //Left Side
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-size,-size,-size);
				glTexCoord2f(0.0f, 1.0f); glVertex3f( -size,size,-size);
				glTexCoord2f(1.0f, 1.0f); glVertex3f( -size,size,size);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-size,-size,size);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[3]);
			glBegin(GL_QUADS);  //Right Side
				glTexCoord2f(0.0f, 0.0f); glVertex3f(size,-size,-size);
				glTexCoord2f(0.0f, 1.0f); glVertex3f( size,size,-size);
				glTexCoord2f(1.0f, 1.0f); glVertex3f( size,size,size);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(size,-size,size);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[4]);
			glBegin(GL_QUADS);  //Top
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-size,size,size);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( -size,size,-size);
				glTexCoord2f(1.0f, 1.0f); glVertex3f( size,size,-size);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(size,size,size);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[5]);
			glBegin(GL_QUADS);  //Bottom
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-size,-size,size);
				glTexCoord2f(0.0f, 1.0f); glVertex3f( -size,-size,-size);
				glTexCoord2f(1.0f, 1.0f); glVertex3f( size,-size,-size);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(size,-size,size);
			glEnd();
}




void checkCollisions(int amt) {

	for(i = 0 ; i < amt ; i++) {
		if(objects[i].active) {
			float distance = sqrt((mainCursor.pos[X] - objects[i].pos[X])*
					              (mainCursor.pos[X] - objects[i].pos[X])+
								  (mainCursor.pos[Y] - objects[i].pos[Y])*
								  (mainCursor.pos[Y] - objects[i].pos[Y]));
			if(distance < mainCursor.size/1.5 + objects[i].size/2) {

				if(objects[i].type == OBJ_ENEMY) {
					//If not invinible
					if(!mainCursor.mods[MOD_INVINCIBLE]) {
						if(mainCursor.mods[MOD_LUCKY]) {
							//If lucky miss half of all enemies
							if(randNum(0,1) < .5);
							//Else count all enemy hits
							else {
								objects[i].active = false;
								thisGame.hits++;
							}
						}
						else {
							objects[i].active = false;
							thisGame.hits++;
						}
						
					}

					//Game Over
					if(thisGame.hits >= 3) mode = MODE_TITLE;
				}
				else if(objects[i].type == OBJ_FRIEND) {
					//Update number killed and award points inversely proportional to the objects size
					objects[i].active = false;
					thisGame.numKilled++;
					thisGame.score += (int)((2-objects[i].size)*100);
				}
				else if(objects[i].type == OBJ_POWERUP) {
					//Select a powerup randomly and set timer
					objects[i].active = false;
					float r;  int newMod;
					r = randNum(0,1);
					if(r <= .33) newMod = MOD_INVINCIBLE;
					else if(r > .33 && r <= .66) newMod = MOD_SLOWMO;
					else newMod = MOD_LUCKY;

					mainCursor.mods[newMod] = true;
					mainCursor.modTimers[newMod] = glutGet(GLUT_ELAPSED_TIME);
				}
				else if(objects[i].type == OBJ_POWERDOWN) {
					//Select a powerdown randomly and set timer
					objects[i].active = false;
					float r;  int newMod;
					r = randNum(0,1);
					if(r <= .33) newMod = MOD_UNLUCKY;
					else if(r > .33 && r <= .66) newMod = MOD_FASTMO;
					else newMod = MOD_MINUSPTS;

					if(newMod != MOD_MINUSPTS) {
						mainCursor.mods[newMod] = true;
						mainCursor.modTimers[newMod] = glutGet(GLUT_ELAPSED_TIME);
					}
					//If power down is minusPts, subtract points and do nothing else
					else thisGame.score -= randNum(1000,3000);
				}
				else if(objects[i].type == OBJ_POINTS) {
					//Award random amount of points up to 2000
					objects[i].active = false;
					thisGame.score += randNum(1000,3000);
				}
			}
		}
	}
}




void updateObjects(int amt) {

	//Update flying objects
	float modFactor = 1;

	//If slowmo reduce speed by half, if fastmo increase speed
	if(mainCursor.mods[MOD_SLOWMO]) modFactor = .5;
	if(mainCursor.mods[MOD_FASTMO]) modFactor = 1.5;

	for(i = 0 ; i < amt ; i++) {
		if(objects[i].active == false) initNewObject(i);  //Reinitialize object

		objects[i].pos[X] += objects[i].deltapos[X]*speed*modFactor;
		objects[i].pos[Y] += objects[i].deltapos[Y]*speed*modFactor;
		objects[i].pos[Z] += objects[i].deltapos[Z]*speed*modFactor;
		objects[i].rot[X] += objects[i].deltarot[X]*speed*modFactor;
		objects[i].rot[Y] += objects[i].deltarot[Y]*speed*modFactor;
		objects[i].rot[Z] += objects[i].deltarot[Z]*speed*modFactor;

		if(objects[i].pos[X] > MAX_X +4 ||  //Object has moved out of bounds
		   objects[i].pos[X] < MIN_X -4 ||
		   objects[i].pos[Y] > MAX_Y +4 ||
		   objects[i].pos[Y] < MIN_Y -4) objects[i].active = false;
	}




	//Update cursor properties

	//Set cursor size and rot
	mainCursor.rot[Z] -= (15*(int)speed)%360;
	mainCursor.size = 1 + (float)thisGame.numKilled / (float)200;

	//Factor in mods
	if(mainCursor.mods[MOD_LUCKY]) mainCursor.size *= .5;  //Decrease size for lucky
	if(mainCursor.mods[MOD_UNLUCKY]) mainCursor.size *= 1.5;  //Increase size for unlucky



	//Check status of mods
	for(i = 0 ; i < NUM_MODS ; i++) {
		//If mod is active
		if(mainCursor.mods[i]) {
			float newTime = glutGet(GLUT_ELAPSED_TIME);
			//Cancel mod after 15 seconds
			if(newTime - mainCursor.modTimers[i] > 15000) mainCursor.mods[i] = false;
		}
	}
	

}



void initNewObject(int id) {

	float r;

	//Choose object type based on probability
	r = randNum(0,1);
	if(r <= .475) objects[id].type = OBJ_ENEMY;
	else if(r > .475 && r <= .95) objects[id].type = OBJ_FRIEND;
	else if(r > .95 && r <= .96) objects[id].type = OBJ_POWERUP;
	else if(r > .96 && r <= .98) objects[id].type = OBJ_POWERDOWN;
	else objects[id].type = OBJ_POINTS;

	//If unlucky, change half of the objects to enemies unconditionally
	if(mainCursor.mods[MOD_UNLUCKY]) {
		if(randNum(0,1) < .5) objects[id].type = OBJ_ENEMY;
	}


	//Choose start location and speeds randomly
	r = randNum(0,1);
	if(r <= .25) {  //Left moving right
		objects[id].pos[X] = MIN_X - 3;
		objects[id].pos[Y] = randNum(MIN_Y,MAX_Y);
		objects[id].deltapos[X] = randNum(.1,.2);
		objects[id].deltapos[Y] = 0;
	}
	else if(r > .25 && r <= .5) {  //Right moving left
		objects[id].pos[X] = MAX_X + 3;
		objects[id].pos[Y] = randNum(MIN_Y,MAX_Y);
		objects[id].deltapos[X] = -randNum(.1,.2);
		objects[id].deltapos[Y] = 0;
	}
	else if(r > .50 && r <= .75) {  //Top moving down
		objects[id].pos[X] = randNum(MIN_X,MAX_X);
		objects[id].pos[Y] = MAX_Y + 3;
		objects[id].deltapos[X] = 0;
		objects[id].deltapos[Y] = -randNum(.1,.2);
	}
	else {  //Bottom moving up
		objects[id].pos[X] = randNum(MIN_X,MAX_X);
		objects[id].pos[Y] = MIN_Y - 3;
		objects[id].deltapos[X] = 0;
		objects[id].deltapos[Y] = randNum(.1,.2);
	}



	//Set rotations randomly
	objects[id].rot[X] = 0;
	objects[id].rot[Y] = 0;
	objects[id].rot[Z] = 0;

	objects[id].deltarot[X] = randNum(0,5);
	objects[id].deltarot[Y] = randNum(0,5);;
	objects[id].deltarot[Z] = randNum(0,5);;

		//Make powerup/powerdown/score objects spin faster
	if(objects[id].type == OBJ_POWERUP || objects[id].type == OBJ_POWERDOWN || objects[id].type == OBJ_POINTS) {
		objects[id].deltarot[X] *= 2.5;
		objects[id].deltarot[Y] *= 2.5;
		objects[id].deltarot[Z] *= 2.5;
	}

	objects[id].size = randNum(.3,.7);

	objects[id].pos[Z] = STD_Z;
	objects[id].deltapos[Z] = 0;
	objects[id].active = true;

}




void setupNewGame(void) {
	//Set up cursor
	mainCursor.pos[X] = 0;  mainCursor.pos[Y] = 0;  mainCursor.pos[Z] = STD_Z;
	mainCursor.rot[X] = 0;	mainCursor.rot[Y] = 0;	mainCursor.rot[Z] = 0;
	mainCursor.lastX = WINX/2; mainCursor.lastY = WINY/2;
	mainCursor.size = 1;
	for(i = 0 ; i < NUM_MODS ; i++) { 
		mainCursor.mods[i] = false;
		mainCursor.modTimers[i] = 0;
	}

	//Set up objects
	for(i = 0 ; i < MAX_OBJECTS ; i++) { objects[i].active = false; }

	//Set up starfield
	for(i = 0 ; i < NUM_STARS ; i++) {
		starField[i].pos[X] = randNum(-1000,1000);
		starField[i].pos[Y] = randNum(-1000,1000);
		starField[i].pos[Z] = randNum(-2000,-1000);
		starField[i].color[0] = randNum(0,1);
		starField[i].color[1] = randNum(0,1);
		starField[i].color[2] = randNum(0,1);
		starField[i].size = randNum(.5,3);
		starField[i].speed = randNum(1,7);
	}


	//Set up game
	thisGame.numKilled = 0;
	thisGame.score = 0;
	thisGame.hits = 0;
	for(i = 0 ; i < NUM_CHAOS ; i++) { thisGame.chaos[i] = false; }

	mode = MODE_GAME;

}



void drawConsole(void) {

	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef(MIN_X,MAX_Y+1,STD_Z);

	glColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glBegin(GL_TRIANGLES);  //Back
		glTexCoord2f(0.0f, 0.45f); glVertex3f(-.5,-4,0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( -.5,1,0);
		glTexCoord2f(1.0f, .95f); glVertex3f( 8,.5,0);
	glEnd();


	glPopMatrix();
	//glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}






////
//Chaos drawing
////

void drawStarField(int amount) {
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glDisable(GL_LIGHTING) ;
	speedMuliplier = (float)amount/(float)4;
	if(amount > 6) starRot = (starRot + (float)thisGame.numKilled/100);
	if(starRot > 360) starRot = 0;

	for(i = 0 ; i < NUM_STARS ; i++) {
		glPushMatrix();
		glLoadIdentity();
		glRotatef(starRot,0,0,1);
		glTranslatef(starField[i].pos[X] , starField[i].pos[Y] , starField[i].pos[Z]);
		if(amount > 5) glColor4f(starField[i].color[0],starField[i].color[1],starField[i].color[2],1);
		else glColor4f(1,1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-starField[i].size,-starField[i].size,0);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(starField[i].size,-starField[i].size,0);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(starField[i].size,starField[i].size,0);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-starField[i].size,starField[i].size,0);
		glEnd();
		starField[i].pos[Z] += starField[i].speed*speedMuliplier;
		if(starField[i].pos[Z] > 1) starField[i].pos[Z] = randNum(-2000,-1000);
		glPopMatrix();
	}


	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING) ;
}












////
//Init
////

void main(int argc, char **argv)
{
    glutInit(&argc, argv);
	glutInitWindowPosition(0,0);
    glutInitWindowSize(WINX,WINY);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    int mainwin = glutCreateWindow("Chaos 1.0a1");

    glutDisplayFunc(draw);
	glutIdleFunc(draw);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutMouseFunc(processMouse);
	glutMotionFunc(processMouseMotion);
	glutPassiveMotionFunc(processMouseMotion);
							
	glShadeModel(GL_SMOOTH);
	LoadGLTextures();

	GLfloat pos[4] = {1,10,5,1} ;
	
	glClearColor(0,0,0,1) ;
	glEnable(GL_LIGHTING) ;
	glEnable(GL_LIGHT0) ;
	glEnable(GL_COLOR_MATERIAL) ;
	glLightfv(GL_LIGHT0,GL_POSITION,pos) ;

	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	currTime = glutGet(GLUT_ELAPSED_TIME);
	srand((unsigned int)time((time_t *)NULL));

    glutMainLoop();
}








////
//Text Drawing Utility Functions
////

void drawString(float x, float y, float z, void *font, char *string) {  
  char *c;
  glRasterPos3f(x,y,z);
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

void setOrthographicProjection() {

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WINX, 0, WINY);
	glScalef(1, -1, 1);
	glTranslatef(0, -WINY, 0);
	glMatrixMode(GL_MODELVIEW);
}

void resetPerpectiveProjection() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}






////
//Texture Mapping Functions
////

AUX_RGBImageRec *LoadBMP(char *Filename)				
{
	FILE *File=NULL;									

	if (!Filename)										
	{
		return NULL;									
	}

	File=fopen(Filename,"r");							

	if (File)											
	{
		fclose(File);									
		return auxDIBImageLoad(Filename);				
	}

	return NULL;										
}

int LoadGLTextures()								
{
	GLuint loop;
	int Status=FALSE;							
	AUX_RGBImageRec *TextureImage[numTextures];					
	memset(TextureImage,0,sizeof(void *)*numTextures);				

	if ((TextureImage[0]=LoadBMP("Data/cube1.bmp")) &&
		(TextureImage[1]=LoadBMP("Data/cube2.bmp")) &&
		(TextureImage[2]=LoadBMP("Data/cube3.bmp")) &&
		(TextureImage[3]=LoadBMP("Data/cube4.bmp")) &&
		(TextureImage[4]=LoadBMP("Data/cube5.bmp")) &&
		(TextureImage[5]=LoadBMP("Data/cube6.bmp")) &&
		(TextureImage[6]=LoadBMP("Data/cursor2.bmp")) &&
		(TextureImage[7]=LoadBMP("Data/title.bmp")) &&
		(TextureImage[8]=LoadBMP("Data/star1.bmp")) &&
		(TextureImage[9]=LoadBMP("Data/console.bmp")) &&
		(TextureImage[10]=LoadBMP("Data/mods.bmp")))
	{
		Status=TRUE;							
		glGenTextures(numTextures, &texture[0]);					

		for (loop=0; loop<numTextures; loop++)					
		{
			glBindTexture(GL_TEXTURE_2D, texture[loop]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[loop]->sizeX, TextureImage[loop]->sizeY,
				0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[loop]->data);
		}
	}
	for (loop=0; loop<numTextures; loop++)						
	{
		if (TextureImage[loop])						
		{
			if (TextureImage[loop]->data)				
			{
				free(TextureImage[loop]->data);			
			}
			free(TextureImage[loop]);				
		}
	}
	return Status;								
}





////
//Resize window function
////

void changeSize(int w, int h)  
{
	if(h == 0) h = 1;

	double ratio = 1.0* w / h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glViewport(0, 0, w, h);

	gluPerspective(45,ratio,1,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0,0.0,5.0, 
		      0.0,0.0,-1.0,
			  0.0f,1.0f,0.0f);
}







////
//Process keys
////

void processNormalKeys(unsigned char key, int x, int y)
{
	if (key == 27) exit(0);

	if(mode == MODE_TITLE) {
		if(key == 'q' || key == 'Q') exit(0);
		if(key == 'p' || key == 'P') setupNewGame();
	}
}

void processSpecialKeys(int key, int x, int y)
{	


}






////
//Process Mouse
////

void processMouse(int button, int state, int x, int y) {

	if(state == GLUT_DOWN) {
		
	}

}


void processMouseMotion(int x, int y) {
	
	float dX = mainCursor.lastX - x;
	float dY = mainCursor.lastY - y;

	mainCursor.lastX = x;
	mainCursor.lastY = y;

	mainCursor.pos[X] -= dX/((float)WINX/20);
	mainCursor.pos[Y] += dY/((float)WINY/16);	
	
}




////
//Rand Num Function
////

float randNum(float low, float high) {
	return (high - low)*((float)rand()/(float)RAND_MAX) + low;
}





	





 

