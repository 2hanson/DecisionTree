#define MAXLEN 100
#define MAXLEVELNUM 200

typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;

typedef struct
{
    char attributeName[MAXLEN];
    char **attributes;
    int attributeNum;
}AttributeMap;

typedef struct 
{
    uint32_t leafClass;
    uint32_t selLevel;
    uint32_t majorClass;
    uint32_t pathAttributeName[MAXLEVELNUM];
    uint32_t pathAttributeValue[MAXLEVELNUM];
    struct tree_node* childNode;
    struct tree_node* siblingNode;
    struct tree_node* nextInnerNode;
    struct tree_node* preLeaf;
    struct tree_node* nextLeaf; 
    uint32_t isLeaf; 
    double infoGain;
}TreeNode;

