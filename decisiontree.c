#include "decisiontree.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

TreeNode* innerNodeList[MAXLEVELNUM] = { NULL };
TreeNode* curInnerNode[MAXLEVELNUM] = { NULL };
TreeNode* leafList = NULL;
TreeNode* curLeaf = NULL;

void Init(int argc, char* argv[])
{
    /*
    int k;
    printf("argc = %d\n", argc);
    for (k = 0; k < argc; ++k)
    {
        printf("%s\n", argv[k]);        
    }*/
    uint32_t i, j;
    uint32_t choice = 0;
    char filename[100] = { 0 };

    for (i = 0; i <= MAXLEVELNUM-1; ++i)
    {
        innerNodeList[i] = (TreeNode*)malloc(sizeof(TreeNode));
        memset(innerNodeList[i], 0, sizeof(TreeNode));

        curInnerNode[i] = innerNodeList[i];
    }

    leafList= (TreeNode*)malloc(sizeof(TreeNode));
    memset(leafList, 0, sizeof(TreeNode));
    curLeaf = leafList;
    //printf("hello world");

}

int main(int argc, char* argv[])
{
    TreeNode* root = NULL;
    /*for test
    root = (TreeNode*)malloc (sizeof(TreeNode));
    root->infoGain = 0.0;
    printf ("%lf\n", root->infoGain);*/
    Init(argc, argv);

    return 0;    
}

