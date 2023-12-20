//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_object_h
#define smm_object_h

#define SMMNODE_TYPE_LECTURE		0 
#define SMMNODE_TYPE_RESTAURANT		1
#define SMMNODE_TYPE_LABORATORY		2 
#define SMMNODE_TYPE_HOME			3
#define SMMNODE_TYPE_GOTOLAB		4 
#define SMMNODE_TYPE_FOODCHANCE		5 
#define SMMNODE_TYPE_FESTIVAL		6

#define SMMNODE_TYPE_MAX			7 //타입이 몇개인지  

#define MAX_CHARNAME                200 //문자열의 길이 제한 

typedef enum smmObjType
{
	smmObjType_board = 0,
	smmObjType_card,
	smmObjType_grade
	
}smmObjType_e;

typedef enum smmObjGrade
{
	smmObjGrade_Ap = 0,
	smmObjGrade_A0,
	smmObjGrade_Am,
	smmObjGrade_Bp,
	smmObjGrade_B0,
	smmObjGrade_Bm,
	smmObjGrade_Cp,
	smmObjGrade_C0,
	smmObjGrade_Cm
}smmObjGrade_e;

typedef enum smmObjNode 
{
	smmObjNode_lecture = 0,
    smmObjNode_restaurant,
    smmObjNode_laboratory,
    smmObjNode_home,
    smmObjNode_experiment,
    smmObjNode_foodChance,
    smmObjNode_festival
    
}smmObjNode_e;

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/


/* grade :
    A+,
    A0,
    A-,
    B+,
    B0,
    B-,
    C+,
    C0,
    C-

static char smmGradeName[SMMNODE_TYPE_MAX][MAX_CHARNAME] =
{
	"A+",
	"A0",
	"A-",
	"B+",
	"B0",
	"B-",
	"C+",
	"C0",
	"C-"
};
*/
/*
char* smmObj_getNodeName(int node_nr);
int smmObj_getNodeType(int node_nr);
int smmObj_getNodeCredit(int node_nr);
int smmObj_getNodeEnergy(int node_nr);
*/
//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade);

//member retrieving
char* smmObj_getNodeName(void* obj);
int smmObj_getNodeType(void* obj);
int smmObj_getNodeCredit(void* obj);
int smmObj_getNodeEnergy(void* obj);

//element to string
char* smmObj_getTypeName(int type);

char* smmObj_getGradeName(smmObjGrade_e grade);

#endif /* smm_object_h */
