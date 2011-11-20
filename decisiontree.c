#include "decisiontree.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

AttributeMap* map; 
//attribute is range from 1 to attributenum. colcume 0 is the classLab
char ***rawData = NULL;
uint32_t **trainingData = NULL;
uint32_t **testData = NULL;
TreeNode* innerNodeList[MAXLEVELNUM] = { NULL };
TreeNode* curInnerNode[MAXLEVELNUM] = { NULL };
TreeNode* leafList = NULL;
TreeNode* curLeaf = NULL;
int classNum;
int attributeNum;
int numberOfTrainingRecord;
int numberOfTestingRecord;
int isPrintResult;
char* trainingSetFile;
char* testingSetFile;
uint32_t ** confusionMatrix;
uint32_t totalLevel;

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
    int j, i;

    for (j = 0; j <= attributeNum; ++j)
    {
        printf("attri: %s: has %d attrs\n", map[j].attributeName, map[j].attributeNum);
        
        for (i = 0; i < map[j].attributeNum; ++i)
        {
            printf("%s; ", map[j].attributes[i]);
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


int ConvertString2Number(char* str)
{
   int retNum = 0;
   int i;
   for (i = 0; str[i] != '\0'; ++i)
   {
       retNum *= 10;
       retNum += (str[i] - '0');
   }

   return retNum;
}

void Read(int argc, char* argv[])
{
    int k;
    printf("start\n");
    for (k = 1; k < argc; ++k)
    {
        printf("%s\n", argv[k]);
        if (strcmp(argv[k],"-r") == 0)
        {
            ++k;
            trainingSetFile = argv[k];
            printf("argv1 = %s\n", argv[k-1]);
        }
        else if (!strcmp(argv[k], "-t"))
        {
            ++k;
            testingSetFile = argv[k];
            printf("argv2 = %s\n", argv[k-1]);
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

    TestRead();
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
        trainingData[i] = (int *)malloc(sizeof(int) *(attributeNum+1));
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
    strcpy(map[0].attributeName, "classLab");
    for (j = 1; j <= attributeNum; j++)
    {
        strcpy(map[j].attributeName, "attribute");
   //     strcat(map[j].attributeName, (i+'0'));
    }
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

		/* free the list */
		while ((ptr = list)) {
			list = list->next;
			free(ptr);
		}
		list = tail = ptr = NULL;

		num = 0;
	}

    TestMap();
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
                trainingData[i][j] = MapAttribute2Num(j, rawData[i][j]);
                i++;
            }
		}

        else if(flag == 2) {
            while (i <= numberOfTestingRecord)
            {
    			testData[i][j] = MapAttribute2Num(j, rawData[i][j]);
	    		i++;
            }
		}
	}

	return 0;
}

void OnReadData(char* filename, int flag/*training or testing*/)
{
    //
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
    //fgets(buffer, n, fp);
    //begin = buffer;
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
            fprintf(stderr, "line 377 err.\n");
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

           // printf("haoba:  %s \n", rawData[i][j]);
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
            
        //printf("haoba:  %s \n", rawData[i][j]);
        i++;
    }

    if (flag == 1)
    {
        ConstructMap();
    }
    ConvertRawData2Map(flag);
    //TestRawData(flag);
}

void ReadData()
{
    OnReadData(trainingSetFile,1);

    OnReadData(testingSetFile, 2);

    TestTrainData();
    TestTestingData();
}

void Init()
{
    /*
     *-r training.txt -t testing.txt -d 65 -c 2 -s 32768 -m 2000 –p 0
    int k;
    printf("argc = %d\n", argc);*/
    
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

uint32_t MatchAttribute(uint32_t levelNo,uint32_t *test, uint32_t* pathAttributeNameMap,uint32_t* pathAttributeValueMap)
{
    if(levelNo == 0)
        return 1;
        
    uint32_t i = 0;
    for(i=0;i<=levelNo-1;i++)
    {
        if(test[pathAttributeNameMap[i]]!=pathAttributeValueMap[i])
            return 0;
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

void InsertIntoInnerList(uint32_t levelNo,TreeNode* innerNode)
{
    curInnerNode[levelNo]->nextInnerNode = innerNode;
    curInnerNode[levelNo] = innerNode;
    if(levelNo > totalLevel)
        totalLevel = levelNo;
}

uint32_t SelectAttributeByRule()
{}

TreeNode* GenerateDecisionTree(uint32_t levelNo, uint32_t pathAttributeNameMap[MAXLEVELNUM],
        uint32_t pathAttributeValueMap[MAXLEVELNUM], uint32_t slipAttrValue)
{
    int i, j;

    uint32_t slipattribute[attributeNum + 1];
    for (i = 1; i <= attributeNum; ++i)
    {
        slipattribute[i] = 0;
    }
    slipattribute[0] = 1;//classLab
    
    uint32_t attributestate[attributeNum+1];
    for (i = 0;  i <= attributeNum; ++i)
    {
        attributestate[i] = 0;
    }
    
    uint32_t attributeclassstate[attributeNum + 1][classNum+1];
    for (i = 0; i <= attributeNum; ++i)
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
        }
        currentNode->pathAttributeName[levelNo-1] = pathAttributeNameMap[levelNo-1];
        currentNode->pathAttributeValue[levelNo-1] = slipAttrValue;
    }
    else if(levelNo == 1){
        currentNode->pathAttributeName[levelNo-1] = pathAttributeNameMap[levelNo-1];
        currentNode->pathAttributeValue[levelNo-1] = slipAttrValue;
    }

    uint32_t subPartitionNum = 0;
    uint32_t * tempData = NULL;
    for (j = 1; j <= numberOfTrainingRecord; ++j)
    {
        tempData = trainingData[j];

        if (MatchAttribute(levelNo, tempData, currentNode->pathAttributeName, currentNode->pathAttributeValue) != 0)
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
    if (levelNo == MAXLEVELNUM || i > attributeNum)
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
    slipAttributeNo = SelectAttributeByRule();//this function have to change the value of majorclass and infogain

    currentNode->classify = 0;
    currentNode->isLeaf = 0;
    currentNode->selfLevel = levelNo;
    currentNode->pathAttributeName[levelNo] = slipAttributeNo;
    currentNode->infoGain = infogain;
    currentNode->majorClass = majorClass;
    currentNode->childNode = NULL;
    currentNode->siblingNode = NULL;
    InsertIntoInnerList(levelNo,currentNode);
    TreeNode* tempNode = currentNode;
    slipattribute[slipAttributeNo] = 1;

    for(i = 1; i <= numberOfTrainingRecord; i++)
    {
        tempData = trainingData[i];
        if(MatchAttribute(levelNo,tempData,currentNode->pathAttributeName,currentNode->pathAttributeValue)!=0)
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
        newNode = GenerateDecisionTree(levelNo+1,currentNode->pathAttributeName,currentNode->pathAttributeValue,0);
        
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
            newNode = GenerateDecisionTree(levelNo+1,currentNode->pathAttributeName,currentNode->pathAttributeValue,i);
        }
        tempNode->siblingNode = newNode;
        tempNode = newNode;
    }

    return currentNode;
}

int main(int argc, char* argv[])
{
    TreeNode* root = NULL;
    uint32_t initPathAttributeName[MAXLEVELNUM]={0};
    uint32_t initPathAttributeValue[MAXLEVELNUM]={0};
    /*for test
    root = (TreeNode*)malloc (sizeof(TreeNode));
    root->infoGain = 0.0;
    printf ("%lf\n", root->infoGain);*/
    Read(argc, argv);
    Init();
 
    root = GenerateDecisionTree(0, initPathAttributeName, initPathAttributeValue, 0);
    //TestRead();
    return 0;    
}

