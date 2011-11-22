#include "decisiontree.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

AttributeMap* map;
//attribute is range from 1 to attributenum. colcume 0 is the classLab
char ***rawData = NULL;
uint32_t **trainingData = NULL;
uint32_t **testData = NULL;
uint32_t **confusionMatrix = NULL;
TreeNode* innerNodeList[MAXLEVELNUM] = { NULL };
TreeNode* curInnerNode[MAXLEVELNUM] = { NULL };
TreeNode* leafList = NULL;
TreeNode* curLeaf = NULL;
int classNum;
int attributeNum;
int numberOfTrainingRecord;
int numberOfTestingRecord;
int isPrintResult;
char* trainingSetFile = NULL;
char* testingSetFile = NULL;
uint32_t totalLevel;
uint32_t differValue = 60;
void VisitTree(FILE*,TreeNode*);

void TestLeafList()
{
    printf("test leaf list: \n");
    TreeNode* tempList = leafList;
    while (tempList!=NULL)
    {
        int i;
        printf("level: %d\n", tempList->selfLevel);
        for (i = 0; i <= tempList->selfLevel; ++i)
        {
            printf("%d; ", tempList->pathAttributeName[i]);
        }
        printf("\n");
        tempList = tempList->nextLeaf;
    }
}

void  TestInnerNodeList()
{
    printf("test inner node list\n");
    TreeNode *tempList;
    int i, k;
    for (k = 0; k <= totalLevel; ++k){
        tempList = innerNodeList[k];

        printf("level: %d and index %d\n", tempList->selfLevel, k);
        while (tempList != NULL) {
            for (i = 0; i <= tempList->selfLevel; ++i)
            {
                printf("%d; ", tempList->pathAttributeName[i]);
            }

            tempList = tempList->nextInnerNode;
        }

        printf("\n\n");
    }
}

void TestRead()
{
    printf("%s, %s,classnum = %d, attributenum = %d, train = %d, test = %d", trainingSetFile, testingSetFile, classNum, attributeNum, numberOfTrainingRecord, numberOfTestingRecord);
    if (isPrintResult == 1)
    {
        printf(" print\n");
    }
    else
    {
        printf(" no print\n");
    }
}

void TestTrainData()
{
    printf("TestTrainingData");
    int i, j;
    for ( i = 1; i <= numberOfTrainingRecord; ++i)
    {
        for (j = 0; j <= attributeNum; ++j)
        {
            printf("%d   ", trainingData[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

void TestTestingData()
{
    printf("TestTestingData");
    int i,j;
    for (i = 1; i <= numberOfTestingRecord; ++i)
    {
        for (j = 0; j<= attributeNum; ++j)
        {
            printf("%d   ", testData[i][j]);
        }
        printf("\n");
    }

    printf("\n\n");
}

void TestMap()
{
    printf("test map\n");
    int j, i;

    for (j = 0; j <= attributeNum; ++j)
    {
        printf("attri: %s: has %d attrs\n", map[j].attributeName, map[j].attributeNum);
        if (map[j].isConsecutive == 0)
        {
        for (i = 0; i < map[j].attributeNum; ++i)
        {
            printf("%s; ", map[j].attributes[i]);
        }
        }
        else {
            for (i = 0; i < map[j].attributeNum; ++i)
            {
                printf("%d; ", map[j].attributeValue[i]);
            }
        }
        printf("\n");
    }

    printf("\n");
}

void TestRawData(int flag)
{
    printf("testRawdata");
    int num;
    if (flag == 1)
    {
        num = numberOfTrainingRecord;
    }
    else
    {
        num = numberOfTestingRecord;
    }
    int i, j;
    for (i = 1; i <= num; ++i)
    {
        for (j = 0; j <= attributeNum; ++j)
        {
            printf("%s      ",rawData[i][j] );
        }
        printf("\n");
    }

    printf("\n\n");
}

void TestVisitTree(FILE* fp,TreeNode* root)
{
    fprintf(fp,"tree View:\n");
    VisitTree(fp,root);
}

double GetErrors(float N,float e)
{
    double CF=0.25;
    double Coeff=0.47955625;

    if ( e < 1E-6 )
    {
	  return N * (1 - exp(log(CF) / N));
    }
    else
    if ( e < 0.9999 )
    {
	  double Val0 = N * (1 - exp(log(CF) / N));
      return Val0 + e * (GetErrors(N, 1.0) - Val0);
    }
    else
    if ( e + 0.5 >= N )
    {
	return 0.67 * (N-e)+e ;
    }
    else
    {
    double Pr = (e + 0.5 + Coeff/2
	        + sqrt(Coeff * ((e + 0.5) * (1 - (e + 0.5)/N) + Coeff/4)) )
             / (N + Coeff);
	return (N * Pr );
    }
}

int cmp ( const void *a , const void *b )
{
    return *(int*)a-*(int*)b;
}

// Calculates log2 of number.
double YuLog2(double n)
{
    // log(n)/log(2) is log2.
    return log( n ) / log( 2 );
}

int ConvertString2Number(char* str)
{
   int retNum = 0;
   int i;
   for (i = 0; str[i] != '\0'; ++i)
   {
       if (str[i] == '.')
       {
            if (str[i+1] > '5')
            {
                retNum += 1;
            }
            break;
       }
       retNum *= 10;
       retNum += (str[i] - '0');
   }

   return retNum;
}

int IsNumber(char* str)
{
   int ret = 1;

   int i = 0;
   for (i = 0; str[i] != '\0'; ++i)
   {
       if (str[i] == '.')
       {
           continue;
       }

       if (str[i] < '0' || str[i] > '9')
       {
           return 0;
       }
   }
   return ret;
}

void Read(int argc, char* argv[])
{
    int k;
    for (k = 1; k < argc; ++k)
    {
        if (strcmp(argv[k],"-r") == 0)
        {
            ++k;
            trainingSetFile = argv[k];
        }
        else if (!strcmp(argv[k], "-t"))
        {
            ++k;
            testingSetFile = argv[k];
        }
        else if (!strcmp(argv[k], "-d"))
        {
            ++k;
            attributeNum = ConvertString2Number(argv[k]);
        }
        else if (!strcmp(argv[k], "-c"))
        {
            ++k;
            classNum = ConvertString2Number(argv[k]);
        }
        else if (!strcmp(argv[k], "-s"))
        {
            ++k;
            numberOfTrainingRecord = ConvertString2Number(argv[k]);
        }
        else if (!strcmp(argv[k], "-m"))
        {
            ++k;
            numberOfTestingRecord = ConvertString2Number(argv[k]);
        }
        else if (!strcmp(argv[k], "-p"))
        {
            ++k;
            isPrintResult = ConvertString2Number(argv[k]);
        }
        else{
            printf("input error, please check you input\n");
        }
    }
}

void ConvertNum2Str(int num, char str[10])
{
    char temp[10];
    int i = 0;
    while (num > 0)
    {
        temp[i] = (num%10)+'0';
        num = num / 10;
        ++i;
    }
    int k = 0;
    --i;
    for (; i >= 0; )
    {
        str[k++] = temp[i--];
    }
    str[k] = '\0';
}

void InitMapName()
{
    int j;
    char str[10];

    strcpy(map[0].attributeName, "classLab");
    for (j = 1; j <= attributeNum; j++)
    {
        strcpy(map[j].attributeName, "attribute");
        ConvertNum2Str(j, str);
        strcat(map[j].attributeName, str);
    }
}

void MallocMemory()
{
    int i, j;
    //malloc raw data
    int numberOfRecords = numberOfTrainingRecord > numberOfTestingRecord ? numberOfTrainingRecord : numberOfTestingRecord;
    rawData = (char ***)malloc(sizeof(char**) * (numberOfRecords+1));
    if (!rawData)
    {
        fprintf(stderr, "malloc memory failed, abort\n");
        abort();
    }

    for (i = 0; i <= numberOfRecords; i++) {
        rawData[i] = (char **)malloc(sizeof(char *) * (attributeNum+1));
        if (!rawData[i]) {
            fprintf(stderr, "malloc memory failed, abort.\n");
            abort();
        }

        for (j = 0; j <= attributeNum; j++) {
            rawData[i][j] = (char *)malloc(MAXLEN);
            if (!rawData[i][j]) {
                fprintf(stderr, "malloc memory failed, abort.\n");
                abort();
            }
        }
    }

    //malloc training data
    trainingData = (uint32_t **)malloc(sizeof(uint32_t *) * (numberOfTrainingRecord+1));
    if (!trainingData) {
        fprintf(stderr, "malloc memory failed, abort.\n");
        abort();
    }

    for (i = 0; i <= numberOfTrainingRecord; i++) {
        trainingData[i] = (uint32_t *)malloc(sizeof(uint32_t) *(attributeNum+1));
        if (!trainingData[i]) {
            fprintf(stderr, "malloc memory failed, abort.\n");
                abort();
        }
    }

    /* malloc test_data */
    testData = (uint32_t **)malloc(sizeof(uint32_t *) * (numberOfTestingRecord+1));
    if (!testData) {
        fprintf(stderr, "malloc memory failed, abort.\n");
        abort();
    }

    for (i = 0; i <= numberOfTestingRecord; i++) {
        testData[i] = (uint32_t *)malloc(sizeof(uint32_t) * (attributeNum+1));
        if (!testData[i]) {
            fprintf(stderr, "malloc memory failed, abort.\n");
            abort();
        }
    }

    // malloc map, map[0] is the classLab, ant map[1 - attribute] is the attribute.
    map = (AttributeMap *)malloc(sizeof(AttributeMap) * (attributeNum+1));
    if (!map) {
        fprintf(stderr, "malloc memory failed, abort.\n");
        abort();
    }
    InitMapName();
}

//map.attributes range from 0 to num - 1
int MapAttribute2Num(int i, char *str)
{
	if (i > attributeNum) {
		return -1;
	}

	int count = 0;
	for (; count < map[i].attributeNum; count++) {
		if (strncmp(map[i].attributes[count], str, MAXLEN - 1) == 0) {
			return count;
		}
	}

	return -1;
}

void ConstructMap()
{
    struct str_list {
        char str[MAXLEN];
        struct str_list *next;
    } *list, *tail, *ptr;

    list = tail = ptr = NULL;
    int totalnum;
    int i = 0, j = 0, k = 0, num = 0;
    totalnum = numberOfTrainingRecord;
	/* analyse input data(raw data) */
	for (j = 0; j <= attributeNum; j++) {//0 is the classLab
		for (i = 1; i <= totalnum; i++) {
			ptr = list;
			while (ptr) {
				if (strncmp(ptr->str, rawData[i][j], MAXLEN-1) == 0) {
					break;
				}

				ptr = ptr->next;
			}

			if (!ptr) {
				ptr = (struct str_list *)malloc(sizeof(struct str_list));
				if (!ptr) {
					fprintf(stderr, "malloc memory failed, abort.\n");
					abort();
				}

				strncpy(ptr->str, rawData[i][j], MAXLEN - 1);
				ptr->next = NULL;
				if (list) {
					tail->next = ptr;
					tail = tail->next;
				}
				else {
					list = tail = ptr;
				}

				num++;
			}
		}

		if (num == 0) {
			fprintf(stderr, "impossible.\n");
			exit(-1);
		}

		/* assign the list to attribute_map */
		map[j].attributeNum = num;
        map[j].isConsecutive = 0;
		map[j].attributes = (char **)malloc(sizeof(char *) * num);
		if (!map[j].attributes) {
			fprintf(stderr, "malloc memory failed, abort.\n");
			abort();
		}

		ptr = list;
		k = 0;
		while (k < num && ptr) {
			map[j].attributes[k] = (char *)malloc(MAXLEN);
			if (!map[j].attributes[k]) {
				fprintf(stderr, "malloc memory failed, abort.\n");
				abort();
			}

			strncpy(map[j].attributes[k], ptr->str, MAXLEN - 1);
			k++;
			ptr = ptr->next;
		}

        if (j > 0)
        {
            if (map[j].attributeNum > 20 && IsNumber(map[j].attributes[0]) == 1) //Consecutive
            {
                map[j].isConsecutive = 1;
                int a;
                map[j].attributeValue = (int*) malloc(sizeof(int)*num);
                for (a = 0; a < num; ++a)
                {
                    map[j].attributeValue[a] = ConvertString2Number(map[j].attributes[a]);
                }
                //TestMap();
                qsort(map[j].attributeValue, map[j].attributeNum, sizeof(map[j].attributeValue[0]),cmp);
                //TestMap();
            }
        }

		/* free the list */
		while ((ptr = list)) {
			list = list->next;
			free(ptr);
		}
		list = tail = ptr = NULL;

		num = 0;
	}

}


int ConvertRawData2Map(int flag)
{
    int i, j;
	/* assign the training data and test data */
	for (j = 0; j <= attributeNum; j++) {
		i = 1;
		if (flag == 1) {
            while (i <= numberOfTrainingRecord)
            {
                if (map[j].isConsecutive == 1)
                {
                    trainingData[i][j] = ConvertString2Number(rawData[i][j]);
                }
                else
                {
                    trainingData[i][j] = MapAttribute2Num(j, rawData[i][j]);
                }
                i++;
            }
		}

        else if(flag == 2) {
            while (i<= numberOfTestingRecord)
            {
                if (map[j].isConsecutive == 1)
                {
                    testData[i][j] = ConvertString2Number(rawData[i][j]);
                }
                else
                {
    			    testData[i][j] = MapAttribute2Num(j, rawData[i][j]);
                }
	    		i++;
            }
		}
	}

	return 0;
}

void OnReadData(char* filename, int flag/*training or testing*/)
{
    FILE *fp = fopen(filename, "r+");

    if (!fp) {
        fprintf(stderr, "can't open file '%s', exit.\n", filename);
        exit(-1);
    }
    int i, j;
    int n = 1024;
    char buffer[1024];
    char *begin = NULL, *end = NULL;
    int totalnum;
    if (flag == 1)
    {
        totalnum = numberOfTrainingRecord;
    }
    else
    {
        totalnum = numberOfTestingRecord;
    }
    /* read all the data */
    i = 1;
    while (i <= totalnum) {
        char *ptr = fgets(buffer, n, fp);
        if (!ptr) {
            fprintf(stderr, "please add the attribute name!!!.\n");
            exit(-1);
        }
        begin = buffer;
        end = NULL;
        for (j =0; j < 1; ++j)
        {
            end = strchr(begin, (int)('\t'));
            if (end)
            {
                begin = end+1;
            }
            else if ( (end = strchr(begin, (int)(' '))) )
            {
                begin = end+1;
            }
            else
            {
                fprintf(stderr, "line tab wasn't found");
                exit(-1);
            }
        }
        //attribute is range from 1 to attributenum. colcume 0 is the classLab
        for (j = 0; j <= attributeNum-1; j++) {
            end = strchr(begin, (int)('\t'));
            if (!end)
            {
                end = strchr(begin, (int)(' '));
            }

            if (!end) {
                fprintf(stderr, "line 404 tab wasn't found.\n");
                exit(-1);
            }
            memset(rawData[i][j], 0, MAXLEN);
            strncpy(rawData[i][j], begin, end - begin);
            begin = end + 1;
        }
        end = strchr(begin, (int)('\r'));

        if (!end) {
            end = strchr(begin, (int)('\n'));
        }

        memset(rawData[i][j], 0, MAXLEN);
        if (end) {
            strncpy(rawData[i][j], begin, end - begin);
        }
        else {
            strcpy(rawData[i][j], begin);
        }

        i++;
    }

    if (flag == 1)
    {
        ConstructMap();
    }
    ConvertRawData2Map(flag);
}

void ReadData()
{
    OnReadData(trainingSetFile,1);
    OnReadData(testingSetFile, 2);
}

void Init()
{
    uint32_t i, j;
    uint32_t choice = 0;
    for (i = 0; i <= MAXLEVELNUM-1; ++i)
    {
        innerNodeList[i] = (TreeNode*)malloc(sizeof(TreeNode));
        memset(innerNodeList[i], 0, sizeof(TreeNode));
        curInnerNode[i] = innerNodeList[i];
    }

    leafList= (TreeNode*)malloc(sizeof(TreeNode));
    memset(leafList, 0, sizeof(TreeNode));
    curLeaf = leafList;
    confusionMatrix = (uint32_t **)malloc(classNum*sizeof(uint32_t));
    memset(confusionMatrix, 0, classNum*sizeof(uint32_t));
    for(i = 0;i<=classNum-1;i++){
        confusionMatrix[i] = (uint32_t*)malloc(classNum*sizeof(uint32_t));
        memset(confusionMatrix[i],0,classNum*sizeof(uint32_t));
    }
    //
    MallocMemory();
    ReadData();
}

uint32_t MatchAttribute(const uint32_t levelNo,uint32_t *test, uint32_t* pathAttributeNameMap,uint32_t* pathAttributeValueMap, uint32_t* pathFlag)
{
    if(levelNo == 0)
        return 1;

    uint32_t i = 0;
    for(i = 0; i <= levelNo - 1; i++)
    {
        if ( map[pathAttributeNameMap[i] ].isConsecutive == 0)
        {
            if(test[pathAttributeNameMap[i]]!=pathAttributeValueMap[i])
                return 0;
        }
        else {
            if (pathFlag[i] == 0)//<=
            {
                if(test[pathAttributeNameMap[i]] > pathAttributeValueMap[i])
                {
                    return 0;
                }
            }
            else if (pathFlag[i] == 1)
            {

                if(test[pathAttributeNameMap[i]] <= pathAttributeValueMap[i])
                {
                    return 0;
                }
            }
        }
    }

    return 1;

}

uint32_t FindMaxClassLab(int len, uint32_t arr[])
{
    uint32_t i = 0, max = 0;
    for (i = 1; i < len; i++) {
        if (arr[max] < arr[i])
            max = i;
    }
    return max;
}

void InsertIntoLeafList(TreeNode* leafNode)
{
    curLeaf->nextLeaf = leafNode;
    leafNode->preLeaf = curLeaf;
    curLeaf = leafNode;
    leafNode->nextLeaf = NULL;
}

void deleteFromLeafList(TreeNode* leafNode)
{
    TreeNode* tempLeaf=leafList->nextLeaf;
    while (tempLeaf!=NULL && (tempLeaf!=leafNode))
      tempLeaf=tempLeaf->nextLeaf;
    if (tempLeaf==NULL) {
           printf("Didnot Find such leaf\n");
           return;
    }
    else if (tempLeaf==curLeaf)
    {
           curLeaf=tempLeaf->preLeaf;
           curLeaf->nextLeaf=NULL;
    }
    else {
            tempLeaf->preLeaf->nextLeaf=tempLeaf->nextLeaf;
            tempLeaf->nextLeaf->preLeaf=tempLeaf->preLeaf;
    }

}

void InsertIntoInnerList(uint32_t levelNo,TreeNode* innerNode)
{
    curInnerNode[levelNo]->nextInnerNode = innerNode;
    curInnerNode[levelNo] = innerNode;
    if(levelNo > totalLevel)
        totalLevel = levelNo;
}

uint32_t FindMaxValue(uint32_t len, double arr[])
{
    uint32_t i = 1, max = 1;
    for (i = 2; i <= len; i++) {
        if (arr[max] < arr[i])
            max = i;
    }

    return max;
}

uint32_t SelectAttributeByRule(uint32_t levelNo, uint32_t* pathAttributeNameMap, uint32_t* pathAttributeValueMap, uint32_t* pathFlag,
        uint32_t subpartitionnum,uint32_t slipAttribute[attributeNum + 1],double* infogainradio,uint32_t* majorclass, int* _splitvalue)
{
    uint32_t attributestate[classNum];
    uint32_t attributeclass[differValue][classNum];
    int i, j;
    for (i = 0; i < classNum; ++i)
    {
        attributestate[i] = 0;
    }

    double infogain[attributeNum + 1];
    for (i = 1; i <= attributeNum; ++i)
    {
        infogain[i] = 0;
    }

    infogain[0] = -100;
    for (i = 1; i < differValue; ++i )
    {
        for (j = 0;  j < classNum; ++j)
        {
            attributeclass[i][j] = 0;
        }
    }

    uint32_t * tempdata;
    double infoGain0 = 0;
    for(i = 1; i<=numberOfTrainingRecord; i++)
    {
        tempdata = trainingData[i];
        if(MatchAttribute(levelNo,tempdata,pathAttributeNameMap,pathAttributeValueMap, pathFlag)!=0)
            attributestate[tempdata[0]]++;
    }

    //calcute the InfoGain
    for (i = 0; i < classNum; ++i)
    {
        if (attributestate[i] != 0)
        {
            infoGain0 += ((double)attributestate[i]/subpartitionnum)*((YuLog2((double)attributestate[i]/subpartitionnum))*(-1));
        }
    }
    double infoSum, partSum, logSum, spitSum;
    int k, g, h;
    for (i = 1; i <= attributeNum; ++i)
    {
        infoSum = 0;

        if (slipAttribute[i] == 1)
        {
            infogain[i] = -100;
            continue;
        }
        else if (map[i].isConsecutive == 0) //discrete
        {
            for (k = 0; k <= differValue; ++k)
            {
                for (j = 0; j < classNum; ++j)
                {
                    attributeclass[k][j] = 0;
                }
            }

            for (k = 1; k <= numberOfTrainingRecord; ++k)
            {
                tempdata = trainingData[k];

                if(MatchAttribute(levelNo,tempdata,pathAttributeNameMap,pathAttributeValueMap, pathFlag)!=0) {
                    attributeclass[tempdata[i]][tempdata[0]]++;
                }
            }

            spitSum = 0;
            for (j = 0; j < map[i].attributeNum; ++j)
            {
                partSum = 0;

                for (g = 0; g < classNum; ++g)
                {
                    partSum += attributeclass[j][g];
                }

                logSum = 0;
                for (g = 0; g < classNum; ++g)
                {
                    if (attributeclass[j][g] != 0)
                    {
                        logSum += ((double)attributeclass[j][g]/partSum)*(YuLog2((double)attributeclass[j][g]/partSum)*(-1));
                    }
                }

                spitSum += ((double)partSum/subpartitionnum)*(YuLog2((double)partSum/subpartitionnum)* (-1.0));
                infoSum += ((double)partSum/subpartitionnum)*logSum;
            }

            infogain[i] = (infoGain0 - infoSum) / spitSum;
        }
        else {//consecutive, split to two subset(<= , and >)
            for (j = 1; j < map[i].attributeNum-1; ++j) {
                for (k = 0; k < 2; ++k)
                {
                    for (h = 0; h < classNum; ++h)
                    {
                        attributeclass[k][h] = 0;
                    }
                }

                for (k = 1; k <= numberOfTrainingRecord; ++k)
                {
                    tempdata = trainingData[k];
                    if (MatchAttribute(levelNo, tempdata, pathAttributeNameMap, pathAttributeValueMap, pathFlag) != 0){
                           if (tempdata[i] <= map[i].attributeValue[j]) {
                                attributeclass[0][tempdata[0]]++; //<=
                           }
                           else {
                                attributeclass[1][tempdata[0]] ++;//>
                           }
                    }
                }

                spitSum = 0;
                for (k = 0; k < 2; ++k) {
                    partSum = 0;

                    for (h = 0; h < classNum; ++h) {
                        partSum += attributeclass[k][h];
                    }

                    logSum = 0;
                    for (h = 0; h < classNum; ++h)
                    {
                        if (attributeclass[k][h] != 0)
                        {
                            logSum += ((double)attributeclass[k][h]/partSum)*(YuLog2((double)attributeclass[k][h]/partSum)*(-1));
                        }
                    }

                    spitSum += ((double)partSum/subpartitionnum)*(YuLog2((double)partSum/subpartitionnum) * (-1.0));
                    infoSum += ((double)partSum/subpartitionnum)*logSum;
                }

                if (j == 1) {
                    infogain[i] = (infoGain0 - infoSum) / spitSum;
                    *_splitvalue = map[i].attributeValue[j];
                }
                else {
                    double tempgain = (infoGain0 - infoSum) / spitSum;
                    if (tempgain > infogain[i])
                    {
                        infogain[i] = tempgain;
                       *_splitvalue = map[i].attributeValue[j];
                    }
                }
            }
        }
    }

    uint32_t returnattributeno = FindMaxValue(attributeNum, infogain);
    *majorclass = FindMaxClassLab(classNum, attributestate);
    *infogainradio = infogain[returnattributeno];
    return returnattributeno;
}

TreeNode* GenerateDecisionTree(uint32_t levelNo, uint32_t pathAttributeNameMap[MAXLEVELNUM+1],
        uint32_t pathAttributeValueMap[MAXLEVELNUM+1], uint32_t pathFlag[MAXLEVELNUM+1], uint32_t slipAttrValue, uint32_t flag)
{
    int i, j;
    uint32_t slipattribute[attributeNum + 1];
    for (i = 1; i <= attributeNum; ++i)
    {
        slipattribute[i] = 0;
    }

    slipattribute[0] = 1;//classLab
    uint32_t attributestate[differValue];
    for (i = 0;  i <= differValue-1; ++i)
    {
        attributestate[i] = 0;
    }

    uint32_t attributeclassstate[differValue][classNum+1];
    for (i = 0; i < differValue; ++i)
    {
        for (j = 0; j <= classNum; ++j)
        {
            attributeclassstate[i][j] = 0;
        }
    }

    if (levelNo != 0)//not the root
    {
        for (i = 0; i < levelNo; ++i)
        {
            slipattribute[pathAttributeNameMap[i]] = 1;//remove those which has sliped attribute.
        }
    }


    TreeNode* currentNode = (TreeNode*)malloc(sizeof(TreeNode));
    memset(currentNode,0,sizeof(TreeNode));
    if(levelNo > 1){
        for(i = 0;i<=levelNo-2;i++){
            currentNode->pathAttributeName[i] = pathAttributeNameMap[i];
            currentNode->pathAttributeValue[i] = pathAttributeValueMap[i];
            currentNode->pathFlag[i] = pathFlag[i];
        }
        currentNode->pathAttributeName[levelNo-1] = pathAttributeNameMap[levelNo-1];
        currentNode->pathAttributeValue[levelNo-1] = slipAttrValue;
        currentNode->pathFlag[levelNo - 1] = flag;
    }
    else if(levelNo == 1){
        currentNode->pathAttributeName[levelNo - 1] = pathAttributeNameMap[levelNo-1];
        currentNode->pathAttributeValue[levelNo - 1] = slipAttrValue;
        currentNode->pathFlag[levelNo - 1] = flag;
    }

    uint32_t subPartitionNum = 0; // the count of records fellow this path.
    uint32_t * tempData = NULL;
    for (j = 1; j <= numberOfTrainingRecord; ++j)
    {
        tempData = trainingData[j];

        if (MatchAttribute(levelNo, tempData, currentNode->pathAttributeName, currentNode->pathAttributeValue, currentNode->pathFlag) != 0)
        {
            attributestate[tempData[0]]++;//classLab
            subPartitionNum++;
        }
    }

    uint32_t currentClass;
    uint32_t testPure = 0;
    for (i = 0; i < classNum; ++i)
    {
        if (attributestate[i] != 0)
        {
            testPure++;
            currentClass = i;
        }
    }
    for (i = 1; i <= attributeNum; ++i)
    {
        if (slipattribute[i] != 1)
            break;
    }
    //hit the max level or donot has any attribute not sliped yet
    if (levelNo >= 200 || i > attributeNum)
    {
        currentNode->classify = FindMaxClassLab(classNum,attributestate);
        currentNode->isLeaf = 1;
        currentNode->selfLevel = levelNo;
        currentNode->childNode = NULL;
        currentNode->siblingNode = NULL;
        InsertIntoLeafList(currentNode);
        return currentNode;
    }

    //just one classLab
    if (testPure == 1)
    {
        currentNode->classify = currentClass;
        currentNode->isLeaf = 1;
        currentNode->selfLevel = levelNo;
        currentNode->childNode = NULL;
        currentNode->siblingNode = NULL;
        InsertIntoLeafList(currentNode);
        return currentNode;
    }

    uint32_t majorClass = 0;
    uint32_t slipAttributeNo = 0;
    double infogain = 0;

    int slipvalueforconsecutive;

    slipAttributeNo = SelectAttributeByRule(levelNo, currentNode->pathAttributeName, currentNode->pathAttributeValue, currentNode->pathFlag,
            subPartitionNum, slipattribute, &infogain, &majorClass, &slipvalueforconsecutive);//this function have to change the value of majorclass and infogain
    if (slipAttributeNo == 0)
    {
        printf("ERROR");
    }
    currentNode->classify = 0;
    currentNode->isLeaf = 0;
    currentNode->selfLevel = levelNo;
    currentNode->pathAttributeName[levelNo] = slipAttributeNo;
    currentNode->infoGainRatio = infogain;
    currentNode->majorClass = majorClass;
    currentNode->childNode = NULL;
    currentNode->siblingNode = NULL;
    InsertIntoInnerList(levelNo,currentNode);
    TreeNode* tempNode = currentNode;
    slipattribute[slipAttributeNo] = 1;
    if (map[slipAttributeNo].isConsecutive == 1)//consecutive
    {
        for(i = 1; i <= numberOfTrainingRecord; i++)
        {
            tempData = trainingData[i];
            if(MatchAttribute(levelNo,tempData,currentNode->pathAttributeName,currentNode->pathAttributeValue, currentNode->pathFlag)!=0)
            {
                if (tempData[slipAttributeNo] <= slipvalueforconsecutive) {
                	attributeclassstate[0][tempData[0]]++; //<=
                }
                else {
                	attributeclassstate[1][tempData[0]] ++;//>
                }
            }
        }

        uint32_t ispureclass = 0;
        for (j = 0; j < classNum; ++j)
        {
            if (attributeclassstate[0][j] != 0)//the first value of slipattribute, because the are index start from 0 to attri.num-1
            {
                ispureclass++;
            }
        }

        TreeNode* newNode;
        int k;
        if(ispureclass == 0)
        {
            newNode = (TreeNode*)malloc(sizeof(TreeNode));
            memset(newNode,0,sizeof(TreeNode));
            if(levelNo == 0){
                newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];
            }
            else{
                for(k = 0;k <= levelNo-1;k++){
                    newNode->pathAttributeName[k] = currentNode->pathAttributeName[k];
                    newNode->pathAttributeValue[k] = currentNode->pathAttributeValue[k];
                    newNode->pathFlag[k] = currentNode->pathFlag[k];
                }
                newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];

            }

            newNode->pathAttributeValue[levelNo] = slipvalueforconsecutive;
            newNode->pathFlag[levelNo] = 0;
            newNode->isLeaf = 1;
            newNode->selfLevel = levelNo+1;
            newNode->childNode = NULL;
            newNode->siblingNode = NULL;
            newNode->classify = FindMaxClassLab(classNum,attributestate);
            InsertIntoLeafList(newNode);
        }
        else{
            newNode = GenerateDecisionTree(levelNo+1,currentNode->pathAttributeName,currentNode->pathAttributeValue, currentNode->pathFlag, slipvalueforconsecutive, 0);

        }
        currentNode->childNode = newNode;
        tempNode = newNode;

        for(i = 1; i<=1;i++)
        {
            ispureclass = 0;
            for(j = 0; j<=classNum-1; j++){
                if(attributeclassstate[i][j]!=0)
                    ispureclass++;
            }
            if(ispureclass == 0){
                newNode = (TreeNode*)malloc(sizeof(TreeNode));
                memset(newNode,0,sizeof(TreeNode));
                if(levelNo == 0){
                    newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];
                }
                else{
                    for(k = 0;k <= levelNo-1;k++){
                        newNode->pathAttributeName[k] = currentNode->pathAttributeName[k];
                        newNode->pathAttributeValue[k] = currentNode->pathAttributeValue[k];
                        newNode->pathFlag[k] = currentNode->pathFlag[k];
                    }
                    newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];
                }
                newNode->pathAttributeValue[levelNo]=slipvalueforconsecutive;
                newNode->pathFlag[levelNo] = 1;//>
                newNode->isLeaf = 1;
                newNode->selfLevel = levelNo+1;
                newNode->childNode = NULL;
                newNode->siblingNode = NULL;
                newNode->classify = FindMaxClassLab(classNum,attributestate);
                InsertIntoLeafList(newNode);
            }
            else{
                newNode = GenerateDecisionTree(levelNo+1,currentNode->pathAttributeName,currentNode->pathAttributeValue,currentNode->pathFlag, slipvalueforconsecutive, 1);
            }
            tempNode->siblingNode = newNode;
            tempNode = newNode;
        }
    }
    else//discrete
    {
        for(i = 1; i <= numberOfTrainingRecord; i++)
        {
            tempData = trainingData[i];
            if(MatchAttribute(levelNo,tempData,currentNode->pathAttributeName,currentNode->pathAttributeValue, currentNode->pathFlag)!=0)
            {
                attributeclassstate[tempData[slipAttributeNo]][tempData[0]]++;
            }

        }

        uint32_t ispureclass = 0;
        for (j = 0; j < classNum; ++j)
        {
            if (attributeclassstate[0][j] != 0)//the first value of slipattribute, because the are index start from 0 to attri.num-1
            {
                ispureclass++;
            }
        }

        TreeNode* newNode;
        int k;
        if(ispureclass == 0)
        {
            newNode = (TreeNode*)malloc(sizeof(TreeNode));
            memset(newNode,0,sizeof(TreeNode));
            if(levelNo == 0){
                newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];
            }
            else{
                for(k = 0;k <= levelNo-1;k++){
                    newNode->pathAttributeName[k] = currentNode->pathAttributeName[k];
                    newNode->pathAttributeValue[k] = currentNode->pathAttributeValue[k];
                }
                newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];

            }

            newNode->pathAttributeValue[levelNo]=0;
            newNode->isLeaf = 1;
            newNode->selfLevel = levelNo+1;
            newNode->childNode = NULL;
            newNode->siblingNode = NULL;
            newNode->classify = FindMaxClassLab(classNum,attributestate);
            InsertIntoLeafList(newNode);
        }
        else{
            newNode = GenerateDecisionTree(levelNo+1,currentNode->pathAttributeName,currentNode->pathAttributeValue,currentNode->pathFlag, 0, 2);

        }
        currentNode->childNode = newNode;
        tempNode = newNode;

        for(i = 1; i<=map[slipAttributeNo].attributeNum-1;i++)
        {
            ispureclass = 0;
            for(j = 0; j<=classNum-1; j++){
                if(attributeclassstate[i][j]!=0)
                    ispureclass++;
            }
            if(ispureclass == 0){
                newNode = (TreeNode*)malloc(sizeof(TreeNode));
                memset(newNode,0,sizeof(TreeNode));
                if(levelNo == 0){
                    newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];
                }
                else{
                    for(k = 0;k <= levelNo-1;k++){
                        newNode->pathAttributeName[k] = currentNode->pathAttributeName[k];
                        newNode->pathAttributeValue[k] = currentNode->pathAttributeValue[k];
                    }
                    newNode->pathAttributeName[levelNo] = currentNode->pathAttributeName[levelNo];
                }
                newNode->pathAttributeValue[levelNo]=i;
                newNode->isLeaf = 1;
                newNode->selfLevel = levelNo+1;
                newNode->childNode = NULL;
                newNode->siblingNode = NULL;
                newNode->classify = FindMaxClassLab(classNum,attributestate);
                InsertIntoLeafList(newNode);
            }
            else{
                newNode = GenerateDecisionTree(levelNo+1,currentNode->pathAttributeName,currentNode->pathAttributeValue,currentNode->pathFlag, i, 3);
            }

            tempNode->siblingNode = newNode;
            tempNode = newNode;
        }
    }

    return currentNode;
}

void VisitTree(FILE* fp,TreeNode* currentNode)
{
    int i;
    int level=currentNode->selfLevel;
    for (i=0;i<=level-1;i++) fprintf(fp,"   ");
    fprintf(fp,"level %d: ", currentNode->selfLevel);
    if (level>0){
        fprintf(fp,"%s",map[currentNode->pathAttributeName[level-1]].attributeName);
        if (map[currentNode->pathAttributeName[level-1]].isConsecutive)
        {
          if (currentNode->pathFlag[level-1]==0) fprintf(fp,"<=");
                else fprintf(fp,">");
        }
        else fprintf(fp,"=");
        fprintf(fp,"%d",currentNode->pathAttributeValue[level-1]);
    }
    if (currentNode->isLeaf==1)
            fprintf(fp,"----> %d",currentNode->classify);
    fprintf(fp,"\n");
    if (currentNode->childNode != NULL)
    {
      VisitTree(fp,currentNode->childNode);
      TreeNode* tempNode=currentNode->childNode;
      while (tempNode->siblingNode != NULL)
      {
        VisitTree(fp,tempNode->siblingNode);
        tempNode=tempNode->siblingNode;
      }
    }
}

void OutputRule(FILE* fp,TreeNode* outputleaflist)
{
    fprintf(fp,"\nthis rule number is:\n");
    TreeNode* tempNode = NULL;
    tempNode = outputleaflist->nextLeaf;
    int ruleNum = 1;
    int i;
    while (tempNode != NULL)
    {
        if (tempNode->selfLevel==0)
        {
          fprintf(fp,"the classLab is always %d\n",tempNode->majorClass);
          return;
        }
        if (map[tempNode->pathAttributeName[0]].isConsecutive == 0)//discrete
        {
            fprintf(fp,"If %s = %s",map[tempNode->pathAttributeName[0]].attributeName, map[tempNode->pathAttributeName[0]].attributes[tempNode->pathAttributeValue[0]]);
        }
        else {//
            if (tempNode->pathFlag[0] == 0) //<=
            {
               fprintf(fp,"If %s <= %d",map[tempNode->pathAttributeName[0]].attributeName, tempNode->pathAttributeValue[0]);
            }
            else { //>
               fprintf(fp,"If %s > %d",map[tempNode->pathAttributeName[0]].attributeName, tempNode->pathAttributeValue[0]);
            }
        }

        if (tempNode->selfLevel >= 1)
        {
            for(i = 1;i<=tempNode->selfLevel-1;i++){
                fprintf(fp," and ");

                if (map[tempNode->pathAttributeName[i]].isConsecutive == 0)//discrete
                {
                    fprintf(fp,"%s = %s",map[tempNode->pathAttributeName[i]].attributeName, map[tempNode->pathAttributeName[i]].attributes[tempNode->pathAttributeValue[i]]);
                }
                else {//
                    if (tempNode->pathFlag[i] == 0) //<=
                    {
                        fprintf(fp,"%s <= %d",map[tempNode->pathAttributeName[i]].attributeName, tempNode->pathAttributeValue[i]);
                    }
                    else { //>
                        fprintf(fp,"%s > %d",map[tempNode->pathAttributeName[i]].attributeName, tempNode->pathAttributeValue[i]);
                    }
            }
            }
        }

        fprintf(fp," then %s = %s\n",map[0].attributeName, map[0].attributes[tempNode->classify]);
        tempNode = tempNode->nextLeaf;
        ++ruleNum;
    }

}

void RunDataOnDecisionTree(FILE* fp, TreeNode* root, uint32_t* testdata, uint32_t** confusionMatrix)
{
    uint32_t ismatched = 0;

    TreeNode* tempNode = NULL;
    tempNode = root->childNode;

    int i,j,result;

    while (tempNode != NULL)
    {
        if (MatchAttribute(tempNode->selfLevel, testdata, tempNode->pathAttributeName, tempNode->pathAttributeValue, tempNode->pathFlag) != 0)
        {
            if (tempNode->isLeaf == 1)
            {
                result = tempNode->classify;
                result = result % (classNum);
                confusionMatrix[testdata[0]%classNum][result] ++;
                ismatched = 1;
                break;
            }
            else {
                ismatched = 1;
                RunDataOnDecisionTree(fp, tempNode, testdata, confusionMatrix);
                break;
            }
        }

        tempNode = tempNode->siblingNode;
    }

    if (ismatched == 0)
    {
        result = root->majorClass;
        result = result%classNum;
        confusionMatrix[testdata[0]%classNum][result]++;
    }

    if (isPrintResult == 1)
    {
        fprintf(fp, "%s .", map[0].attributes[result]);
    }
}

void RunTestData(FILE* fp, TreeNode* root)
{
    int i;
    for (i = 1; i <= numberOfTestingRecord; ++i)
    {
        if (isPrintResult == 1)
        {
            fprintf(fp, "test data%d's predict result is: ", i);
        }
        RunDataOnDecisionTree(fp, root, testData[i], confusionMatrix);
        if (isPrintResult)
        {
            fprintf(fp, "\n");
        }
    }
}

void OutputConfusionMatrix(FILE* fp)
{
    fprintf(fp,"\nconfusion matrix:\n");
    int i, j;
    for (i = 0; i < classNum; ++i)
    {
        for (j = 0; j < classNum; ++j)
        {
            fprintf(fp,"%d ", confusionMatrix[i][j]);
        }
        fprintf(fp,"\n");
    }

    fprintf(fp,"\n\n");
}

void OutputAccuracyRating(FILE* fp)
{
    fprintf(fp,"\naccuracy rating:\n");
    int i, j;
    int rightsum = 0;
    int totalsum = 0;
    for (i = 0; i < classNum; ++i)
    {
        for (j = 0; j < classNum; ++j)
        {
            if (i == j)
            {
                rightsum += confusionMatrix[i][j];
            }
            totalsum += confusionMatrix[i][j];
        }
    }
    double ans = 1.0*rightsum/totalsum;
    fprintf(fp,"%lf\n", ans);
    fprintf(fp,"\n\n");
}

void DeleteTree(TreeNode* T)
{
   TreeNode* deleteNode=T->childNode;
   TreeNode* del_temp;
   while(deleteNode!=NULL){
      del_temp = deleteNode;
      deleteNode = del_temp->siblingNode;
      del_temp->siblingNode = NULL;
      DeleteTree(del_temp);
      if (del_temp->isLeaf==1)
         deleteFromLeafList(del_temp);
      free(del_temp);
   }

}

double  EstimateErrors(TreeNode* T)
{
    uint32_t subPartitionNum = 0; // the count of records fellow this path.
    uint32_t * tempData = NULL;
    TreeNode* currentNode=T;

    uint32_t attributestate[differValue];
    uint32_t levelNo=T->selfLevel;

    int i,j;

    for (i = 0;  i <= differValue-1; ++i)
    {
        attributestate[i] = 0;
    }

    for (j = 1; j <= numberOfTrainingRecord; ++j)
    {
        tempData = trainingData[j];

        if (MatchAttribute(levelNo, tempData, currentNode->pathAttributeName, currentNode->pathAttributeValue, currentNode->pathFlag) != 0)
        {
            attributestate[tempData[0]]++;//classLab
            subPartitionNum++;
        }
    }
    int majorclass = FindMaxClassLab(classNum, attributestate);
    int classNo=subPartitionNum;
    int major_ClassNo=attributestate[majorclass];
    int Error_ClassNo=classNo-major_ClassNo;

   double tree_errors=GetErrors(classNo,Error_ClassNo);

   if (!T->isLeaf){
      double branch_errors=EstimateErrors(T->childNode);
      if (T->childNode==NULL) printf("dfasdfasd");
      TreeNode* tempNode=T->childNode->siblingNode;
      while (tempNode!=NULL)
      {
          branch_errors+=EstimateErrors(tempNode);
          tempNode=tempNode->siblingNode;
          }
      if (tree_errors<=branch_errors)
      {
        DeleteTree(T);
        T->childNode=NULL;
        T->isLeaf=1;
        T->classify=majorclass;
        InsertIntoLeafList(T);
      } else tree_errors=branch_errors;
   }
   return tree_errors;
}

void PostPrune(FILE* fp,TreeNode *T)
{
   fprintf(fp,"\nPostPrunning Decision Tree!\n\n");

   EstimateErrors(T);
}

int main(int argc, char* argv[])
{
    TreeNode* root = NULL;
    uint32_t initPathAttributeName[MAXLEVELNUM + 1]={0};
    uint32_t initPathAttributeValue[MAXLEVELNUM + 1]={0};
    uint32_t initPathFlag[MAXLEVELNUM + 1] = {0};
    Read(argc, argv);
    Init();
    clock_t start, end;
    start = clock();

    root = GenerateDecisionTree(0, initPathAttributeName, initPathAttributeValue, initPathFlag, 0, 0);

    FILE *fp=fopen("output.txt","w");
    if (!fp) {
          printf("Open File Error!\n");
          exit(-1);

    }

    PostPrune(fp,root);
    
    TestVisitTree(fp,root);

    OutputRule(fp,leafList);
    

    if (isPrintResult == 1)
    {
        fprintf(fp, "Predict Result is:\n");    
    }

    RunTestData(fp, root);
    
    end = clock();
    OutputConfusionMatrix(fp);
    OutputAccuracyRating(fp);

    //printf elapsed time
    fprintf( fp,"Elapsed time: \n %lf seconds\n\n", (double)(end-start)/CLOCKS_PER_SEC);
    fclose(fp);
    return 0;
}

