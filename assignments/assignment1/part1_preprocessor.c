/* Main.c for Assignment 1, Q1 */

#include <stdio.h>

/************* STUDENT SETUP *************/

/* Adjust the following to be your student ID */

#define STUDENTID0 "B"
#define STUDENTID1  0
#define STUDENTID2  0
#define STUDENTID3  1
#define STUDENTID4  2
#define STUDENTID5  3
#define STUDENTID6  4
#define STUDENTID7  5
#define STUDENTID8  6

/* Comment out the following */
#error "ERROR: You have not yet done the required setup"

void print_pp_id(void){
  printf("Student ID: %s%d%d%d%d%d%d%d%d\n", STUDENTID0, STUDENTID1, STUDENTID2,
            STUDENTID3, STUDENTID4, STUDENTID5, STUDENTID6, STUDENTID7, STUDENTID8);
}

/*** PART 1-A: Pre-processor math ***/

#define ID_SUM \
    (STUDENTID1 + STUDENTID2 + STUDENTID3 + STUDENTID4 + \
     STUDENTID5 + STUDENTID6 + STUDENTID7 + STUDENTID8)

#define ID_WEIGHTED \
    (STUDENTID1*1 + STUDENTID2*2 + STUDENTID3*3 + STUDENTID4*4 + \
     STUDENTID5*5 + STUDENTID6*6 + STUDENTID7*7 + STUDENTID8*8)

void print_pp_math(void){
  printf("ID_SUM: %d\n", ID_SUM);
}

/*** PART 1-B: Pre-processor conditional statements ***/

#if (ID_SUM % 3) == 0
#define MODE "ALPHA"
#define MODE_ALPHA
#elif (ID_SUM % 3) == 1
#define MODE "BETA"
#define MODE_BETA
#else
#define MODE "GAMMA"
#define MODE_GAMMA
#endif

#ifdef MODE_ALPHA
#define STR_BUILD "Alpha mode build"
#elif defined(MODE_BETA)
#define STR_BUILD "Beta mode build"
#elif defined(MODE_GAMMA)
#define STR_BUILD "Gamma mode build"
#endif

/*** PART 1-C: Pre-processor macros ***/

#define CAT(a,b) a##b
#define CAT_EXPAND(a,b) CAT(a,b)
#define DIGIT(n) STUDENTID##n
#define MAKE_VAR(n) CAT_EXPAND(id_, DIGIT(n))

/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/
int main(void)
{
  int MAKE_VAR(5) = DIGIT(2)*200;

  print_pp_id();
  print_pp_math();

  while(1);
}

/*************************** End of file ****************************/
