#include "decisiontree.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

AttributeMap* map;
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
    
    for (i = 0; i < numberOfTestingRecord; i++) {
        testData[i] = (uint32_t *)malloc(sizeof(uint32_t) * (1+attributeNum));
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

int MapStr2Num(int i, char *str)
{
	if (i >= attributeNum) {
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

void TestTrainData()
{
    printf("TestTrainingData");
    int i, j;
    for ( i = 1; i <= numberOfTrainingRecord; ++i)
    {
        for (j = 1; j <= attributeNum; ++j)
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
        for (j = 1; j<= attributeNum; ++j)
        {
            printf("%d   ", testData[i][j]);
        }
        printf("\n");
    }

    printf("\n\n");
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
	for (j = 1; j <= attributeNum; j++) {
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
}


int ConvertRawData2Map(int flag)
{
    int i, j;
	/* assign the training data and test data */
	for (j = 1; j < attributeNum; j++) {
		i = 0;
		if (flag == 1) {
			trainingData[i][j] = MapStr2Num(j, rawData[i][j]);
			i++;
		}

        else if(flag == 2) {
			testData[i][j] = MapStr2Num(j, rawData[i][j]);
			i++;
		}
	}

	return 0;
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
        for (j = 1; j <= attributeNum; ++j)
        {
            printf("%s      ",rawData[i][j] );
        }
        printf("\n");
    }

    printf("\n\n");
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
        for (j =0; j < 2; ++j)
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
        //attribute is range from 1 to attributenum.
        
        for (j = 1; j <= attributeNum; j++) {
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

            printf("haoba:  %s \n", rawData[i][j]);
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
    //    ConvertRawData2Map(flag);
    TestRawData(flag);
}

void ReadData()
{
    OnReadData(trainingSetFile,1);

    OnReadData(testingSetFile, 2);
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

TreeNode* GenerateDecisionTree(uint32_t level_no,uint32_t level_map[MAXLEVELNUM],
        uint32_t level_class_map[MAXLEVELNUM],uint32_t slip_attr_value)
{
    
}

int main(int argc, char* argv[])
{
    TreeNode* root = NULL;
    /*for test
    root = (TreeNode*)malloc (sizeof(TreeNode));
    root->infoGain = 0.0;
    printf ("%lf\n", root->infoGain);*/
    Read(argc, argv);
    Init();
//   root = GenerateDecisionTree();
    //TestRead();
    return 0;    
}

