#include <stdio.h>
#include <wiringPi.h>
#include <piFace.h>

#define PIFACE_BASE   200
//Times for different states of the lights,1 equals 500 ms
#define WAIT_TIME     10
#define CHANGE_TIME   5
#define PASS_TIME     30

//Read status of a button on the piFace
int readButton(int button)
{
  if(digitalRead(PIFACE_BASE + button)==HIGH)
    return 0; // Not pressed
  else if (digitalRead(PIFACE_BASE + button)==LOW)
    return 1; //Pressed
}

//For creating a sound effect of pedestrian traffic lights
//with the relay output
void beeper(int *state,int light,int waitTick)
{
  if((*state==0 || *state==1)  && (waitTick % 7)==0)
    {
      digitalWrite(PIFACE_BASE+ light,1);
      delay(100);
      digitalWrite(PIFACE_BASE+ light,0);
    }
  if(*state==2)
    {
      digitalWrite(PIFACE_BASE+ light,1);
      delay(50);
      digitalWrite(PIFACE_BASE+ light,0);
    }

}

//Management of the lights
//I.e. turning on and off certain outputs on the piFace
void lights(int *state,int light,int lightTick)
{
  //Three states for lights
  //0: Red light (e.g. LED 2)
  //1: Firing LED 3 and after a moment shutting it down
  //2: Firing LED 4 for x amount of time
  //Then lighting LED 3 and after a moment shutting it down
  if(*state==0)
    {
      digitalWrite(PIFACE_BASE+light,1); //RED light
    }
  else if (*state==1)
    {
      digitalWrite(PIFACE_BASE + light +1,1);  //YELLOW light
      if(lightTick==PASS_TIME + CHANGE_TIME-1)
	{
	  digitalWrite(PIFACE_BASE + light +1,0);
	  digitalWrite(PIFACE_BASE + light +2,0);
	}
    }
  else if (*state==2)
    {
      digitalWrite(PIFACE_BASE + light,0);
      digitalWrite(PIFACE_BASE + light +1,0);
      digitalWrite(PIFACE_BASE + light +2,1); //GREEN light
      
    }
}

int main()
{
  wiringPiSetupSys();       //Set-up wiringPi
  piFaceSetup(PIFACE_BASE);
  int pressed=0;  //Indicates if buttons are pressed
  int power=1;    //For main loop
  int st=0;
  int *state=&st; //0 = red lights, 1 = yellow, 2=green
  int lightTick=0;  //For time counting
  int waitTick=0;   //Time counting for sound effect
  int i = 0;
  int light = 2;//LED number
  int beep = 0;
  //Set all off and enable internal pull-ups
  for(i;i<8;i++)
    {
      pullUpDnControl(PIFACE_BASE + i, PUD_UP);
      digitalWrite(PIFACE_BASE + i, 0);
    }
  printf("Traffic light control program v 0.1\n");
  printf("Press button 0 for pedestrian lights. Press button 3 to close program\n");
  printf("Lights:RED\n");
  while(power==1)
    {
	  
      lights(state,light,lightTick);  //Manage LEDs
      beeper(state,beep,waitTick);      //Create sound effect with relay
      int push = readButton(0); // Read state of the button
      // printf("pu:%d pr:%d lt:%d st:%d\n",push,pressed,lightTick,*state);
      if(push == 1 && pressed == 0)
	{
          pressed=1;
	  printf("Pedestrian waiting\n");
	}

      if(pressed==1)
	{
	  lightTick++;  //Increment time
	}
      delay(500);   //Slow things down
      waitTick++;   //Increment beeper timer
      if(lightTick==WAIT_TIME)
	{
	  *state=1;  //Change to yellow
	  printf("Lights:YELLOW\n");
	}

      if(*state==1 && lightTick== WAIT_TIME + CHANGE_TIME)
	{
	  *state=2;  //Change to green
	  printf("Lights:GREEN\n");
	}
      if(*state==2 && lightTick== PASS_TIME)
	{
	  *state=1;  //Back to yellow
	  printf("Lights:YELLOW\n");
	}
      if(*state==1 && lightTick== PASS_TIME + CHANGE_TIME)
	{
	  *state=0;  //Back to red
	  printf("Lights:RED\n");
	  lightTick=0;
	  pressed=0;
	}
      //SHUTDOWN
      if(readButton(3)==1)
	{
	  power=0;//Ends while loop
	  int i=0;
	  //Close down all LEDs
	  for(i;i<8;i++)
	    {
	      digitalWrite(PIFACE_BASE+i,0);
	    }
	  printf("Emergency shutdown!\n");
	}
    }
  return 0;
}
