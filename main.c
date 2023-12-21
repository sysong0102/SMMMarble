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

static int player_nr; // �÷��̾� �� 

//�÷��̾��� ����üȭ 
typedef struct player
{
	int energy;
	int position;
	char name[MAX_CHARNAME];
	int accumCredit; //���� ���� 
	int flag_graduate; //������ �Ǵ� ���� 
	int threshold; // ����� ç���� ���ذ� 
	int Lab_flag; //����� Ż�� �Ǵ� ���� 
	
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

const char* gradeStrings[] = {"A+", "A0", "A-", "B+", "B0", "B-", "C+", "C0", "C-"}; //������ ���ڿ��� ��Ÿ�� �迭 
const float gradeScores[] = {4.3, 4.0, 3.7, 3.4, 3.1, 2.8, 2.5, 2.2, 1.9}; //������ float������ ��Ÿ�� �迭 

int isGraduated(void) //������ �л��� �ִ��� �����ϴ� �Լ�  
{
	int i;
	for(i=0;i<player_nr;i++)
	{
		if((cur_player[i].accumCredit >= GRADUATE_CREDIT) && (cur_player[i].position == 0))//�������� �̻��� ä�� �л��� ���� �����ϸ� ��������  
		{
			cur_player[i].flag_graduate = 1; //������ �л��� �ִٸ� �� �л��� flag_graduate�� 1�� ��ȯ�Ͽ� ���� 
			return cur_player[i].flag_graduate; //flag_graduate �� ��ȯ
		}
	}	
	return 0;
	 
}

float calcAverageGrade(int player) // ���� ��� ��� �Լ� 
{
	float TotalGrade = 0; 
	int course_nr = smmdb_len(LISTNO_OFFSET_GRADE + player); //������ ���� ���� ��Ÿ���� ����  
	//TotalGrade = ��ü ���� �� % smmdb_len(LISTNO_OFFSET_GRADE + player);
	if(course_nr == 0) //������ ������ ���� ��� �״�� 0��ȯ  
	{
		return 0;
	}
	else // ������ ������ ���� ��� ��� ��� �� ��ȯ  
	{
		int i;
		for (i = 0; i < course_nr; i++) // ������ ���� ����ŭ �ݺ� 
		{
      	  	void *gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
      	  	smmObjGrade_e grade = smmObj_getNodeGrade(gradePtr);
        	float grade_Value = gradeScores[grade]; //gradeScores�迭���� grade�� ������ ��ġ�� �ִ� ���� ȹ�� 
        	//float gradeValue = smmObj_getGradeValue(smmObj_getNodeGrade(gradePtr));
        	TotalGrade += grade_Value; //��ü ������ ���ϱ� 
    	}
    	float average_of_grade = TotalGrade / course_nr; // ��� ��� 
    	return (average_of_grade); // ��� ��ȯ 
	}
	
}


void* findGrade(int player, char *lectureName) // ������ ������ ���� ã�� �Լ�  
{
	int i;
	for (i = 0; i < smmdb_len(LISTNO_OFFSET_GRADE + player); i++) //�ش� �÷��̾��� ���� ����Ʈ ��ȯ 
	{
        void *gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i); 
        
        if (strcmp(smmObj_getNodeName(gradePtr), lectureName) == 0) // ���� ���� ��ü�� ���� �̸��� �÷��̾ ������ ������ �̸��� ���� ��� 
		{
            return gradePtr; // ���� ��ü ������ ��ȯ 
        }
		return NULL; // ��ġ�ϴ� ���Ǹ��� ���� ��� NULL��ȯ  
	}    
}

void printGrades(int player) // ���� ����ϴ� �Լ� 
{
	int i;
	void *gradePtr;
	for(i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++)
	{
		gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i); //Ư�� �ε����� ���� ��ü�� ���� 
		printf("=> %s (credit:%i) : %s\n", smmObj_getNodeName(gradePtr), smmObj_getNodeCredit(gradePtr), gradeStrings[smmObj_getNodeGrade(gradePtr)]);
 	}
} //� ���Ǹ� ����� �� Ȯ�� ���� -> �� ������ ������� �ٽ� �ȵ赵�� �ϴ� ���ǹ� �̿�  


void printPlayerStatus(void)//�÷��̾��� ���¸� ����ϴ� �Լ�  
{
	int i;
	
	printf("========================== PLAYER STATUS ==========================\n");
    for(i=0;i<player_nr;i++)
    {
    	if(cur_player[i].Lab_flag==1) // �������϶��� ǥ���ϱ� 
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
		cur_player[i].flag_graduate = 0; // �̰� 0�� ���� ���� ��� ���� 
	}
	
}



int rolldie(int player) // �ֻ����� ������ �Լ�  
{
    char c;
    printf(" This is %s's turn :::: Press any key to roll a die (press g to see grade): ", cur_player[player].name);
    scanf("%s", &c); // �÷��̾ �ֻ����� ����  
    printf("\n");
    fflush(stdin);
    

    if (c == 'g') // 'g'�� �Է����� ��� 
    {
    	int i;
        printGrades(player); // ���� ���  �Լ� ȣ�� 
        float average_grade = calcAverageGrade(player);//���� ��� ���ϴ� �Լ� ȣ�� 
       	printf(" --> Printing player %s's grade (average %.6f) ::::\n", cur_player[player].name, average_grade); // ���� ��� ��� -> �Լ� ���  
 	
	}
	
	int roll_die_result; 
	roll_die_result = rand()%MAX_DIE + 1; // 0~5������ ������ �ϳ� ���� �� 1���ϱ� -> 1~6�� �ϳ� ���õ� 
	
	printf("--> result : %i\n", roll_die_result);

    return (roll_die_result); // �ֻ��� ����� ��ȯ  
}


void goForward(int player, int step) // ���� �Լ� 
{
   	int j;
	for (j = 1; j < step+1; j++) 
	{
		if(j != step)//�̵��ϴ� ���� ��Ÿ���� 
		{
			void *boardPtr;        	 
        	if(cur_player[player].position == smmdb_len(LISTNO_NODE)-1) //�ѹ��� ���� �ٽ� ��ȣ �ʱ�ȭ ��Ű�� 
   			{
   				cur_player[player].position = 0;
   				isGraduated();
   				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));	
			}
			else
			{
				cur_player[player].position += 1; // ��ĭ�� ����
				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));
			}
		}
		else // ���� ĭ �����ϱ� 
		{
			void *boardPtr;
        	if(cur_player[player].position == smmdb_len(LISTNO_NODE)-1) //�ѹ��� ���� �ٽ� ��ȣ �ʱ�ȭ ��Ű�� 
   			{
    			cur_player[player].position = 0;
				isGraduated();
   				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));
			}
			else
			{
				cur_player[player].position += 1; //��ĭ ���� 
				boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
				printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));
			}
    		printf("%s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
		}
    }
	
	
}

smmObjGrade_e takeLecture(int player, char *lectureName, int type, int credit, int energy) // ���� ��� �Լ�  
{
	
	if(cur_player[player].flag_graduate == 0) 
	{
       	if(cur_player[player].energy >= energy) //���� �������� �ҿ信���� �̻� �ִ� ��� 
       	{
       		void* foundGrade = findGrade(player, lectureName);
			if(foundGrade == NULL) //������ ���� ���� ���  
			{
				int flag_play = 1; 
				while(flag_play == 1) // ����ε� �Է��� ���������� �ݺ� 
				{
					char input[100]; // ���� �Է¹޴� ���� ����  
						
					printf(" %s %s (credit:%i, energy:%i) starts! are you going to join? or drop? :", lectureName, smmObj_getTypeName(type), credit, energy);		
					scanf("%s", input);
		
					
					if(strcmp(input, "join") == 0) // join�� �Է� �޾��� ��� 
					{
						//�÷��̾��� ������ ������ ������ �߰���, �������� �Ҹ�� 
						cur_player[player].accumCredit += credit;
						cur_player[player].energy -=  energy;
						//������ �������� ����
						void *gradePtr = smmObj_genObject(lectureName, smmObjType_grade ,0, credit, 0, rand()%(smmObjGrade_Cm+1));
       					smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
       							
       					printf(" %s successfully takes the lecture %s with grade %s (average : %i), remained energy : %i\n", cur_player[player].name, lectureName, gradeStrings[smmObj_getNodeGrade(gradePtr)], 0 , cur_player[player].energy);
						//grade average ���ϱ�  
						flag_play = 0;
					}	
					else if(strcmp(input, "drop") == 0) //drop�� �Է¹޾��� �� 
					{
						//���� ĭ���� �Ѿ 
						printf("%s dropped the lecture.\n", cur_player[player].name); 
						printf("-> Player %s drops the lecture %s!\n", cur_player[player].name, lectureName);
						flag_play = 0;
					}	
					else //join�� drop �� �� �ϳ��� �Է¹��� ������ ��� 
					{
						//�ٽ� �Է� �ȳ� ��Ʈ 
						printf("-> invalid input! input \"drop\" or \"join\"!\n");
					}
				}	
							
			}
			else //������ �� �ִ� ���
       		{	//������ �� ���� 
       			printf("%s cannot take classes because %s has a history of taking classes in the past\n", cur_player[player].name, cur_player[player].name);
			}
		}
		else // �������� �����Ͽ� ������ �� ���� 
		{
			printf("-> %s is too hungry to take the lecture %s (remained:%i, required:%i)\n)",cur_player[player].name, lectureName, cur_player[player].energy, energy );
		}
	}
}

int do_experiment(int player, int threshold, int energy) // ����ǿ� ������ ��� 
{
	if(cur_player[player].flag_graduate == 0) 
	{
		if(cur_player[player].Lab_flag==1) // ����ð� ĭ���� �̹� 1�� ����� -> �� 1�� ��쿡�� ����ð� ĭ�� ���� �°����� �����ϰ� ���� ���� 
    	//�ֻ��� ������ ���ذ� �̻��� �Ǹ� Ż��
   		{
			
			int challenge_value = rand()%MAX_DIE+1; //������ ���� �̱�  
			printf("-> Experiment time! Let's see if you can satisfy professor (threshold: %i)\nPress any key to roll a die (press g to see grade):", cur_player[player].threshold); 
			getchar();
		
   			if(challenge_value >= cur_player[player].threshold)//�������� ���ذ� �̻��� ��� 
   			{
   				//Ż�� 
   				cur_player[player].energy -= energy; // �������� ���ҵ� �� Ż�� 
   				printf("-> Experiment result : %i, success! %s can exit this lab!\n", challenge_value, cur_player[player].name);
       			cur_player[player].Lab_flag = 0; //0���� �ٲ㼭 Ż���Ŵ 
			}
			else // ���ذ��� ���� ������ ��� 
			{
				//Ż�� ���� 
				cur_player[player].energy -= energy; // ������ ���� 
		
				printf("-> Experiment result : %i, fail T_T. %s needs more experiment......\n", challenge_value, cur_player[player].name);
			}
		}	
		else // �׳� ���ڰ��н���Ƿ� �ٷ� ���� ��� - �׳� ����� 
		{
			printf("-> This is not experiment time. You can go through this lab.\n");
		}
	}
	
}

void RandFestGame(int player) // �佺Ƽ�� ī�� �̱� 
{
	if(cur_player[player].flag_graduate == 0)
	{
		
		printf(" -> mom participates to Snow Festival! press any key to pick a festival card:");
		getchar();
	
		int Fest_Card = rand()%smmdb_len(LISTNO_FESTCARD); //�������� ������ ī�� ���� 
		void *festivalPtr = smmdb_getData(LISTNO_FESTCARD, Fest_Card);
		cur_player[player].energy +=  smmObj_getNodeEnergy(festivalPtr); 
		
		printf("-> MISSION : %s !!\n(Press any key when mission is ended.)\n", smmObj_getNodeName(festivalPtr));
		getchar(); // �ƹ� ���̳� �Է¹��� �� ���� 
		
	}
		
}

void RandFoodGame(int player) // ���� ���� ī�� �̱� 
{
	if(cur_player[player].flag_graduate == 0)
	{
			printf(" -> %s gets a food chance! press any key to pick a food card: ",cur_player[player].name);
	getchar();
	
	int Food_Card = rand()%smmdb_len(LISTNO_FOODCARD); // �������� ������ ī�� ���� 
	void *foodPtr = smmdb_getData(LISTNO_FOODCARD, Food_Card);
	cur_player[player].energy +=  smmObj_getNodeEnergy(foodPtr); // ���� ī���� ��������ŭ ���� 
		
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
        	//�Լ� ȣ�� 
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
			//�����ĭ���� �̵�
			if(cur_player[player].flag_graduate == 0)
			{
				int i;
				for (i = 0; i < smmdb_len(LISTNO_NODE); i++) //�ش� �÷��̾��� ��� ����Ʈ ��ȯ 
				{
  					void *boardPtr = smmdb_getData(LISTNO_NODE, i); 
        
   		          	if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_LABORATORY) // ���� ��� ��ü�� Ÿ�԰� SMMNODE_TYPE_LABORATORY�� ���� ��  
					{
           				cur_player[player].position = i; //�÷��̾ ����Ƿ� �̵���Ŵ 
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
        
        //�Լ� ȣ�� 
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
			//food ���� �̱� �Լ� ȣ��  
			RandFoodGame(player);
			break;
        }	
        
        //case Festival:
		case SMMNODE_TYPE_FESTIVAL:
		{
			//festival �̼� ���� �Լ� ȣ�� 
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
    
    while (fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set, �ݺ��� �ѹ� ���� �� ���� ���� 
    {
        //store the parameter set
        void *boardObj = smmObj_genObject(name, smmObjType_board ,type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj); //0������ ���� ������ ��ü�� ������� 
		if (type == SMMNODE_TYPE_HOME)
		{
        	initEnergy = energy;
			
		}
        
        board_nr++;
    }
    fclose(fp);

    for(i = 0; i<board_nr;i++) //��� 
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
        smmdb_addTail(LISTNO_FOODCARD, foodObj); //0������ ���� ������ ��ü�� ������� 
        
        food_nr++;
    }
    fclose(fp);
    
    for(i = 0; i<food_nr;i++) // ��� 
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
        smmdb_addTail(LISTNO_FESTCARD, festivalObj); //0������ ���� ������ ��ü�� ������� 
        
        festival_nr++;
    }
    fclose(fp);
    
    
    for(i = 0; i<festival_nr;i++) // ��� 
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
        if (cur_player[turn].Lab_flag == 0) //�������� �ƴҶ�  
        {
       		die_result = rolldie(turn);
	    
        //4-3. go forward
      		 goForward(turn, die_result);

		//4-4. take action at the destination node of the board
        	actionNode(turn);
        
        }
        else
        {
        	void *boardPtr = smmdb_getData(LISTNO_NODE,cur_player[turn].position); // �������϶� 
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
