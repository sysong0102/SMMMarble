//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"


//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;

typedef struct player
{
	int energy;
	int position;
	char name[MAX_CHARNAME];
	int accumCredit; //누적 학점 
	int flag_graduate;
}player_t;

static player_t *cur_player;

//static player_t cur_player[MAX_PLAYER];


#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif

//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
void printGrades(int player); //print grade history of the player
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
#endif

const char* gradeStrings[] = {"A+", "A0", "A-", "B+", "B0", "B-", "C+", "C0", "C-"};

void printGrades(int player)
{
	int i;
	void *gradePtr;
	for(i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++)
	{
		gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
		printf("%s : %s\n", smmObj_getNodeName(gradePtr),gradeStrings[smmObj_getNodeGrade(gradePtr)]);
	}
} //어떤 강의를 들었는 지 확인 가능 -> 이 과목을 들었으면 다시 안듣도록 하는 조건문 이용  


void printPlayerStatus(void)
{
	int i;
	
	for (i=0;i<player_nr;i++)
	{
		printf("%s : credit %i, position %i, energy %i\n", cur_player[i].name, cur_player[i].accumCredit, cur_player[i].position, cur_player[i].energy);
	}
}
void generatePlayers(int n, int initEnergy) //generate a new player
{
	int i;
	// n time loop
	
	for (i = 0; i<n; i++)
	{
	//input name
		printf("Input player %i's name:", i);
		scanf("%s", cur_player[i].name);
		fflush(stdin);
		
	//set position
		//player_position[i] = 0;
		cur_player[i].position = 0;
	//set energy
		//player_position[i] = initEnergy;
		cur_player[i].energy = initEnergy;
		cur_player[i].accumCredit = 0;
		cur_player[i].flag_graduate = 0;
	}
	
}



int rolldie(int player)
{
    char c;
    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);
    
#if 1
    if (c == 'g')
        printGrades(player);
#endif
    
    return (rand()%MAX_DIE + 1);
}

/*
int rollDice() {
    return rand() % MAX_DIE + 1; // 주사위를 굴리는 함수 
}
*/

/*
int do_experiment
*/

void goForward(int player, int step)
{
     void *boardPtr;
     cur_player[player].position += step;
     boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position );
     
     printf("%s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
}


  

//action code when a player stays at a node
void actionNode(int player)
{
	void *boardPtr = smmdb_getData(LISTNO_NODE,cur_player[player].position);
	//int type =smmObj_getNodeType(cur_player[player].position);
	int type = smmObj_getNodeType(boardPtr);
	char *name = smmObj_getNodeName(boardPtr);
	void *gradePtr;
	
    switch(type)
    {
        //case lecture:
        case SMMNODE_TYPE_LECTURE:
        	if(cur_player[player].energy < smmObj_getNodeEnergy(boardPtr)) //현재 에너지가 소요에너지 이상 있는 경우 
        		if(1) //수강한 적 있는 경우
        		{
        			printf("%s cannot take classes because %s has a history of taking classes in the past\n", cur_player[player].name);
				}
				else{
				
					int input_course_answer;
					printf("Do you want to take the course?\n");
					printf("Yes : Please input a number 1\nNo : Please input a number 0\n");
					printf("Yes or no? :");
					scanf("%d", &input_course_answer);
					
					if(input_course_answer == 1)
					{
						//플레이어의 학점에 강의의 학점이 추가됨, 에너지는 소모됨 
						cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
						cur_player[player].energy -=  smmObj_getNodeEnergy(boardPtr);
						//성적이 랜덤으로 나옴
						gradePtr = smmObj_genObject(name, smmObjType_grade ,0, smmObj_getNodeCredit(boardPtr), 0, rand()%(smmObjGrade_Cm+1));
        				smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
        				break;
					}
					else
						//다음 칸으로 넘어감 
						printf("%s dropped the lecture.", cur_player[player].name); 
						break;
				}
        	
        case SMMNODE_TYPE_RESTAURANT:
        {
        	printf("%s have arrived restaurant.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(boardPtr));
        	cur_player[player].energy +=  smmObj_getNodeEnergy(boardPtr);
        	break;
		}
        	
        	
        case SMMNODE_TYPE_LABORATORY:
        {
        	break;	
		}
        
        	
        case SMMNODE_TYPE_HOME:
        {
			
        	printf("%s have arrived home.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(boardPtr));
        	cur_player[player].energy +=  smmObj_getNodeEnergy(boardPtr);
        	break;
        }
		case SMMNODE_TYPE_GOTOLAB:
		{
			break;
		}
		
			
		case SMMNODE_TYPE_FOODCHANCE:
		{
				
			int Food_Card = rand()%(food_nr);
			void *FoodCardConfig = smmdb_getData(LISTNO_FESTCARD, Food_Card);
			cur_player[player].energy +=  smmObj_getNodeEnergy(FoodCardConfig);
			printf("%s have arrived home.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(FoodCardConfig));
        	break;
        }	
		case SMMNODE_TYPE_FESTIVAL:
		{
			int Fest_Card = rand()%(festival_nr);
			void *FestCardConfig = smmdb_getData(LISTNO_FESTCARD, Fest_Card);
			cur_player[player].energy +=  smmObj_getNodeEnergy(boardPtr);
			printf("%s have arrived festival.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(FestCardConfig));
        	break;
		}
		

		
			
        default:
            break;
    }
}



int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int initEnergy; 
    int turn = 0;
    int i;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while (fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set, 반복문 한번 돌때 한 줄을 받음 
    {
        //store the parameter set
        void *boardObj = smmObj_genObject(name, smmObjType_board ,type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj); //0번에다 새로 생성한 객체를 집어넣음 
		if (type == SMMNODE_TYPE_HOME)
        	initEnergy = energy;
        
        board_nr++;
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
    
    for(i = 0; i<board_nr;i++)
    {
    	void *boardObj =smmdb_getData(LISTNO_NODE, i); 
    	printf("node %i : %s, %i(%s), credit %i, energy %i\n", i, smmObj_getNodeName(boardObj), smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)), smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    
	}
    /*특정 유형을 찍고 싶으면 printf("(%s)", smmObj_getType */
    
    #if 0
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while () //read a food parameter set
    {
        //store the parameter set
        
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);
    
    
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while () //read a festival card string
    {
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);
    #endif  
    
    
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
        printf("input player number : ");
    	scanf("%d", &player_nr);
    	fflush(stdin);
	}
    while (player_nr < 0 || player_nr > MAX_PLAYER);

cur_player = (player_t*)malloc(player_nr*sizeof(player_t));
    generatePlayers(player_nr, initEnergy);
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);

		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        turn = (turn + 1) %player_nr;
    }
    free(cur_player);
    system("PAUSE");
    return 0;
}
