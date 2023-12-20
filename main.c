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
	int threshold;
	
}player_t;

static player_t *cur_player;

//static player_t cur_player[MAX_PLAYER];


#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif

//function prototypes

int isGraduated(void); //check if any player is graduated
void printGrades(int player); //print grade history of the player
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
float calcAverageGrade(int player); //calculate average grade of the player
smmObjGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player


const char* gradeStrings[] = {"A+", "A0", "A-", "B+", "B0", "B-", "C+", "C0", "C-"};

int isGraduated(void)
{
	int i;
	for(i=0;i<player_nr;i++)
	{
		if(cur_player[i].accumCredit >= GRADUATE_CREDIT)
		{
			return 1;
		
		}
	}	
	return 0;
}

float calcAverageGrade(int player)
{
	float TotalGrade = 0;
	
}

void printGrades(int player)
{
	int i;
	void *gradePtr;
	for(i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++)
	{
		gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
		printf("=> %s (credit:%i) : %s\n", smmObj_getNodeName(gradePtr), smmObj_getNodeCredit(gradePtr), gradeStrings[smmObj_getNodeGrade(gradePtr)]);

} //어떤 강의를 들었는 지 확인 가능 -> 이 과목을 들었으면 다시 안듣도록 하는 조건문 이용  


void printPlayerStatus(void)
{
	int i;
	
	for (i=0;i<player_nr;i++)
	{
		printf("========================== PLAYER STATUS ==========================\n");
    	for(i=0;i<player_nr;i++)
		printf("%s at %i.%s, credit: %i, energy: %i", cur_player[i].name, cur_player[i].position ,smmObj_getNodeName(cur_player[i].position), cur_player[i].accumCredit, cur_player[i].energy );
    	printf("========================== PLAYER STATUS ==========================\n");
   	
		
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
		cur_player[i].flag_graduate = 0; // 이게 0일 동안 게임 계속 진행 
	}
	
}



int rolldie(int player)
{
    char c;
    printf(" This is %s's turn :::: Press any key to roll a die (press g to see grade): ", cur_player[player].name);
    c = getchar();
    fflush(stdin);
    
#if 1
    if (c == 'g')
    {
    	int i;
        printGrades(player);
        float average_grade = calcAverageGrade(player);
       	printf(" --> Printing player %s's grade (average %.6f) ::::\n", cur_player[player].name, average_grade);
 		printGrades(player);
	}
	int i;
	int roll_die_result = rand()%MAX_DIE + 1;
	printf("--> result : %i", roll_die_result);
	for (i = 1; i < roll_die_result+1; i++) 
	{
        printf("  => Jump to %s\n", smmObj_getNodeName(cur_player[player].position + i));
    }		 
    

#endif
    
    return (roll_die_result);
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
    if(cur_player[player].position > 15)
    {
    	cur_player[player].position -= 16;
	}
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
        	{
        		if(cur_player[player].flag_graduate==1)
        		{
        			if(cur_player[player].energy < smmObj_getNodeEnergy(boardPtr)) //현재 에너지가 소요에너지 이상 있는 경우 
        			{
        				
						if(1) //수강한 적 있는 경우
        				{
        					printf("%s cannot take classes because %s has a history of taking classes in the past\n", cur_player[player].name);
						}
						
						else
						{
							int flag_play = 1;
							while(flag_play == 1)
							{
								char input[50];
							
								printf(" %s %s (credit:%i, energy:%i) starts! are you going to join? or drop? :", smmObj_getNodeName(boardPtr), smmObj_getTypeName(smmObj_getNodeType(boardPtr)), smmObj_getNodeCredit(boardPtr), smmObj_getNodeEnergy(boardPtr));
							
								scanf("%s", &input);
					
								if(strcmp(input, "join") == 0)
								{
									//플레이어의 학점에 강의의 학점이 추가됨, 에너지는 소모됨 
									cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
									cur_player[player].energy -=  smmObj_getNodeEnergy(boardPtr);
									//성적이 랜덤으로 나옴
									gradePtr = smmObj_genObject(name, smmObjType_grade ,0, smmObj_getNodeCredit(boardPtr), 0, rand()%(smmObjGrade_Cm+1));
        							smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
        						
        							printf(" %s successfully takes the lecture %s with grade %s (average : %i), remained energy : %i)", cur_player[player].name, smmObj_getNodeName(boardPtr), gradeStrings[smmObj_getNodeGrade(gradePtr)], 0 , cur_player[player].energy);
									//grade average 구하기  
									flag_play = 0;
								}
								else if(strcmp(input, "drop") == 0)
								{
									//다음 칸으로 넘어감 
									printf("%s dropped the lecture.", cur_player[player].name); 
									printf("-> Player %s drops the lecture %s!", cur_player[player].name, smmObj_getNodeName(boardPtr));
									flag_play = 0;
								}
								else
								{
									printf("-> invalid input! input \"drop\" or \"join\"!");
								}
							}
								
						}
					}
					else
					{
						printf("-> %s is too hungry to take the lecture %s (remained:%i, required:%i)",cur_player[player].name, smmObj_getNodeName(boardPtr), cur_player[player].energy, smmObj_getNodeEnergy(boardPtr) );
					}
				break;
			}
        	
        	
        	
        case SMMNODE_TYPE_RESTAURANT:
        {
        	if (cur_player[player].flag_graduate == 0)
        	{
        		printf("-> Let's eat in %s and charge %i energies (remained energy : %i)",smmObj_getNodeName(boardPtr), smmObj_getNodeEnergy(boardPtr), cur_player[player].energy );
        		cur_player[player].energy +=  smmObj_getNodeEnergy(boardPtr);
        		printf("%s have arrived restaurant.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(boardPtr));
        		
	
			}
			break;
		
		}	
		
        case SMMNODE_TYPE_GOTOLAB:
		{
			//실험실칸으로 이동
			
			cur_player[player].threshold = rollDice();
     		cur_player[player].position = 8;
     		cur_player[player].flag_graduate = 1;
			
     		printf("%s go to %s)\n", cur_player[player].name, smmObj_getNodeName(boardPtr));
			printf("OMG! This is experiment time!! Player %s goes to the lab.", cur_player[player].name);
			break;
		}	
        
        case SMMNODE_TYPE_LABORATORY:
        {
        	if(cur_player[player].flag_graduate==1)
        	//주사위 굴려서 기준값 이상이 되면 탈출
        	{
				
				int challenge_value = rollDice();
				int rolldice_num;
				printf("-> Experiment time! Let's see if you can satisfy professor (threshold: %i)\nPress any key to roll a die (press g to see grade):", cur_player[player].threshold); 
				scanf("%i", &rolldice_num);
        		if(challenge_value >= cur_player[player].threshold)
        		{
        			//탈출 
        			cur_player[player].energy -=  smmObj_getNodeEnergy(boardPtr);
        			printf("-> Experiment result : %i, success! %s can exit this lab!", challenge_value, cur_player[player].name);
        			
				}
				else
				{
					cur_player[player].energy -=  smmObj_getNodeEnergy(boardPtr);
					cur_player[player].flag_graduate = 0;
					printf("-> Experiment result : %i, fail T_T. %s needs more experiment......", challenge_value, cur_player[player].name);
        			
				}
			}	
			else
			{
				printf("-> This is not experiment time. You can go through this lab.");
			}	
			
        	break;
		}
        
        	
        case SMMNODE_TYPE_HOME:
        {
			
        	printf("%s have arrived home.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(boardPtr));
        	cur_player[player].energy +=  smmObj_getNodeEnergy(boardPtr);
        	break;
        }
		
		
			
		case SMMNODE_TYPE_FOODCHANCE:
		{
			int food_card_choose;
			printf(" -> %s gets a food chance! press any key to pick a food card: ",cur_player[player].name);
			scanf("%i", &food_card_choose);
			
			int Food_Card = rand()%(food_nr);
			void *foodPtr = smmdb_getData(LISTNO_FESTCARD, Food_Card);
			cur_player[player].energy +=  smmObj_getNodeEnergy(foodPtr);
			
			printf("-> %s picks %s and charges %i (remained energy : %i)",cur_player[player].name, smmObj_getNodeName(foodPtr), smmObj_getNodeEnergy(foodPtr), cur_player[player].energy );
			break;
        }	
        
		case SMMNODE_TYPE_FESTIVAL:
		{
			int festival_card_choose;
			int fest_mission_answer;
			printf(" -> mom participates to Snow Festival! press any key to pick a festival card:");
			scanf("%i", &festival_card_choose);
			
			int Fest_Card = rand()%(festival_nr);
			void *festivalPtr = smmdb_getData(LISTNO_FESTCARD, Fest_Card);
			cur_player[player].energy +=  smmObj_getNodeEnergy(festivalPtr);
			
			printf("-> MISSION : %s !!\n(Press any key when mission is ended.)", smmObj_getNodeName(festivalPtr));
			scanf("%s",fest_mission_answer );
			
			break;
		}
		

        default:
            break;
    }
}



int main(int argc, const char * argv[]) 
{
    
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
		{
        	initEnergy = energy;
			
		}
        
        board_nr++;
    }
    fclose(fp);

    for(i = 0; i<board_nr;i++)
    {
    	void *boardObj =smmdb_getData(LISTNO_NODE, i); 
    	printf("=> %i. %s, %i(%s), credit %i, energy %i\n", i, smmObj_getNodeName(boardObj), smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)), smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    
	}
	printf("Total number of board nodes : %i\n", board_nr);
	
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i", name, &energy) == 2) //read a food parameter set
    {
        //store the parameter set
        void *foodObj = smmObj_genObject(name, smmObjType_card, SMMNODE_TYPE_FOODCHANCE, 0, energy, 0);
        smmdb_addTail(LISTNO_FOODCARD, foodObj); //0번에다 새로 생성한 객체를 집어넣음 
        
        food_nr++;
    }
    fclose(fp);
    
    for(i = 0; i<food_nr;i++)
    {
    	void *foodObj =smmdb_getData(LISTNO_FOODCARD, i); 
    	printf("=> %i. %s, charge : %i\n", i, smmObj_getNodeName(foodObj), smmObj_getNodeEnergy(foodObj));
    
	}
    printf("Total number of food cards : %i\n", food_nr);
    
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s %i", name) == 1) //read a food parameter set
    {
        //store the parameter set
        void *festivalObj = smmObj_genObject(name, smmObjType_card, SMMNODE_TYPE_FESTIVAL, 0, 0, 0);
        smmdb_addTail(LISTNO_FESTCARD, festivalObj); //0번에다 새로 생성한 객체를 집어넣음 
        
        festival_nr++;
    }
    fclose(fp);
    
    
    for(i = 0; i<festival_nr;i++)
    {
    	void *festivalObj =smmdb_getData(LISTNO_FOODCARD, i); 
    	printf("=> %i. %s\n", i, smmObj_getNodeName(festivalObj));
    
	}
	printf("Total number of festival cards : %i\n", festival_nr);
    
    //opening
	printf("=======================================================================\n");
	printf("-----------------------------------------------------------------------\n");
	printf("        Sookmyung Marble !! Let's Graduate (total credit : 30)!!       \n");
	printf("-----------------------------------------------------------------------\n");
	printf("=======================================================================\n");
	
	//2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
        printf("Input No. of players (1 ~ 10) : "); 
    	scanf("%d", &player_nr);
    	fflush(stdin);
    	
    	
	}
	while(player_nr < 0 || player_nr > MAX_PLAYER);

	cur_player = (player_t*)malloc(player_nr*sizeof(player_t));
    generatePlayers(player_nr, initEnergy);
    

    //3. SM Marble game starts ---------------------------------------------------------------------------------
    int graduate_ox = 0;
	while (graduate_ox == 0) //is anybody graduated?
    {
		graduate_ox = isGraduated();
        
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
