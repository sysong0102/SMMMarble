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

static int player_nr; // 플레이어 수 

//플레이어의 구조체화 
typedef struct player
{
	int energy;
	int position;
	char name[MAX_CHARNAME];
	int accumCredit; //누적 학점 
	int flag_graduate; //졸업자 판단 변수 
	int threshold; // 실험실 챌린지 기준값 
	int Lab_flag; //실험실 탈출 판단 변수 
	
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
smmObjGrade_e takeLecture(int player, char *name, int type, int credit, int energy); //take the lecture (insert a grade of the player)
//void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player

int do_experiment(int player, int threshold, int energy);
void RandFestGame(int player);
void RandFoodGame(int player);

const char* gradeStrings[] = {"A+", "A0", "A-", "B+", "B0", "B-", "C+", "C0", "C-"}; //학점을 문자열로 나타낸 배열 
const float gradeScores[] = {4.3, 4.0, 3.7, 3.4, 3.1, 2.8, 2.5, 2.2, 1.9}; //학점을 float형으로 나타낸 배열 

int isGraduated(void) //졸업한 학생이 있는지 점검하는 함수  
{
	int i;
	for(i=0;i<player_nr;i++)
	{
		if((cur_player[i].accumCredit >= GRADUATE_CREDIT) && (cur_player[i].position == 0))//졸업학점 이상을 채운 학생이 집에 도달하면 게임종료  
		{
			cur_player[i].flag_graduate = 1; //졸업한 학생이 있다면 그 학생의 flag_graduate를 1로 변환하여 저장 
			return cur_player[i].flag_graduate; //flag_graduate 값 반환
		}
	}	
	return 0;
	 
}

float calcAverageGrade(int player) // 학점 평균 계산 함수 
{
	float TotalGrade = 0; 
	int course_nr = smmdb_len(LISTNO_OFFSET_GRADE + player); //수강한 과목 수를 나타내는 변수  
	//TotalGrade = 전체 점수 합 % smmdb_len(LISTNO_OFFSET_GRADE + player);
	if(course_nr == 0) //수강한 과목이 없을 경우 그대로 0반환  
	{
		return 0;
	}
	else // 수강한 과목이 있을 경우 평균 계산 후 반환  
	{
		int i;
		for (i = 0; i < course_nr; i++) // 수강한 강의 수만큼 반복 
		{
      	  	void *gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
      	  	smmObjGrade_e grade = smmObj_getNodeGrade(gradePtr);
        	float grade_Value = gradeScores[grade]; //gradeScores배열에서 grade와 동일한 위치에 있는 점수 획득 
        	//float gradeValue = smmObj_getGradeValue(smmObj_getNodeGrade(gradePtr));
        	TotalGrade += grade_Value; //전체 점수에 더하기 
    	}
    	float average_of_grade = TotalGrade / course_nr; // 평균 계산 
    	return (average_of_grade); // 평균 반환 
	}
	
}


void* findGrade(int player, char *lectureName) // 수강한 강의의 점수 찾기 함수  
{
	int i;
	for (i = 0; i < smmdb_len(LISTNO_OFFSET_GRADE + player); i++) //해당 플레이어의 학점 리스트 순환 
	{
        void *gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i); 
        
        if (strcmp(smmObj_getNodeName(gradePtr), lectureName) == 0) // 현재 학점 개체의 강의 이름과 플레이어가 도착한 강의의 이름이 같을 경우 
		{
            return gradePtr; // 학점 객체 포인터 반환 
        }
		return NULL; // 일치하는 강의명이 없을 경우 NULL반환  
	}    
}

void printGrades(int player) // 학점 출력하는 함수 
{
	int i;
	void *gradePtr;
	for(i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++)
	{
		gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i); //특정 인덱스의 학점 객체를 얻음 
		printf("=> %s (credit:%i) : %s\n", smmObj_getNodeName(gradePtr), smmObj_getNodeCredit(gradePtr), gradeStrings[smmObj_getNodeGrade(gradePtr)]);
 	}
} //어떤 강의를 들었는 지 확인 가능 -> 이 과목을 들었으면 다시 안듣도록 하는 조건문 이용  


void printPlayerStatus(void)//플레이어의 상태를 출력하는 함수  
{
	int i;
	
	printf("========================== PLAYER STATUS ==========================\n");
    for(i=0;i<player_nr;i++)
    {
    	if(cur_player[i].Lab_flag==1) // 실험중일때는 표시하기 
    	{
    		void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[i].position);
			printf("%s at %i.%s(exp), credit: %i, energy: %i\n", cur_player[i].name, cur_player[i].position ,smmObj_getNodeName(boardPtr), cur_player[i].accumCredit, cur_player[i].energy );
		}
		else
		{
			void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[i].position);
			printf("%s at %i.%s, credit: %i, energy: %i\n", cur_player[i].name, cur_player[i].position ,smmObj_getNodeName(boardPtr), cur_player[i].accumCredit, cur_player[i].energy );
   		}
   	}
	printf("========================== PLAYER STATUS ==========================\n");
		
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



int rolldie(int player) // 주사위를 굴리는 함수  
{
    char c;
    printf(" This is %s's turn :::: Press any key to roll a die (press g to see grade): ", cur_player[player].name);
    scanf("%s", &c); // 플레이어가 주사위를 굴림  
    printf("\n");
    fflush(stdin);
    

    if (c == 'g') // 'g'를 입력했을 경우 
    {
    	int i;
        printGrades(player); // 학점 출력  함수 호출 
        float average_grade = calcAverageGrade(player);//학점 평균 구하는 함수 호출 
       	printf(" --> Printing player %s's grade (average %.6f) ::::\n", cur_player[player].name, average_grade); // 학점 평균 출력 -> 함수 출력  
 	
	}
	
	int roll_die_result; 
	roll_die_result = rand()%MAX_DIE + 1; // 0~5까지의 난수를 하나 뽑은 후 1더하기 -> 1~6중 하나 선택됨 
	
	printf("--> result : %i\n", roll_die_result);

    return (roll_die_result); // 주사위 결과값 반환  
}


void goForward(int player, int step) // 전진 함수 
{
   	int j;
	for (j = 1; j < step+1; j++) 
	{
		if(j != step)//이동하는 과정 나타내기 
		{
			void *boardPtr;        	 
        	if(cur_player[player].position == smmdb_len(LISTNO_NODE)-1) //한바퀴 돌면 다시 번호 초기화 시키기 
   			{
   				cur_player[player].position = 0;
   				isGraduated();
   				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));	
			}
			else
			{
				cur_player[player].position += 1; // 한칸씩 전진
				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));
			}
		}
		else // 최종 칸 도착하기 
		{
			void *boardPtr;
        	if(cur_player[player].position == smmdb_len(LISTNO_NODE)-1) //한바퀴 돌면 다시 번호 초기화 시키기 
   			{
    			cur_player[player].position = 0;
				isGraduated();
   				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));
			}
			else
			{
				cur_player[player].position += 1; //한칸 전진 
				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));
			}
    		printf("%s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
		}
    }
	
	
}

smmObjGrade_e takeLecture(int player, char *lectureName, int type, int credit, int energy) // 강의 듣기 함수  
{
	
	if(cur_player[player].flag_graduate == 0) 
	{
       	if(cur_player[player].energy >= energy) //현재 에너지가 소요에너지 이상 있는 경우 
       	{
       		void* foundGrade = findGrade(player, lectureName);
			if(foundGrade == NULL) //수강한 적이 없는 경우  
			{
				int flag_play = 1; 
				while(flag_play == 1) // 제대로된 입력을 받을때까지 반복 
				{
					char input[100]; // 문자 입력받는 변수 설정  
						
					printf(" %s %s (credit:%i, energy:%i) starts! are you going to join? or drop? :", lectureName, smmObj_getTypeName(type), credit, energy);		
					scanf("%s", input);
		
					
					if(strcmp(input, "join") == 0) // join을 입력 받았을 경우 
					{
						//플레이어의 학점에 강의의 학점이 추가됨, 에너지는 소모됨 
						cur_player[player].accumCredit += credit;
						cur_player[player].energy -=  energy;
						//성적이 랜덤으로 나옴
						void *gradePtr = smmObj_genObject(lectureName, smmObjType_grade ,0, credit, 0, rand()%(smmObjGrade_Cm+1));
       					smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
       							
       					printf(" %s successfully takes the lecture %s with grade %s (average : %i), remained energy : %i\n", cur_player[player].name, lectureName, gradeStrings[smmObj_getNodeGrade(gradePtr)], 0 , cur_player[player].energy);
						//grade average 구하기  
						flag_play = 0;
					}	
					else if(strcmp(input, "drop") == 0) //drop을 입력받았을 때 
					{
						//다음 칸으로 넘어감 
						printf("%s dropped the lecture.\n", cur_player[player].name); 
						printf("-> Player %s drops the lecture %s!\n", cur_player[player].name, lectureName);
						flag_play = 0;
					}	
					else //join과 drop 둘 중 하나를 입력받지 못했을 경우 
					{
						//다시 입력 안내 멘트 
						printf("-> invalid input! input \"drop\" or \"join\"!\n");
					}
				}	
							
			}
			else //수강한 적 있는 경우
       		{	//수강할 수 없음 
       			printf("%s cannot take classes because %s has a history of taking classes in the past\n", cur_player[player].name, cur_player[player].name);
			}
		}
		else // 에너지가 부족하여 수강할 수 없음 
		{
			printf("-> %s is too hungry to take the lecture %s (remained:%i, required:%i)\n)",cur_player[player].name, lectureName, cur_player[player].energy, energy );
		}
	}
}

int do_experiment(int player, int threshold, int energy) // 실험실에 갇혔을 경우 
{
	if(cur_player[player].flag_graduate == 0) 
	{
		if(cur_player[player].Lab_flag==1) // 실험시간 칸에서 이미 1로 변경됨 -> 즉 1일 경우에만 실험시간 칸을 통해 온것으로 간주하고 실험 진행 
    	//주사위 굴려서 기준값 이상이 되면 탈출
   		{
			
			int challenge_value = rand()%MAX_DIE+1; //도전값 랜덤 뽑기  
			printf("-> Experiment time! Let's see if you can satisfy professor (threshold: %i)\nPress any key to roll a die (press g to see grade):", cur_player[player].threshold); 
			getchar();
		
   			if(challenge_value >= cur_player[player].threshold)//도전값이 기준값 이상일 경우 
   			{
   				//탈출 
   				cur_player[player].energy -= energy; // 에너지는 감소된 후 탈출 
   				printf("-> Experiment result : %i, success! %s can exit this lab!\n", challenge_value, cur_player[player].name);
       			cur_player[player].Lab_flag = 0; //0으로 바꿔서 탈출시킴 
			}
			else // 기준값을 넘지 못했을 경우 
			{
				//탈출 실패 
				cur_player[player].energy -= energy; // 에너지 감소 
		
				printf("-> Experiment result : %i, fail T_T. %s needs more experiment......\n", challenge_value, cur_player[player].name);
			}
		}	
		else // 그냥 전자공학실험실로 바로 왔을 경우 - 그냥 통과함 
		{
			printf("-> This is not experiment time. You can go through this lab.\n");
		}
	}
	
}

void RandFestGame(int player) // 페스티벌 카드 뽑기 
{
	if(cur_player[player].flag_graduate == 0)
	{
		
		printf(" -> mom participates to Snow Festival! press any key to pick a festival card:");
		getchar();
	
		int Fest_Card = rand()%smmdb_len(LISTNO_FESTCARD); //랜덤으로 한장의 카드 뽑음 
		void *festivalPtr = smmdb_getData(LISTNO_FESTCARD, Fest_Card);
		cur_player[player].energy +=  smmObj_getNodeEnergy(festivalPtr); 
		
		printf("-> MISSION : %s !!\n(Press any key when mission is ended.)\n", smmObj_getNodeName(festivalPtr));
		getchar(); // 아무 값이나 입력받은 후 종료 
		
	}
		
}

void RandFoodGame(int player) // 음식 찬스 카드 뽑기 
{
	if(cur_player[player].flag_graduate == 0)
	{
			printf(" -> %s gets a food chance! press any key to pick a food card: ",cur_player[player].name);
	getchar();
	
	int Food_Card = rand()%smmdb_len(LISTNO_FOODCARD); // 랜덤으로 한장의 카드 뽑음 
	void *foodPtr = smmdb_getData(LISTNO_FOODCARD, Food_Card);
	cur_player[player].energy +=  smmObj_getNodeEnergy(foodPtr); // 뽑은 카드의 에너지만큼 증가 
		
	printf("-> %s picks %s and charges %i (remained energy : %i)\n",cur_player[player].name, smmObj_getNodeName(foodPtr), smmObj_getNodeEnergy(foodPtr), cur_player[player].energy );
	
	}

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
        	//함수 호출 
        	takeLecture(player, name, type, smmObj_getNodeCredit(boardPtr), smmObj_getNodeEnergy(boardPtr));
        	
			break;
		}
        	
        	
        //case Restraurant:	
        case SMMNODE_TYPE_RESTAURANT:
        {
        	if (cur_player[player].flag_graduate == 0)
        	{
        		printf("-> Let's eat in %s and charge %i energies (remained energy : %i)\n",smmObj_getNodeName(boardPtr), smmObj_getNodeEnergy(boardPtr), cur_player[player].energy );
        		cur_player[player].energy +=  smmObj_getNodeEnergy(boardPtr);
        		printf("%s have arrived restaurant.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(boardPtr));
        		
	
			}
			break;
		
		}	
		
		//case Gotolab:
        case SMMNODE_TYPE_GOTOLAB:
		{
			//실험실칸으로 이동
			if(cur_player[player].flag_graduate == 0)
			{
				int i;
				for (i = 0; i < smmdb_len(LISTNO_NODE); i++) //해당 플레이어의 노드 리스트 순환 
				{
  					void *boardPtr = smmdb_getData(LISTNO_NODE, i); 
        
   		          	if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_LABORATORY) // 현재 노드 개체의 타입과 SMMNODE_TYPE_LABORATORY이 같을 때  
					{
           				cur_player[player].position = i; //플레이어를 실험실로 이동시킴 
        			}	
				}
				cur_player[player].threshold = rand()%MAX_DIE+1;
     			
     			cur_player[player].Lab_flag = 1;
				printf("OMG! This is experiment time!! Player %s goes to the lab.\n", cur_player[player].name);
				break;
			}	
			
		}	
        
        //case Laboratory:
        case SMMNODE_TYPE_LABORATORY:
        {	
        
        //함수 호출 
			do_experiment(player, cur_player[player].threshold,smmObj_getNodeEnergy(boardPtr) );
        	break;
		}
        
        //case Home:
        case SMMNODE_TYPE_HOME:
        {
			if(cur_player[player].flag_graduate == 0)
			{
				printf("%s have arrived home.\nYou can replenish %d energy :)\n", cur_player[player].name, smmObj_getNodeEnergy(boardPtr));
        		cur_player[player].energy +=  smmObj_getNodeEnergy(boardPtr);
        	
			}
        	
        	break;
        }
		
		
		//case Foodchance:
		case SMMNODE_TYPE_FOODCHANCE:
		{
			//food 랜덤 뽑기 함수 호출  
			RandFoodGame(player);
			break;
        }	
        
        //case Festival:
		case SMMNODE_TYPE_FESTIVAL:
		{
			//festival 미션 수행 함수 호출 
			RandFestGame(player);
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
    
    //srand(time(NULL));
    
    
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

    for(i = 0; i<board_nr;i++) //출력 
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
    
    for(i = 0; i<food_nr;i++) // 출력 
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
    while (fscanf(fp, "%s", name) == 1) //read a food parameter set
    {
        //store the parameter set
        void *festivalObj = smmObj_genObject(name, smmObjType_card, SMMNODE_TYPE_FESTIVAL, 0, 0, 0);
        smmdb_addTail(LISTNO_FESTCARD, festivalObj); //0번에다 새로 생성한 객체를 집어넣음 
        
        festival_nr++;
    }
    fclose(fp);
    
    
    for(i = 0; i<festival_nr;i++) // 출력 
    {
    	void *festivalObj =smmdb_getData(LISTNO_FESTCARD, i); 
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
	while (!isGraduated()) //is anybody graduated?
    {        
    
		srand(time(NULL));  
		int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        if (cur_player[turn].Lab_flag == 0) //실험중이 아닐때  
        {
       		die_result = rolldie(turn);
	    
        //4-3. go forward
      		 goForward(turn, die_result);

		//4-4. take action at the destination node of the board
        	actionNode(turn);
        
        }
        else
        {
        	void *boardPtr = smmdb_getData(LISTNO_NODE,cur_player[turn].position); // 실험중일때 
        	do_experiment(turn, cur_player[turn].threshold,smmObj_getNodeEnergy(boardPtr) );
		}
        //4-5. next turn
        turn = (turn + 1) %player_nr;
    }
   	
   	 //ending
    printf("==============================================\n");
    printf("----------------------------------------------\n");
    printf("              !!!Game End!!!                 \n");
    printf("----------------------------------------------\n");
    printf("==============================================\n");
    
   

    free(cur_player);
    system("PAUSE");
    
    
    return 0;
}
