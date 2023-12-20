//
//  smm_node.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include "smm_common.h"
#include "smm_object.h"
#include <string.h>

#define MAX_NODETYPE    7
#define MAX_GRADE       9
#define MAX_NODE 		100

static char smmNodeName[SMMNODE_TYPE_MAX][MAX_CHARNAME] =
{
	"lecture",
	"restaurant",
	"laboratory",
	"home",
	"gotolab",
	"foodchance",
	"festival"
};

char* smmObj_getTypeName(int type) //�־��� Ÿ�Կ� �����ϴ� ��� �̸� ��ȯ 
{
	return (char*)smmNodeName[type];
}


//1. ����ü ���� ����
typedef struct smmObject
{
	char name[MAX_CHARNAME];
	smmObjType_e objType;
	int type;   //ĭ ���� 
	int credit; //���� 
	int energy; //������ 
	smmObjGrade_e grade; //��� 
}smmObject_t;

//2. ����ü ���� ���� 
//static smmObject_t smm_node[MAX_NODE];

#if 0 
static char smmObj_name[MAX_NODE][MAX_CHARNAME];
static int smmObj_type[MAX_NODE];
static int smmObj_credit[MAX_NODE];
static int smmObj_energy[MAX_NODE];
#endif

// static int smmObj_noNode=0;

//3. ���� �Լ� ���� 
//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade)
{
	
	smmObject_t* ptr;
	
	ptr = (smmObject_t*)malloc(sizeof(smmObject_t));
	
    strcpy(ptr->name, name);
    ptr->objType = objType;
    ptr->type = type;
    ptr->credit = credit;
    ptr->energy = energy;
    ptr->grade = grade;
    
    return ptr;
}

char* smmObj_getNodeName(void* obj)
{
	smmObject_t* ptr = (smmObject_t*)obj;
	return ptr->name;
}

int smmObj_getNodeType(void* obj)
{
	smmObject_t* ptr = (smmObject_t*)obj;
	return ptr->type;
}

int smmObj_getNodeCredit(void* obj)
{
	smmObject_t* ptr = (smmObject_t*)obj;
	return ptr->credit;
}

int smmObj_getNodeEnergy(void* obj)
{
	smmObject_t* ptr = (smmObject_t*)obj;
	return ptr->energy;
}

smmObj_getNodeGrade(void* obj) 
{
	smmObject_t*ptr = (smmObject_t*)obj;
	return ptr->grade;
}
#if 0


//member retrieving


//element to string
/*char* smmObj_getNodeName(smmNode_e type)
{
    return smmNodeName[type];
}

char* smmObj_getGradeName(smmGrade_e grade)
{
    return smmGradeName[grade];
}
*/
#endif
